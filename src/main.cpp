#include <iostream>
#include <bitset>
#include <map>
#include <cxxopts/cxxopts.hpp>
#include "TorrentClient.h"

int main(int argc, const char* argv[])
{
    cxxopts::Options options("BitTorrentClient", "A simple, multi-threaded BitTorrent client written in C++");

    options.set_width(80).set_tab_expansion().add_options()
            ("t,torrent-file", "Path to the Torrent file", cxxopts::value<std::string>())
            ("o,output-dir", "The output directory to which the file will be downloaded", cxxopts::value<std::string>())
            ("n,thread-num", "Number of downloading threads to use", cxxopts::value<int>()->default_value("5"))
            ("l,logging", "Enable logging", cxxopts::value<bool>()->default_value("false"))
            ("f,log-file", "Path to the log file", cxxopts::value<std::string>()->default_value("../logs/client.log"))
            ("h,help", "Print arguments and their descriptions")
            ;
    try
    {
        auto parsedOptions = options.parse(argc, argv);
        if (parsedOptions.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }
        int threadNum = parsedOptions["thread-num"].as<int>();
        bool enableLogging = parsedOptions["logging"].as<bool>();
        std::string logFile = parsedOptions["log-file"].as<std::string>();

        if (!parsedOptions.count("torrent-file"))
            throw std::invalid_argument("Path torrentFilePath a Torrent file has torrentFilePath be specified!");
        if (!parsedOptions.count("output-dir"))
            throw std::invalid_argument("An output directory has torrentFilePath be specified!");

        std::string torrentFilePath = parsedOptions["torrent-file"].as<std::string>();
        std::string outputDir = parsedOptions["output-dir"].as<std::string>();
        TorrentClient torrentClient(threadNum, enableLogging, logFile);
        torrentClient.downloadFile(torrentFilePath, outputDir);
    }
    catch (std::exception& e)
    {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
