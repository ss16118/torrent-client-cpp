//
// Created by siyuan on 04/05/2021.
//

#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include <random>
#include <stdexcept>
#include <bitset>
#include <bencode/bencoding.h>

#include "utils.h"
#include "PeerRetriever.h"

/**
 * Constructor of the class PeerRetriever. Takes in the URL as specified by the
 * value of announce in the Torrent file, the info hash of the file, as well as
 * a port number.
 * @param announceURL: the HTTP URL to the tracker.
 * @param infoHash: the info hash of the Torrent file.
 * @param port: the TCP port this client listens on.
 */
PeerRetriever::PeerRetriever(std::string announceUrl, std::string infoHash, int port, long fileSize)
{
    this->announceUrl = announceUrl;
    this->infoHash = infoHash;
    this->port = port;
    this->fileSize = fileSize;

    // Generate a random 20-byte peer Id for the client as per the convention described
    // on the following web page.
    // https://wiki.theory.org/BitTorrentSpecification#peer_id
    peerId = "-UT2021-";
    // Generate 12 random numbers
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(1, 9);
    for (int i = 0; i < 12; i++)
        peerId += std::to_string(distrib(gen));

    // std::cout << peerId << std::endl;
}

/**
 * Retrieves the list of peers from the URL specified by the 'announce' property.
 * The list of parameters and their descriptions are as follows
 * (found on this page https://markuseliasson.se/article/bittorrent-in-python/):
 * - info_hash: the SHA1 hash of the info dict found in the .torrent.
 * - peer_id: a unique ID generated for this client.
 * - uploaded: the total number of bytes uploaded.
 * - downloaded: the total number of bytes downloaded.
 * - left: the number of bytes left to download for this client.
 * - port: the TCP port this client listens on.
 * - compact: whether or not the client accepts a compacted list of peers or not.
 * @return a vector that contains the information of all peers.
 */
std::vector<Peer> PeerRetriever::retrievePeers()
{
    // std::cout << "Announce: " << std::string(announceUrl) << std::endl;
    // std::cout << "Peer ID: " << std::string(peerId) << std::endl;
    // std::cout << "File size: " <<  std::to_string(fileSize) << std::endl;
    std::cout << "Retrieving peers from " << announceUrl << " with the following parameters..." << std::endl;
    // Note that info hash will be URL-encoded by the cpr library
    std::cout << "info_hash: " << infoHash << std::endl;
    std::cout << "peer_id: " << peerId << std::endl;
    std::cout << "port: " << port << std::endl;
    std::cout << "uploaded: " << 0 << std::endl;
    std::cout << "downloaded: " << 0 << std::endl;
    std::cout << "left: " << std::to_string(fileSize) << std::endl;
    std::cout << "compact: " << std::to_string(1) << std::endl;

    cpr::Response res = cpr::Get(cpr::Url{announceUrl}, cpr::Parameters {
        { "info_hash", std::string(hexDecode(infoHash)) },
        { "peer_id", std::string(peerId) },
        { "port", std::to_string(port) },
        { "uploaded", std::to_string(0) },
        { "downloaded", std::to_string(0) },
        { "left", std::to_string(fileSize) },
        { "compact", std::to_string(1) }
    });

    // If response successfully retrieved
    if (res.status_code == 200)
    {
        std::cout << "Retrieving response from tracker: SUCCESS" << std::endl;
        // std::shared_ptr<bencoding::BItem> decodedResponse = bencoding::decode(res.text);
        // std::string formattedResponse = bencoding::getPrettyRepr(decodedResponse);
        // std::cout << formattedResponse << std::endl;
        std::vector<Peer> peers = decodeResponse(res.text);
        return peers;
    }
    else
    {
        std::cout << "Retrieving response from tracker: FAILED [ " << res.text << " ]" << std::endl;
    }
    return std::vector<Peer>();
}

/**
 * Decodes the response string sent by the tracker. If the string can successfully decoded,
 * returns a list of Peer structs. Note that this functions handles two distinct representations,
 * one of them has the peers denoted as a long binary blob, the other represents peers in a list
 * with all the information already in place. The former can be seen in the response of
 * the kali-linux tracker, whereas the latter can be found in the tracker response of the
 * other two files.
 */
