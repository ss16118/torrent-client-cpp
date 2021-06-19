#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>
#include <map>
#include <ctime>
#include <limits>

#include <netinet/in.h>
#include "TorrentClient.h"

int main() {
    TorrentClient torrentClient(8, false);
    // torrentClient.downloadFile("../res/kali-linux-2020.4-vbox-amd64.ova.torrent");
    torrentClient.downloadFile("../res/ubuntu-21.04-desktop-amd64.iso.torrent", "../res/");
    // torrentClient.downloadFile("../res/ComputerNetworks.torrent", "../res/");
    return 0;
}
