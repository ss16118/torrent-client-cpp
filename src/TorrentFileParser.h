//
// Created by Siyuan Shen on 2021/5/1.
//

#ifndef BITTORRENTCLIENT_TORRENTFILEPARSER_H
#define BITTORRENTCLIENT_TORRENTFILEPARSER_H
#include <string>
#include <bencode/BDictionary.h>

using byte = unsigned char;
/**
 * A class that parses a given Torrent file by using the bencoding library in following repo:
 * https://github.com/s3rvac/cpp-bencoding.
 * The result returned by bencoding decoder is a pointer to a custom BItem object, which can
 * be a dictionary, a list, an integer, or a string. To retrieve the value of a specific key
 * in the top level dictionary, an instance function named getValue() was added to the
 * BDictionary class. It recursively checks the keys of all dictionaries from the top level
 * and returns the value of key, if it exists.
 */
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