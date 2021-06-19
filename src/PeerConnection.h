//
// Created by siyuan on 08/05/2021.
//

#ifndef BITTORRENTCLIENT_PEERCONNECTION_H
#define BITTORRENTCLIENT_PEERCONNECTION_H

#include "PeerRetriever.h"
#include "BitTorrentMessage.h"
#include "PieceManager.h"
#include "SharedQueue.h"

using byte = unsigned char;

class PeerConnection
{
private:
    int sock{};
    bool choked = true;
    bool terminated = false;
    bool requestPending = false;
    const std::string clientId;
    const std::string infoHash;
    SharedQueue<Peer*>* queue;
    Peer* peer;
    std::string peerBitField;
    std::string peerId;
    PieceManager* pieceManager;

    std::string createHandshakeMessage();
    void performHandshake();
    void receiveBitField();
    void sendInterested();
    void receiveUnchoke();
    void requestPiece();
    void closeSock();
    bool establishNewConnection();
    BitTorrentMessage receiveMessage(int bufferSize = 0) const;

public:
    const std::string &getPeerId() const;

    explicit PeerConnection(SharedQueue<Peer*>* queue, std::string  clientId, std::string  infoHash, PieceManager* pieceManager);
    ~PeerConnection();
    void start();
    void stop();
};


#endif //BITTORRENTCLIENT_PEERCONNECTION_H
