#include <iostream>
#include <fstream>
#include "TorrentFileParser.h"
#include <bencode/BString.h>

int main() {
    TorrentFileParser torrentFileParser("../kali-linux-2020.4-vbox-amd64.ova.torrent");
    // std::shared_ptr<bencoding::BDictionary> value = std::dynamic_pointer_cast<bencoding::BDictionary>(torrentFileParser.get("info"));
    std::cout << torrentFileParser.getInfoHash() << std::endl;
    return 0;
}
