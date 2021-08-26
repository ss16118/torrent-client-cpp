# Torrent Client C++
This is a simple BitTorrent client written in C++. The code and the general structure of the program are primarily based on the following blog posts:

- [Building a BitTorrent client from the ground up in Go](https://blog.jse.li/posts/torrent/)
- [A BitTorrent client in Python 3.5](https://markuseliasson.se/article/bittorrent-in-python/)


I have found both posts in [this repository](https://github.com/danistefanovic/build-your-own-x), which contains numerous tutorials. It is definitely worth looking into for anyone who wants to implement their own tools from ground up.

Note that this is my very first C++ project, which I mainly started to familiarize myself with the language. Hence, the code might not comply with the normal C++ styles, and there are undoubtedly places where the code can be optimized.


 Quick start
==========================
If you want to try this BitTorrent client yourself, follow the steps below:
- Clone this repository, and make sure that cmake (> 3.19.2) is installed.
- Type the following commands into the terminal (note that fetching the `cpr` library will take some time, so the compilation process might be time-consuming.): 

```console
$ cd torrent-client-cpp && mkdir build && cd build
$ cmake .. && make -j
```
- To download the ubuntu iso as an example, enter the following command:
```console
$ ./BitTorrentClient -t ../res/ubuntu-12.04.5-alternate-amd64.iso.torrent -o ../res/  # logging disabled
$ ./BitTorrentClient -t ../res/ubuntu-12.04.5-alternate-amd64.iso.torrent -o ../res/  -l # logging enabled, log can be found in logs/client.log
```
- The output will be similar to this:
```console
Parsing Torrent file ../res/ubuntu-12.04.5-alternate-amd64.iso.torrent...
Download initiated...
[Peers: 4/5, 0.52 MB/s, ETA: 25:18][>                                       ] 4/1522 [0.26%] in 00:13
```
- All files in the `res` directory can be used for testing. The files `MoralPsychHandbooks.torrent` and `ComputerNetworks.torrent` all come from [Academic Torrents](https://academictorrents.com/). They are small Torrent files which are suitable for testing all the basic functionalities of the BitTorrent client. Whereas the Torrent files for ubuntu and debian are great for stress tests.


Commandline Options
==========================
The program supports the following commandline options:

| Options | Alternative    | Description                                                                                        | Default            |
|---------|----------------|----------------------------------------------------------------------------------------------------|--------------------|
| -t      | --torrent-file | Path to the Torrent file                                                                           | REQUIRED           |
| -o      | --output-dir   | The output directory to which the file will be downloaded                                          | REQUIRED           |
| -n      | --thread-num   | Number of downloading threads to use. (i.e maximum number of peers that the client can connect to) | 5                  |
| -l      | --logging      | Enable logging                                                                                     | false              |
| -f      | --log-file     | Path to the log file                                                                               | ../logs/client.log |
| -h      | --help         | Print arguments and their descriptions                                                             |                    |


Supported Features
==========================
The current implementation of this BitTorrent client only supports the following features:
- Retrieving a list of peers from the tracker periodically.
- Downloading single-file Torrents in a multi-threaded manner.

To make it an actual usable BitTorrent client, it will have to include:
- Seeding
- Resuming a download.
- Downloading multi-file Torrents
- Probably a more intuitive user interface.
- Pipelining when requesting blocks from peers.
- Connecting to as many peers as possible.

Since this is only a project that I started for fun and to learn C++, it is unlikely that any of the unsupported features will be implemented any time soon. If you wish to build on top of my current code, feel free to do so.

