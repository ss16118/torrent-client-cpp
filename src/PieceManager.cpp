//
// Created by siyuan on 17/05/2021.
//

#include <cmath>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <loguru/loguru.hpp>
#include <bencode/bencoding.h>
#include <iomanip>

#include "PieceManager.h"
#include "Block.h"
#include "utils.h"

#define BLOCK_SIZE 16384         // 2 ^ 14
#define MAX_PENDING_TIME 120     // 2 minutes

PieceManager::PieceManager(const TorrentFileParser& fileParser, const std::string& downloadPath): fileParser(fileParser)
{
    missingPieces = initiatePieces();
    // Creates the destination file with the file size specified in the Torrent file
    downloadedFile.open(downloadPath, std::ios::binary | std::ios::out);
    downloadedFile.seekp(fileParser.getFileSize() - 1);
    downloadedFile.write("", 1);

//    for (const Piece* piece : missingPieces)
//    {
//        for (Block* block : piece->blocks)
//        {
//            std::cout << "[DEBUG] Block piece: " << std::to_string(block->piece) << std::endl;
//            std::cout << "[DEBUG] Block offset: " << std::to_string(block->offset) << std::endl;
//            std::cout << "[DEBUG] Block length: " << std::to_string(block->length) << std::endl;
//        }
//    }
}

/**
 * Destructor of the PieceManager class. Frees all resources allocated.
 */
PieceManager::~PieceManager() {
    for (Piece* piece: missingPieces)
        delete piece;

    for (Piece *piece : ongoingPieces)
        delete piece;

    for (PendingRequest* pending : pendingRequests)
        delete pending;

    downloadedFile.close();
}


/**
 * Pre-constructs the list of pieces and blocks based on
 * the number of pieces and the size of the block.
 * @return a vector containing all the pieces in the file.
 */
std::vector<Piece*> PieceManager::initiatePieces()
{
    std::vector<std::string> pieceHashes = fileParser.splitPieceHashes();
    totalPieces = pieceHashes.size();
    std::vector<Piece*> torrentPieces;
    missingPieces.reserve(totalPieces);

    long totalLength = fileParser.getFileSize();
    long pieceLength = fileParser.getPieceLength();

    // number of blocks in a normal piece (i.e. pieces that are not the last one)
    int blockCount = ceil(pieceLength / BLOCK_SIZE);
    long remLength = pieceLength;

    for (int i = 0; i < totalPieces; i++)
    {
        // The final piece is likely to have a smaller size.
        if (i == totalPieces - 1)
        {
            remLength = totalLength % pieceLength;
            blockCount = std::max((int) ceil(remLength / BLOCK_SIZE), 1);
        }
        std::vector<Block*> blocks;
        blocks.reserve(blockCount);

        for (int offset = 0; offset < blockCount; offset++)
        {
            auto block = new Block;
            block->piece = i;
            block->status = missing;
            block->offset = offset * BLOCK_SIZE;
            int blockSize = BLOCK_SIZE;
            if (i == totalPieces - 1 && offset == blockCount - 1)
                blockSize = remLength % BLOCK_SIZE;
            block->length = blockSize;
            blocks.push_back(block);
        }
        auto piece = new Piece(i, blocks, pieceHashes[i]);
        torrentPieces.emplace_back(piece);
    }
    return torrentPieces;
}

/**
 * Checks if all Pieces have been downloaded.
 * @return true if all Pieces are present false otherwise.
 */
bool PieceManager::isComplete() {
    lock.lock();
    bool isComplete = havePieces.size() == totalPieces;
    lock.unlock();
    return isComplete;
}

/**
 * Adds a peer and the BitField representing the pieces the peer has.
 * Store the given information in the instance variable peers.
 */
void PieceManager::addPeer(const std::string& peerId, std::string bitField)
{
    peers[peerId] = bitField;
}

/**
 * Updates the information about which pieces a peer has (i.e. reflects
 * a Have message).
 */
void PieceManager::updatePeer(std::string peerId, int index)
{
    if (peers.find(peerId) != peers.end())
        setPiece(peers[peerId], index);
    else
        throw std::runtime_error("Connection has not been established with peer " + peerId);
}

/**
 * Removes a previous added peer in case of a connection lost.
 * @param peerId: Id of the peer to be removed.
 */
void PieceManager::removePeer(std::string peerId)
{
    auto iter = peers.find(peerId);
    if (iter != peers.end())
        peers.erase(iter);
    else
        throw std::runtime_error("Attempting to remove a peer " + peerId +
                                 " with whom a connection has not been established.");
}


/**
 * Get the next block that should be requested from the given peer.
 * If there are no more blocks left to retrieve or if this peer does not
 * have any of the missing pieces None is returned
 * @return pointer to the Block struct to be requested.
 */
Block* PieceManager::nextRequest(std::string peerId)
{
    // The algorithm implemented for which piece to retrieve is a simple
    // one. This should preferably be replaced with an implementation of
    // "rarest-piece-first" algorithm instead.
    // The algorithm tries to download the pieces in sequence and will try
    // to finish started pieces before starting with new pieces.
    //
    // 1. Check any pending blocks to see if any request should be reissued
    // due to timeout
    // 2. Check the ongoing pieces to get the next block to request
    // 3. Check if this peer have any of the missing pieces not yet started

    // If the peer has not been added yet

    lock.lock();
    if (missingPieces.empty())
    {
        lock.unlock();
        return nullptr;
    }

    if (peers.find(peerId) == peers.end())
    {
        lock.unlock();
        return nullptr;
    }

    Block* block = expiredRequest(peerId);
    if (!block)
    {
        block = nextOngoing(peerId);
        if (!block)
            block = getRarestPiece(peerId)->nextRequest();
    }
    lock.unlock();

    return block;
}

