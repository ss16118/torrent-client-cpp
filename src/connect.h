//
// Created by siyuan on 13/05/2021.
//

#ifndef BITTORRENTCLIENT_CONNECT_H
#define BITTORRENTCLIENT_CONNECT_H
#include <string>

/**
 * Functions that handle network connection.
 */

int createConnection(const std::string& ip, int port);
void sendData(int sock, const std::string& data);
std::string receiveData(int sock, uint32_t bufferSize = 0);

#endif //BITTORRENTCLIENT_CONNECT_H
