//
// Created by siyuan on 17/05/2021.
//

#include <cmath>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <bencode/bencoding.h>

#include "PieceManager.h"
#include "Block.h"
#include "utils.h"

#define BLOCK_SIZE 16384         // 2 ^ 14
#define MAX_PENDING_TIME 120     // 2 minutes

PieceManager::PieceManager(const TorrentFileParser& fileParser): fileParser(fileParser)
{
    missingPieces = initiatePieces();
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
}


/**
 * Pre-constructs the list of pieces and blocks based on
 * the number of pieces and the size of the block.
 * @return a vector containing all the pieces in the file.
 */
std::vector<Piece*> PieceManager::initiatePieces()
{
    std::vector<std::string> pieceHashes = fileParser.splitPieceHashes();
    int piecesCount = pieceHashes.size();
    std::vector<Piece*> torrentPieces;
    missingPieces.reserve(piecesCount);

    std::shared_ptr<bencoding::BItem> totalLengthValue = fileParser.get("length");
    if (!totalLengthValue)
        throw std::runtime_error("Torrent file is malformed. [File does not contain key 'length']");
    long totalLength = std::dynamic_pointer_cast<bencoding::BInteger>(totalLengthValue)->value();

    std::shared_ptr<bencoding::BItem> pieceLengthValue = fileParser.get("piece length");
    if (!pieceLengthValue)
        throw std::runtime_error("Torrent file is malformed. [File does not contain key 'piece length']");
    long pieceLength = std::dynamic_pointer_cast<bencoding::BInteger>(pieceLengthValue)->value();

    // number of blocks in a normal piece (i.e. pieces that are not the last one)
    int blockCount = ceil(pieceLength / BLOCK_SIZE);
    long remLength = pieceLength;

    for (int i = 0; i < piecesCount; i++)
    {
        // The final piece is likely to have a smaller size.
        if (i == piecesCount - 1)
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
            if (i == piecesCount - 1 && offset == blockCount - 1)
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
    if (peers.find(peerId) == peers.end())
        return nullptr;

    lock.lock();
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
Piece* PieceManager::getRarestPiece(std::string peerId) {
    std::map<Piece*, int> pieceCount;
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
    std::remove(missingPieces.begin(), missingPieces.end(), rarest), missingPieces.end()
    );
    ongoingPieces.push_back(rarest);
    return rarest;
}
