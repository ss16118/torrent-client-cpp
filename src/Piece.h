//
// Created by siyuan on 24/05/2021.
//

#ifndef BITTORRENTCLIENT_PIECE_H
#define BITTORRENTCLIENT_PIECE_H

#include "Block.h"

/**
 * A class representation of a piece of the Torrent content.
 * Each piece except the final one has a length equal to the
 * value specified by the 'piece length' attribute in the
 * Torrent file.
 * A piece is what is defined in the torrent meta-data. However,
 * when sharing data between peers a smaller unit is used - this
 * smaller piece is refereed to as `Block` by the unofficial
 * specification.
 * The implementation is based on the Python code from the
 * following repository:
 * https://github.com/eliasson/pieces/
 */
class Piece
{
private:
    const std::string hashValue;

public:
    const int index;
    std::vector<Block*> blocks;

    explicit Piece(int index, std::vector<Block*> blocks, std::string hashValue);
    ~Piece();
    void reset();
    std::string getData();
    Block* nextRequest();
    void blockReceived(int offset, std::string data);
    bool isComplete();
    bool isHashMatching();
};

#endif //BITTORRENTCLIENT_PIECE_H
