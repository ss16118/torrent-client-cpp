//
// Created by Siyuan Shen on 2021/5/1.
//
#include <regex>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <bencode/BItem.h>
#include <bencode/Decoder.h>
#include <bencode/bencoding.h>
#include <crypto/sha1.h>

#include "TorrentFileParser.h"

/**
 * Constructor of the class TorrentFileParser. Takes in
 * a string that represents the path of the torrent file.
 * parses its content into a string. Stores the string
 * in the instance attribute fileString.
 * @param filePath: path of the torrent file.
 */
TorrentFileParser::TorrentFileParser(std::string filePath)
{
    std::ifstream fileStream(filePath, std::ifstream::binary);
    std::shared_ptr<bencoding::BItem> decodedTorrentFile = bencoding::decode(fileStream);
    std::shared_ptr<bencoding::BDictionary> rootDict =
            std::dynamic_pointer_cast<bencoding::BDictionary>(decodedTorrentFile);
    root = rootDict;
    // std::string prettyRepr = bencoding::getPrettyRepr(decodedTorrentFile);
    // std::cout << prettyRepr << std::endl;
}

/**
 * Retrieves the BItem which has the given key in the decoded Torrent file.
 * Traverses all the key-value pairs in the parsed dictionary, including
 * sub-dictionaries (i.e. dictionaries which are values).
 */
std::shared_ptr<bencoding::BItem> TorrentFileParser::get(std::string key)
{
    std::shared_ptr<bencoding::BItem> value = root->getValue(key);
    if (value)
    {
        return value;
    }
    return std::shared_ptr<bencoding::BItem>();
}


/**
 * Returns the info hash of the Torrent file as a string.
 * The description of what the info hash is can be found in the following post:
 * https://stackoverflow.com/questions/28348678/what-exactly-is-the-info-hash-in-a-torrent-file.
 * The sha1 function comes from http://www.zedwood.com/article/cpp-sha1-function.
 */
std::string TorrentFileParser::getInfoHash()
{
    std::shared_ptr<bencoding::BItem> infoDictionary = get("info");
    std::string infoString = bencoding::encode(infoDictionary);
    std::string sha1Hash = sha1(infoString);
    return sha1Hash;
}

/**
 * Splits the string representation of the value of 'pieces' into
 * a two dimensional byte array.
 * @param numHashes: total number of hashes.
 * @param piecesString: the string representation of pieces.
 * @param buffer a byte array whose memory has already been allocated.
 */
void TorrentFileParser::splitPieceHashes(int numHashes, std::string piecesString, byte** buffer)
{
    const int hashLength = 20;
    for (int i = 0; i < numHashes; i++)
    {
        memcpy(buffer[i], &piecesString[i * hashLength], hashLength);
    }
}