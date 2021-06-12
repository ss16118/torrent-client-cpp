//
// Created by siyuan on 13/05/2021.
//

#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <string>
#include "PeerRetriever.h"
#include "PeerConnection.h"

class TorrentClient
{
private:
    std::string peerId;
    std::vector<Peer*> peers;
    std::vector<PeerConnection> peerConnections;
public:
    explicit TorrentClient();
    ~TorrentClient();
    void downloadFile(const std::string& torrentFilePath, const std::string& downloadPath);
};

#endif //BITTORRENTCLIENT_TORRENTCLIENT_H
