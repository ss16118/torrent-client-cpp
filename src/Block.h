//
// Created by siyuan on 24/05/2021.
//

#ifndef BITTORRENTCLIENT_BLOCK_H
#define BITTORRENTCLIENT_BLOCK_H

enum BlockStatus
{
    missing = 0,
    pending = 1,
    retrieved = 2
};

/**
 * A part of a piece that is requested and transferred
 * between peers.
 * A Block, by convention, usually has the size of 2 ^ 14 bytes,
 * except for the last Block in a piece.
 */
struct Block
{
    int piece;
    int offset;
    int length;
    BlockStatus status;
    std::string data;
};

#endif //BITTORRENTCLIENT_BLOCK_H
