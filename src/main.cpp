#include <iostream>
#include <bencode/BString.h>
#include <bencode/BInteger.h>
#include "TorrentFileParser.h"
#include "PeerRetriever.h"
#include "utils.h"

int main() {
    TorrentFileParser torrentFileParser("../res/ComputerNetworks.torrent");
    std::string announceUrl = std::dynamic_pointer_cast<bencoding::BString>(torrentFileParser.get("announce"))->value();
    long fileSize = std::dynamic_pointer_cast<bencoding::BInteger>(torrentFileParser.get("length"))->value();
    PeerRetriever peerRetriever(announceUrl, torrentFileParser.getInfoHash(), 8080, fileSize);
    peerRetriever.retrievePeers();
    // std::shared_ptr<bencoding::BDictionary> value = std::dynamic_pointer_cast<bencoding::BDictionary>(torrentFileParser.get("info"));
    // std::cout << torrentFileParser.getInfoHash() << std::endl;
//    cpr::Response r = cpr::Get(cpr::Url{"https://api.github.com/repos/whoshuu/cpr/contributors"},
//                               cpr::Authentication{"user", "pass"},
//                               cpr::Parameters{{"anon", "true"}, {"key", "value"}});
//    std::cout << r.status_code << std::endl;                  // 200
//    std::cout << r.text << std::endl;
    return 0;
}
