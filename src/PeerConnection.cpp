//
// Created by siyuan on 08/05/2021.
//

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <unistd.h>

#include "PeerConnection.h"
#include "utils.h"
#include "connect.h"

#define INFO_HASH_STARTING_POS 28
#define PEER_ID_STARTING_POS 48
#define HASH_LEN 20

/**
 * Constructor of the class PeerConnection.
 * @param peer: the Peer struct as defined in PeerRetriever.h.
 * @param clientId: the peer ID of this C++ BitTorrent client. Generated in the TorrentClient class.
 * @param infoHash: info hash of the Torrent file.
 * @param pieceManager: pointer to the PieceManager.
 */
PeerConnection::PeerConnection(
    const Peer peer,
    const std::string clientId,
    const std::string infoHash,
    PieceManager* pieceManager
) : peer(peer), clientId(clientId), infoHash(infoHash), pieceManager(pieceManager) {}


/**
 * Destructor of the PeerConnection class. Closes the established TCP connection with the peer
 * on object destruction.
 */
PeerConnection::~PeerConnection() {
    close(sock);
    std::cout << "Closed connection with peer " << peer.ip << std::endl;
}

/**
 * Establishes a TCP connection with the peer and sent it our initial BitTorrent handshake message.
 * Waits for its reply, and compares the info hash contained in its response message with
 * the info hash we calculated from the Torrent file. If they do not match, close the connection.
 */
void PeerConnection::performHandshake()
{
    // Connects to the peer
    std::cout << "Connecting to peer " + peer.ip << std::endl;
    sock = createConnection(peer.ip, peer.port);
    std::cout << "Establish TCP connection with peer " + std::to_string(sock) + ": SUCCESS" << std::endl;

    // Send the handshake message to the peer
    std::cout << "Sending handshake message to " + peer.ip << std::endl;
    std::string handshakeMessage = createHandshakeMessage();
    sendData(sock, handshakeMessage);
    std::cout << "Send handshake message: SUCCESS" << std::endl;

    // Receive the reply from the peer
    std::cout << "Receiving handshake reply from peer " + peer.ip << std::endl;
    std::string reply = receiveData(sock, handshakeMessage.length());
    std::cout << "Receive handshake reply from peer: SUCCESS" << std::endl;
    peerId = reply.substr(PEER_ID_STARTING_POS, HASH_LEN);

    // Compare the info hash from the peer's reply message with the info hash we sent.
    // If the two values are not the same, close the connection and raise an exception.
    std::string receivedInfoHash = reply.substr(INFO_HASH_STARTING_POS, HASH_LEN);
    if ((receivedInfoHash == infoHash) != 0)
        throw std::runtime_error("Perform handshake with peer " + peer.ip +
                                 ": FAILED [Received mismatching info hash]");
    std::cout << "Hash comparison: SUCCESS" << std::endl;
}

/**
 * Receives and reads the message which contains BitField from the peer.
 */
void PeerConnection::receiveBitField()
{
    // Receive BitField from the peer
    std::cout << "Receiving BitField message from peer " + peer.ip << "..." << std::endl;
    BitTorrentMessage message = receiveMessage();
    if (message.getMessageId() != bitField)
        throw std::runtime_error("Receive BitField from peer: FAILED [Wrong message ID]");
    peerBitField = message.getPayload();

    // Informs the PieceManager of the BitField received
    pieceManager->addPeer(peerId, peerBitField);

    std::cout << "Receive BitField from peer: SUCCESS" << std::endl;
}

/**
 * Send an Interested message to the peer.
 */
void PeerConnection::sendInterested()
{
    std::cout << "Sending Interested message to peer " << peer.ip << std::endl;
    std::string interestedMessage = BitTorrentMessage(interested).toString();
    sendData(sock, interestedMessage);
    std::cout << "Send Interested message: SUCCESS" << std::endl;
}

/**
 * Receives and reads the Unchoke message from the peer.
 * If the received message does not match the expected Unchoke, raise an error.
 */
void PeerConnection::receiveUnchoke() {
    std::cout << "Receiving Unchoke message from peer " << peer.ip << std::endl;
    BitTorrentMessage message = receiveMessage();
    if (message.getMessageId() != unchoke)
        throw std::runtime_error("Receive Unchoke message from peer: FAILED [Wrong message ID]");
    chocked = false;
    std::cout << "Receive Unchoke message: SUCCESS" << std::endl;
}

/**
 * This function establishes a TCP connection with peer and performs
 * the following actions:
 *
 * 1. Sends the peer a BitTorrent handshake message, waits for its reply and
 * compares the info hashes.
 * 2. Receives and stores the BitField from the peer.
 * 3. Send an Interested message to the peer.
 *
 * To understand the details, the following links can be helpful:
 * - https://blog.jse.li/posts/torrent/
 * - https://markuseliasson.se/article/bittorrent-in-python/
 * - https://wiki.theory.org/BitTorrentSpecification#Handshake
 */
void PeerConnection::establishNewConnection() {
    try
    {
        performHandshake();
        receiveBitField();
        sendInterested();
        receiveUnchoke();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "An error occurred while connecting with peer " << peer.ip << std::endl;
        std::cout << e.what() << std::endl;
        close(sock);
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
    auto messageId = (uint8_t) reply[0];
    std::string payload = reply.substr(1);
    return BitTorrentMessage(messageId, payload);
}

/**
 * Retrieves the peer ID of the peer that is currently in contact with us.
 */
const std::string &PeerConnection::getPeerId() const {
    return peerId;
}
