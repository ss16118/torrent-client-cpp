//
// Created by siyuan on 16/05/2021.
//
#include <iostream>
#include <sstream>
#include <bitset>
#include "BitTorrentMessage.h"

/**
 * An object that represents different types of BitTorrent messages.
 * A detailed descriptions of the messages can be found at the
 * following page:
 * https://wiki.theory.org/BitTorrentSpecification
 * @param id: message ID, indicates the type of BitTorrent message.
 * @param payload: payload of the message in a string format.
 */
BitTorrentMessage::BitTorrentMessage(const uint8_t id, const std::string& payload):
        id(id), payload(payload), messageLength(payload.length() + 1) {}

std::string BitTorrentMessage::toString() {
    std::stringstream buffer;
    char* messageLengthAddr = (char*) &messageLength;
    std::string messageLengthStr;
    // Bytes are pushed in reverse order, assuming the data
    // is stored in little-endian order locally.
    for (int i = 0; i < 4; i++)
        messageLengthStr.push_back((char) messageLengthAddr[3 - i]);
    buffer << messageLengthStr;
    buffer << (char) id;
    buffer << payload;
    return buffer.str();
}

/**
 * A getter function for the attribute Id.
 * @return the Id of the BitTorrentMessage instance.
 */
uint8_t BitTorrentMessage::getMessageId() const {
    return id;
}

/**
 * A getter function for the attribute payload.
 * @return the payload of the BitTorrentMessage instance.
 */
std::string BitTorrentMessage::getPayload() const {
    return payload;
}
