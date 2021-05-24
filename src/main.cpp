#include <iostream>
#include <string>
#include <map>
#include "TorrentClient.h"
#include "utils.h"
#include "PieceManager.h"

int main() {
    TorrentClient torrentClient;
    // torrentClient.downloadFile("../res/kali-linux-2020.4-vbox-amd64.ova.torrent");
    torrentClient.downloadFile("../res/MoralPsychHandbook.torrent");
    return 0;
}
