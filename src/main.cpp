#include <iostream>
#include "TorrentClient.h"

int main() {
    TorrentClient torrentClient;
    torrentClient.downloadFile("../res/MoralPsychHandbook.torrent");
    return 0;
}
