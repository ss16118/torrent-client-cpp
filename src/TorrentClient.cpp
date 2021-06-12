//
// Created by siyuan on 13/05/2021.
//

#include <random>
#include <iostream>
#include <thread>
#include <bencode/bencoding.h>
#include "PieceManager.h"
#include "TorrentClient.h"
#include "TorrentFileParser.h"
#include "PeerRetriever.h"
#include "PeerConnection.h"

#define MAX_PEER_CONNECTIONS 10
#define PORT 8080

TorrentClient::TorrentClient()
{
    // Generate a random 20-byte peer Id for the client as per the convention described
    // on the following web page.
    // https://wiki.theory.org/BitTorrentSpecification#peer_id
    peerId = "-UT2021-";
    // Generate 12 random numbers
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(1, 9);
    for (int i = 0; i < 12; i++)
        peerId += std::to_string(distrib(gen));
}

/**
 * Ensures that all resources are freed when TorrentClient is destroyed
 */
TorrentClient::~TorrentClient()
{
    for (Peer* peer : peers)
        delete peer;
}

/**
 * Download the file as per the content of the given Torrent file.
 * @param torrentFilePath: path to the Torrent file.
 * @param downloadPath: path of the file when it is finished (i.e. the destination path).
 */
void TorrentClient::downloadFile(const std::string& torrentFilePath, const std::string& downloadPath)
{
    // Parse Torrent file
    TorrentFileParser torrentFileParser(torrentFilePath);
    std::string announceUrl = std::dynamic_pointer_cast<bencoding::BString>(torrentFileParser.get("announce"))->value();

    long fileSize = torrentFileParser.getFileSize();
    const std::string infoHash = torrentFileParser.getInfoHash();

    PieceManager pieceManager(torrentFileParser, downloadPath);

    // Retrieve peers from the tracker
    PeerRetriever peerRetriever(peerId, announceUrl, infoHash, PORT, fileSize);
    peers = peerRetriever.retrievePeers();

    std::vector<std::thread> threads;

    // Connect to peers
    if (!peers.empty())
    {
        PeerConnection connection(peers[0], peerId, infoHash, &pieceManager);
        peerConnections.push_back(connection);
        std::thread thread(&PeerConnection::start, connection);
        threads.push_back(std::move(thread));
//        Block* block = pieceManager.nextRequest(connection.getPeerId());
//
//        std::cout << "[DEBUG] Block piece: " << std::to_string(block->piece) << std::endl;
//        std::cout << "[DEBUG] Block offset: " << std::to_string(block->offset) << std::endl;
//        std::cout << "[DEBUG] Block length: " << std::to_string(block->length) << std::endl;
//        std::cout << "[DEBUG] Block status: " << std::to_string(block->status) << std::endl;
    }
    for (std::thread& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
    std::cout << "File downloaded to " << downloadPath << std::endl;
}
