//
// Created by siyuan on 13/05/2021.
//

#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <string>
#include "PeerRetriever.h"
#include "PeerConnection.h"
#include "SharedQueue.h"

class TorrentClient
{
private:
    bool terminated = false;
    std::string peerId;
    SharedQueue<Peer*> queue;
    std::vector<std::thread> threadPool;
    std::vector<PeerConnection*> connections;
    std::mutex threadPoolMutex;
public:
    explicit TorrentClient(bool enableLogging = true);
    ~TorrentClient();
    void terminate();
    void downloadFile(const std::string& torrentFilePath, const std::string& downloadDirectory);
};

#endif //BITTORRENTCLIENT_TORRENTCLIENT_H
