//
// Created by siyuan on 06/05/2021.
//

#ifndef BITTORRENTCLIENT_UTILS_H
#define BITTORRENTCLIENT_UTILS_H

std::string urlEncode(const std::string& value);

std::string hexDecode(const std::string& value);

std::string hexEncode(const std::string& input);

bool hasPiece(const std::string& bitField, int index);

void setPiece(std::string& bitField, int index);

int bytesToInt(std::string bytes);

std::string formatTime(long seconds);

#endif //BITTORRENTCLIENT_UTILS_H