std::vector<Peer> PeerRetriever::decodeResponse(std::string response) {
    std::cout << "Decoding tracker response..." << std::endl;
    std::shared_ptr<bencoding::BItem> decodedResponse = bencoding::decode(response);

    std::shared_ptr<bencoding::BDictionary> responseDict =
            std::dynamic_pointer_cast<bencoding::BDictionary>(decodedResponse);
    std::shared_ptr<bencoding::BItem> peersValue = responseDict->getValue("peers");
    if (!peersValue)
        throw std::runtime_error("Response returned by the tracker is not in the correct format. ['peers' not found]");

    std::vector<Peer> peers;

    // Handles the first case where peer information is sent in a binary blob
    if (typeid(*peersValue) == typeid(bencoding::BString))
    {
        // Unmarshalls the peer information:
        // Detailed explanation can be found here:
        // https://blog.jse.li/posts/torrent/
        // Essentially, every 6 bytes represent a single peer with the first 4 bytes being the IP
        // and the last 2 bytes being the port number.
        const int peerInfoSize = 6;
        std::string peersString = std::dynamic_pointer_cast<bencoding::BString>(peersValue)->value();

        if (peersString.length() % peerInfoSize != 0)
            throw std::runtime_error(
                    "Received malformed 'peers' from tracker. ['peers' length needs to be divisible by 6]");

        const int peerNum = peersString.length() / peerInfoSize;
        for (int i = 0; i < peerNum; i++)
        {
            int offset = i * peerInfoSize;
            std::stringstream peerIp;
            peerIp << std::to_string((uint8_t) peersString[offset]) << ".";
            peerIp << std::to_string((uint8_t) peersString[offset + 1]) << ".";
            peerIp << std::to_string((uint8_t) peersString[offset + 2]) << ".";
            peerIp << std::to_string((uint8_t) peersString[offset + 3]);
            std::string portBinStr =
                    std::bitset<8>(peersString[offset + 4]).to_string() +
                    std::bitset<8>(peersString[offset + 5]).to_string();
            int peerPort = stoi(portBinStr, 0, 2);
            // std::cout << "IP: " << peerIp.str() << std::endl;
            // std::cout << "Port: " << std::to_string(peerPort) << std::endl;
            peers.push_back(Peer { peerIp.str(), peerPort });
        }
        return peers;
    }
    // Handles the second case where peer information is stored in a list
    else if (typeid(*peersValue) == typeid(bencoding::BList))
    {
        std::shared_ptr<bencoding::BList> peerList = std::dynamic_pointer_cast<bencoding::BList>(peersValue);
        for (auto &item : *peerList)
        {
            // Casts each item to a dictionary
            std::shared_ptr<bencoding::BDictionary> peerDict = std::dynamic_pointer_cast<bencoding::BDictionary>(item);

            // Gets peer ip from the dictionary
            std::shared_ptr<bencoding::BItem> tempPeerIp = peerDict->getValue("ip");

            if (!tempPeerIp)
                throw std::runtime_error("Received malformed 'peers' from tracker. [Item does not contain key 'ip']");

            std::string peerIp = std::dynamic_pointer_cast<bencoding::BString>(tempPeerIp)->value();
            // Gets peer port from the dictionary
            std::shared_ptr<bencoding::BItem> tempPeerPort = peerDict->getValue("port");
            if (!tempPeerPort)
                throw std::runtime_error("Received malformed 'peers' from tracker. [Item does not contain key 'port']");
            int peerPort = (int) std::dynamic_pointer_cast<bencoding::BInteger>(tempPeerPort)->value();
            peers.push_back(Peer { peerIp, peerPort });
        }
    }
    else
    {
        throw std::runtime_error(
                "Response returned by the tracker is not in the correct format. ['peers' has the wrong type]");
    }
    std::cout << "Decoding tracker response: SUCCESS" << std::endl;
    std::cout << "Number of peers discovered: " << std::to_string(peers.size()) << std::endl;
    return peers;
}

