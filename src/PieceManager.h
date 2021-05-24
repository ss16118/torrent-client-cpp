//
// Created by siyuan on 17/05/2021.
//

#ifndef BITTORRENTCLIENT_PIECEMANAGER_H
#define BITTORRENTCLIENT_PIECEMANAGER_H

#include <map>
#include <vector>
#include <ctime>
#include <mutex>

#include "Piece.h"
#include "TorrentFileParser.h"


struct PendingRequest
{
    Block* block;
    std::tm* timestamp;
};

/**
 * Responsible for keeping track of all the available pieces
 * from the peers. Implementation is based on the Python code
 * in this repository:
 * https://github.com/eliasson/pieces/
 */
class PieceManager
{
private:
    std::map<std::string, std::string> peers;
    std::vector<Piece*> missingPieces;
    std::vector<Piece*> ongoingPieces;
    std::vector<PendingRequest*> pendingRequests;
    const TorrentFileParser& fileParser;

    // Uses a lock to prevent race condition
    std::mutex lock;

    std::vector<Piece*> initiatePieces();
    Block* expiredRequest(std::string peerId);
    Block* nextOngoing(std::string peerId);
    Piece* getRarestPiece(std::string peerId);
public:
    explicit PieceManager(const TorrentFileParser& fileParser);
    ~PieceManager();
    void addPeer(const std::string& peerId, std::string bitField);
    void updatePeer(std::string peerId, int index);
    Block* nextRequest(std::string peerId);
};

#endif //BITTORRENTCLIENT_PIECEMANAGER_H
