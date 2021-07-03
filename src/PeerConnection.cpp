//
// Created by siyuan on 08/05/2021.
//

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <netinet/in.h>
#include <loguru/loguru.hpp>
#include <utility>

#include "PeerConnection.h"
#include "utils.h"
#include "connect.h"

#define INFO_HASH_STARTING_POS 28
#define PEER_ID_STARTING_POS 48
#define HASH_LEN 20
#define DUMMY_PEER_IP "0.0.0.0"

/**
 * Constructor of the class PeerConnection.
 * @param queue: the thread-safe queue that contains the available peers.
 * @param clientId: the peer ID of this C++ BitTorrent client. Generated in the TorrentClient class.
 * @param infoHash: info hash of the Torrent file.
 * @param pieceManager: pointer to the PieceManager.
 */
PeerConnection::PeerConnection(
    SharedQueue<Peer*>* queue,
    std::string clientId,
    std::string infoHash,
    PieceManager* pieceManager
) : queue(queue), clientId(std::move(clientId)), infoHash(std::move(infoHash)), pieceManager(pieceManager) {}


/**
 * Destructor of the PeerConnection class. Closes the established TCP connection with the peer
 * on object destruction.
 */
PeerConnection::~PeerConnection() {
    closeSock();
    LOG_F(INFO, "Downloading thread terminated");
}


void PeerConnection::start() {
    LOG_F(INFO, "Downloading thread started...");
    while (!(terminated || pieceManager->isComplete()))
    {
        peer = queue->pop_front();
        // Terminates the thread if it has received a dummy Peer
        if (peer->ip == DUMMY_PEER_IP)
            return;

        try
        {
            // Establishes connection with the peer, and lets it know
            // that we are interested.
            if (establishNewConnection())
            {
                while (!pieceManager->isComplete())
                {
                    BitTorrentMessage message = receiveMessage();
                    if (message.getMessageId() > 10)
                        throw std::runtime_error("Received invalid message Id from peer " + peerId);
                    switch (message.getMessageId())
                    {
                        case choke:
                            choked = true;
                            break;

                        case unchoke:
                            choked = false;
                            break;

                        case piece:
                        {
                            requestPending = false;
                            std::string payload = message.getPayload();
                            int index = bytesToInt(payload.substr(0, 4));
                            int begin = bytesToInt(payload.substr(4, 4));
                            std::string blockData = payload.substr(8);
                            pieceManager->blockReceived(peerId, index, begin, blockData);
                            break;
                        }
                        case have:
                        {
                            std::string payload = message.getPayload();
                            int pieceIndex = bytesToInt(payload);
                            pieceManager->updatePeer(peerId, pieceIndex);
                            break;
                        }

                        default:
                            break;
                    }
                    if (!choked)
                    {
                        if (!requestPending)
                        {
                            requestPiece();
                        }
                    }
                }
            }
        }
        catch (std::exception &e)
        {
            closeSock();
            LOG_F(ERROR, "An error occurred while downloading from peer %s [%s]", peerId.c_str(), peer->ip.c_str());
            LOG_F(ERROR, "%s", e.what());
        }
    }
}

/**
 * Terminates the peer connection
 */
void PeerConnection::stop()
{
    terminated = true;
}


/**
 * Establishes a TCP connection with the peer and sent it our initial BitTorrent handshake message.
 * Waits for its reply, and compares the info hash contained in its response message with
 * the info hash we calculated from the Torrent file. If they do not match, close the connection.
 */
void PeerConnection::performHandshake()
{
    // Connects to the peer
    LOG_F(INFO, "Connecting to peer [%s]...", peer->ip.c_str());
    try
    {
        sock = createConnection(peer->ip, peer->port);
    }
    catch (std::runtime_error &e)
    {
        throw std::runtime_error("Cannot connect to peer [" + peer->ip + "]");
    }
    LOG_F(INFO, "Establish TCP connection with peer at socket %d: SUCCESS", sock);

    // Send the handshake message to the peer
    LOG_F(INFO, "Sending handshake message to [%s]...", peer->ip.c_str());
    std::string handshakeMessage = createHandshakeMessage();
    sendData(sock, handshakeMessage);
    LOG_F(INFO, "Send handshake message: SUCCESS");

    // Receive the reply from the peer
    LOG_F(INFO, "Receiving handshake reply from peer [%s]...", peer->ip.c_str());
    std::string reply = receiveData(sock, handshakeMessage.length());
    if (reply.empty())
        throw std::runtime_error("Receive handshake from peer: FAILED [No response from peer]");
    peerId = reply.substr(PEER_ID_STARTING_POS, HASH_LEN);
    LOG_F(INFO, "Receive handshake reply from peer: SUCCESS");

    // Compare the info hash from the peer's reply message with the info hash we sent.
    // If the two values are not the same, close the connection and raise an exception.
    std::string receivedInfoHash = reply.substr(INFO_HASH_STARTING_POS, HASH_LEN);
    if ((receivedInfoHash == infoHash) != 0)
        throw std::runtime_error("Perform handshake with peer " + peer->ip +
                                 ": FAILED [Received mismatching info hash]");
    LOG_F(INFO, "Hash comparison: SUCCESS");
}

/**
 * Receives and reads the message which contains BitField from the peer.
 */
void PeerConnection::receiveBitField()
{
    // Receive BitField from the peer
    LOG_F(INFO, "Receiving BitField message from peer [%s]...", peer->ip.c_str());
    BitTorrentMessage message = receiveMessage();
    if (message.getMessageId() != bitField)
        throw std::runtime_error("Receive BitField from peer: FAILED [Wrong message ID]");
    peerBitField = message.getPayload();

    // Informs the PieceManager of the BitField received
    pieceManager->addPeer(peerId, peerBitField);

    LOG_F(INFO, "Receive BitField from peer: SUCCESS");
}

