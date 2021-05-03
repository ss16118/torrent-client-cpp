cmake_minimum_required(VERSION 3.19)
project(BitTorrentClient)

set(CMAKE_CXX_STANDARD 17)
include_directories(lib)
include_directories(include)

file(GLOB BENCODING_SRC "lib/bencode/*.cpp" "include/bencode/*.h")

# Bencoding
add_library(bencoding STATIC ${BENCODING_SRC})

# SHA1
add_library(crypto STATIC lib/crypto/sha1.h lib/crypto/sha1.cpp)


add_executable(BitTorrentClient src/main.cpp src/TorrentFileParser.cpp src/TorrentFileParser.h)

target_link_libraries(BitTorrentClient PRIVATE bencoding crypto)