/**
 * Go through previously requested blocks, if any one have been in the
 * requested state for longer than `MAX_PENDING_TIME` return the block to
 * be re-requested. If no pending blocks exist, None is returned
 */
Block* PieceManager::expiredRequest(std::string peerId)
{
    time_t currentTime = std::time(nullptr);
    for (PendingRequest* pending : pendingRequests)
    {
        if (hasPiece(peers[peerId], pending->block->piece))
        {
            // If the request has expired
            auto temp = pending->timestamp;
            temp->tm_sec += MAX_PENDING_TIME;
            time_t boundary = std::mktime(temp);
            auto diff = std::difftime(boundary, currentTime);
            if (diff <= 0)
            {
                // Resets the timer for that request
                pending->timestamp = temp;
                return pending->block;
            }
        }
    }
    return nullptr;
}

/**
 * Iterates through the pieces that are currently being downloaded, and returns
 * the next Block to be requested or NULL if no Block is left to be requested
 * from the list of Pieces.
 */
Block* PieceManager::nextOngoing(std::string peerId)
{
    for (Piece* piece : ongoingPieces)
    {
        if (hasPiece(peers[peerId], piece->index))
        {
            Block* block = piece->nextRequest();
            if (block)
            {
                auto currentTime = std::time(nullptr);
                auto newPendingRequest = new PendingRequest;
                newPendingRequest->block = block;
                newPendingRequest->timestamp = std::localtime(&currentTime);
                pendingRequests.push_back(newPendingRequest);
                return block;
            }
        }
    }
    return nullptr;
}

/**
 * Given the list of missing pieces, finds the rarest one (i.e. a piece
 * which is owned by the fewest number of peers).
 */
Piece* PieceManager::getRarestPiece(std::string peerId)
{
    // Custom comparator to make sure that the map is ordered by the index of the Piece.
    auto comp = [](const Piece* a, const Piece* b) { return a->index < b->index; };
    std::map<Piece*, int, decltype(comp)> pieceCount(comp);
    for (Piece* piece : missingPieces)
    {
        // If a connection has been established with the peer
        if (peers.find(peerId) != peers.end())
        {
            if (hasPiece(peers[peerId], piece->index))
                pieceCount[piece] += 1;
        }
    }

    Piece* rarest;
    int leastCount = INT16_MAX;
    for (auto const& [piece, count] : pieceCount)
    {
        if (count < leastCount)
        {
            leastCount = count;
            rarest = piece;
        }
    }

    missingPieces.erase(
            std::remove(missingPieces.begin(), missingPieces.end(), rarest),
            missingPieces.end()
    );
    ongoingPieces.push_back(rarest);
    return rarest;
}

/**
 * This method is called when a block of data has been received successfully.
 * Once an entire Piece has been received, a SHA1 hash is computed on the data
 * of all the retrieved blocks in the Piece. The hash will be compared to
 * that from the Torrent meta-info. If a mismatch is detected, all the blocks
 * in the Piece will be reset to a missing state. If the hash matches, the data
 * in the Piece will be written to disk.
 */
void PieceManager::blockReceived(std::string peerId, int pieceIndex, int blockOffset, std::string data)
{
    LOG_F(INFO, "Received block %d for piece %d from peer %s", blockOffset, pieceIndex, peerId.c_str());
    // Removes the received block from pending requests
    PendingRequest* requestToRemove = nullptr;
    for (PendingRequest* pending : pendingRequests)
    {
        if (pending->block->piece == pieceIndex && pending->block->offset == blockOffset)
        {
            requestToRemove = pending;
            break;
        }
    }

    pendingRequests.erase(
            std::remove(pendingRequests.begin(), pendingRequests.end(), requestToRemove),
            pendingRequests.end()
    );

    // Retrieves the Piece to which this Block belongs
    Piece* targetPiece = nullptr;
    for (Piece* piece : ongoingPieces)
    {
        if (piece->index == pieceIndex)
        {
            targetPiece = piece;
            break;
        }
    }
    if (!targetPiece)
        throw std::runtime_error("Received Block does not belong to any ongoing Piece.");

    targetPiece->blockReceived(blockOffset, std::move(data));
    if (targetPiece->isComplete())
    {
        // If the Piece is completed and the hash matches,
        // writes the Piece to disk
        if (targetPiece->isHashMatching())
        {
            write(targetPiece);
            // Removes the Piece from the ongoing list
            ongoingPieces.erase(
                    std::remove(ongoingPieces.begin(), ongoingPieces.end(), targetPiece),
                    ongoingPieces.end()
            );

            havePieces.push_back(targetPiece);
            std::stringstream info;
            info << "(" << std::fixed << std::setprecision(2) << (((float) havePieces.size()) / (float) totalPieces * 100) << "%) ";
            info << std::to_string(havePieces.size()) + " / " + std::to_string(totalPieces) + " Pieces downloaded...";
            std::cout << info.str() << std::endl;
            LOG_F(INFO, "%s", info.str().c_str());
        }
        else
        {
            targetPiece->reset();
        }
    }
}

/**
 * Writes the given Piece to disk.
 */
void PieceManager::write(Piece* piece)
{
    lock.lock();
    long position = piece->index * fileParser.getPieceLength();
    downloadedFile.seekp(position);
    downloadedFile << piece->getData();
    lock.unlock();
}
