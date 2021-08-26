cmake_minimum_required(VERSION 3.19)
project(BitTorrentClient)

set(CMAKE_CXX_STANDARD 17)
include_directories(lib)
include_directories(include)

# Bencoding
file(GLOB BENCODING_SRC "lib/bencode/*.cpp" "include/bencode/*.h")
add_library(bencoding STATIC ${BENCODING_SRC})

# SHA1
add_library(crypto STATIC lib/crypto/sha1.h lib/crypto/sha1.cpp)

# loguru
add_library(loguru STATIC lib/loguru/loguru.cpp lib/loguru/loguru.hpp)

# cxxopts
add_library(cxxopts STATIC lib/cxxopts/cxxopts.hpp)
set_target_properties(cxxopts PROPERTIES LINKER_LANGUAGE CXX)

# cpr
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

find_package(CURL)
include_directories(${CURL_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(OPENSSL REQUIRED openssl)

if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif()

add_executable(BitTorrentClient src/main.cpp src/TorrentFileParser.cpp src/TorrentFileParser.h src/PeerRetriever.h src/PeerRetriever.cpp src/utils.cpp src/utils.h src/PeerConnection.cpp src/PeerConnection.h src/connect.cpp src/connect.h src/TorrentClient.h src/TorrentClient.cpp src/BitTorrentMessage.h src/BitTorrentMessage.cpp src/PieceManager.h src/PieceManager.cpp src/Piece.h src/Piece.cpp src/Block.h src/SharedQueue.h)

target_link_libraries(BitTorrentClient PRIVATE bencoding crypto cpr loguru cxxopts ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})