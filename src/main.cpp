#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>
#include <map>
#include <ctime>

#include <netinet/in.h>
#include "TorrentClient.h"

int main() {
    TorrentClient torrentClient;
    // torrentClient.downloadFile("../res/kali-linux-2020.4-vbox-amd64.ova.torrent");
    torrentClient.downloadFile("../res/ComputerNetworks.torrent", "../res/");
    return 0;
}
