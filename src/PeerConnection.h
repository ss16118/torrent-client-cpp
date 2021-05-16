//
// Created by siyuan on 08/05/2021.
//

#ifndef BITTORRENTCLIENT_PEERCONNECTION_H
#define BITTORRENTCLIENT_PEERCONNECTION_H

#include "PeerRetriever.h"
#include "BitTorrentMessage.h"

using byte = unsigned char;

class PeerConnection
{
private:
    int sock{};
    bool chocked = true;
    std::string bitField;
    const struct Peer peer;
    const std::string peerId;
    const std::string infoHash;
    std::string createHandshakeMessage();
    void performHandshake();
    void receiveBitField();
    void sendInterested();
    void receiveUnchoke();
    void closeConnection();
public:
    explicit PeerConnection(const struct Peer peer, const std::string peerId, const std::string infoHash);
    ~PeerConnection();
    void establishNewConnection();
};


#endif //BITTORRENTCLIENT_PEERCONNECTION_H
