//
// Created by siyuan on 13/05/2021.
//

#ifndef BITTORRENTCLIENT_TORRENTCLIENT_H
#define BITTORRENTCLIENT_TORRENTCLIENT_H

#include <string>

class TorrentClient
{
private:
    std::string peerId;
public:
    explicit TorrentClient();
    void downloadFile(const std::string& torrentFilePath);
};

#endif //BITTORRENTCLIENT_TORRENTCLIENT_H
