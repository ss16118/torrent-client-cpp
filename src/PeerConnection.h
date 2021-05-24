//
// Created by siyuan on 08/05/2021.
//

#ifndef BITTORRENTCLIENT_PEERCONNECTION_H
#define BITTORRENTCLIENT_PEERCONNECTION_H

#include "PeerRetriever.h"
#include "BitTorrentMessage.h"
#include "PieceManager.h"

using byte = unsigned char;

class PeerConnection
{
private:
    int sock{};
    bool chocked = true;
    const struct Peer peer;
    const std::string clientId;
    const std::string infoHash;
    std::string peerBitField;
    std::string peerId;
    PieceManager* pieceManager;

    std::string createHandshakeMessage();
    void performHandshake();
    void receiveBitField();
    void sendInterested();
    void receiveUnchoke();
    BitTorrentMessage receiveMessage(int bufferSize = NULL) const;

public:
    const std::string &getPeerId() const;

    explicit PeerConnection(struct Peer peer, std::string clientId, std::string infoHash, PieceManager* pieceManager);
    ~PeerConnection();
    void establishNewConnection();
};


#endif //BITTORRENTCLIENT_PEERCONNECTION_H