/**
 * Sends a request message to the peer for the next block
 * to be downloaded.
 */
void PeerConnection::requestPiece() {
    Block* block = pieceManager->nextRequest(peerId);

    if (!block)
        return;

    int payloadLength = 12;
    char temp[payloadLength];
    // Needs to convert little-endian to big-endian
    uint32_t index = htonl(block->piece);
    uint32_t offset = htonl(block->offset);
    uint32_t length = htonl(block->length);
    std::memcpy(temp, &index, sizeof(int));
    std::memcpy(temp + 4, &offset, sizeof(int));
    std::memcpy(temp + 8, &length, sizeof(int));
    std::string payload;
    for (int i = 0; i < payloadLength; i++)
        payload += (char) temp[i];

    std::stringstream info;
    info << "Sending Request message to peer " << peer->ip << " ";
    info << "[Piece: " << std::to_string(block->piece) << " ";
    info << "Offset: " << std::to_string(block->offset) << " ";
    info << "Length: " << std::to_string(block->length) << "]";
    LOG_F(INFO, "%s", info.str().c_str());
    std::string requestMessage = BitTorrentMessage(request, payload).toString();
    sendData(sock, requestMessage);
    requestPending = true;
    LOG_F(INFO, "Send Request message: SUCCESS");
}


/**
 * Send an Interested message to the peer.
 */
void PeerConnection::sendInterested()
{
    LOG_F(INFO, "Sending Interested message to peer [%s]...", peer->ip.c_str());
    std::string interestedMessage = BitTorrentMessage(interested).toString();
    sendData(sock, interestedMessage);
    LOG_F(INFO, "Send Interested message: SUCCESS");
}

/**
 * Receives and reads the Unchoke message from the peer.
 * If the received message does not match the expected Unchoke, raise an error.
 */
void PeerConnection::receiveUnchoke() {
    LOG_F(INFO, "Receiving Unchoke message from peer [%s]...", peer->ip.c_str());
    BitTorrentMessage message = receiveMessage();
    if (message.getMessageId() != unchoke)
        throw std::runtime_error(
                "Receive Unchoke message from peer: FAILED [Wrong message ID: " +
                std::to_string(message.getMessageId()) + "]");
    choked = false;
    LOG_F(INFO, "Receive Unchoke message: SUCCESS");
}

/**
 * This function establishes a TCP connection with the peer and performs
 * the following actions:
 *
 * 1. Sends the peer a BitTorrent handshake message, waits for its reply and
 * compares the info hashes.
 * 2. Receives and stores the BitField from the peer.
 * 3. Send an Interested message to the peer.
 *
 * Returns true if a stable connection has been successfully established,
 * false otherwise.
 *
 * To understand the details, the following links can be helpful:
 * - https://blog.jse.li/posts/torrent/
 * - https://markuseliasson.se/article/bittorrent-in-python/
 * - https://wiki.theory.org/BitTorrentSpecification#Handshake
 */
bool PeerConnection::establishNewConnection() {
    try
    {
        performHandshake();
        receiveBitField();
        sendInterested();
        return true;
    }
    catch (const std::runtime_error& e)
    {
        LOG_F(ERROR, "An error occurred while connecting with peer [%s]", peer->ip.c_str());
        LOG_F(ERROR, "%s", e.what());
        return false;
    }
}

/**
 * Generates the initial handshake message to send to the peer.
 * Essentially, the handshake message has the following structure:
 *
 * handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
 * pstrlen: string length of <pstr>, as a single raw byte
 * pstr: string identifier of the protocol
 * reserved: eight (8) reserved bytes.
 * info_hash: 20-byte SHA1 hash Torrent file.
 * peer_id: 20-byte string used as a unique ID for the client.
 *
 * The detailed description can be found at:
 * https://wiki.theory.org/BitTorrentSpecification#Handshake
 *
 * @return a string representation of the Torrent handshake message.
 */
std::string PeerConnection::createHandshakeMessage()
{
    const std::string protocol = "BitTorrent protocol";
    std::stringstream buffer;
    buffer << (char) protocol.length();
    buffer << protocol;
    std::string reserved;
    for (int i = 0; i < 8; i++)
        reserved.push_back('\0');
    buffer << reserved;
    buffer << hexDecode(infoHash);
    buffer << clientId;
    assert (buffer.str().length() == protocol.length() + 49);
    return buffer.str();
}


/**
 * A wrapper around the receiveData() function, in a sense that it returns
 * a BitTorrentMessage object so that parameters such as message length, id
 * and payload can be accessed more easily.
 */
BitTorrentMessage PeerConnection::receiveMessage(int bufferSize) const {
    std::string reply = receiveData(sock, 0);
    if (reply.empty())
        return BitTorrentMessage(keepAlive);
    auto messageId = (uint8_t) reply[0];
    std::string payload = reply.substr(1);
    LOG_F(INFO, "Received message with ID %d from peer [%s]", messageId, peer->ip.c_str());
    return BitTorrentMessage(messageId, payload);
}

/**
 * Retrieves the peer ID of the peer that is currently in contact with us.
 */
const std::string &PeerConnection::getPeerId() const {
    return peerId;
}

/**
 * Closes the socket to a peer and sets the
 * instance variable 'sock' to null;
 */
void PeerConnection::closeSock()
{
    if (sock)
    {
        // Close socket
        LOG_F(INFO, "Closed connection at socket %d", sock);
        close(sock);
        sock = {};
        requestPending = false;
        // If the peer has been added to piece manager, remove it
        if (!peerBitField.empty())
        {
            peerBitField.clear();
            pieceManager->removePeer(peerId);
        }
    }
}
