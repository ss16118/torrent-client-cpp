//
// Created by Siyuan Shen on 2021/5/1.
//

#ifndef BITTORRENTCLIENT_TORRENTFILEPARSER_H
#define BITTORRENTCLIENT_TORRENTFILEPARSER_H
#include <string>
#include <bencode/BDictionary.h>

using byte = unsigned char;

class TorrentFileParser
{
private:
    std::shared_ptr<bencoding::BDictionary> root;
    void splitPieceHashes(int numHashes, std::string piecesString, byte** buffer);
public:
    explicit TorrentFileParser(std::string filePath);
    std::shared_ptr<bencoding::BItem> get(std::string key);
    std::string getInfoHash();
};


#endif //BITTORRENTCLIENT_TORRENTFILEPARSER_H

/**

created by13:mktorrent 1.013:creation datei1570750285e4:infod6:lengthi20e4:name4:priv12:piece lengthi262144e6:pieces20:...B644.o'.......z"+7:privatei1eee


*/