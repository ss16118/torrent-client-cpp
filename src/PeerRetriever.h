//
// Created by siyuan on 04/05/2021.
//

#ifndef BITTORRENTCLIENT_PEERRETRIEVER_H
#define BITTORRENTCLIENT_PEERRETRIEVER_H

#include <vector>
#include <cpr/cpr.h>

/**
 * An representation of peers which the reponse retrieved from the tracker.
 * Contains a string that denotes the IP of the peer as well as a port number.
 */
struct Peer
{
    std::string ip;
    int port;
};

/**
 * Retrieves a list of peers by sending a GET request to the tracker.
 */
class PeerRetriever
{
private:
    std::string announceUrl;
    std::string infoHash;
    std::string peerId;
    int port;
    const unsigned long fileSize;
    std::vector<Peer*> decodeResponse(std::string response);
public:
    explicit PeerRetriever(std::string peerId, std::string announceUrL, std::string infoHash, int port, unsigned long fileSize);
    std::vector<Peer*> retrievePeers(unsigned long bytesDownloaded = 0);
};

#endif //BITTORRENTCLIENT_PEERRETRIEVER_H
