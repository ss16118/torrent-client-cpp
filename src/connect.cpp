//
// Created by siyuan on 13/05/2021.
//

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include "connect.h"
#include "utils.h"

/**
 * Starts a TCP connection with the given IP address and port number. Returns the
 * socket of the connection if successful.
 * @param ip: IP address of the host.
 * @param port: port number of the host.
 * @return socket number of the created connection.
 */
int createConnection(const std::string& ip, const int port)
{
    int sock = 0;
    struct sockaddr_in address;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error("Socket creation error: " + std::to_string(sock));

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    char* tempIp = new char[ip.length() + 1];
    strcpy(tempIp, ip.c_str());

    // Converts IP address from string to struct in_addr
    if (inet_pton(AF_INET, tempIp, &address.sin_addr) <= 0)
        throw std::runtime_error("Invalid IP address: " + ip);

    if (connect(sock, (struct sockaddr *) &address, sizeof(address)) < 0 )
        throw std::runtime_error("Failed to connect to " + ip);

    return sock;
}


/**
 * Writes data to the given socket.
 * @param sock: socket number.
 * @param data: data to be written (sent) to the socket.
 */
void sendData(const int sock, const std::string data)
{
    int n = data.length();
    char buffer[n];
    for (int i = 0; i < n; i++)
        buffer[i] = data[i];

    int res = send(sock, buffer, n, 0);
    if (res < 0)
        throw std::runtime_error("Failed to write data to socket " + std::to_string(sock));
}


/**
 * Receives data from the connected host. If the bufferSize argument is None, the first 4 bytes
 * of the received message will be read and interpreted as an integer that specifies
 * the total length of the message.
 * @param sock: socket number that specifies the connection to the host.
 * @param bufferSize: size of the receiving buffer.
 * @return received data in the form a string.
 */
std::string receiveData(const int sock, int bufferSize)
{
    // If buffer size is not specified, read the first 4 bytes of the message
    // to obtain the total length of the response.
    if (!bufferSize)
    {
        const int lengthIndicatorSize = 4;
        char buffer[lengthIndicatorSize];
        if (read(sock, buffer, sizeof(buffer)) < 0)
            throw std::runtime_error("Failed to read total length of the message from socket " + std::to_string(sock));
        uint32_t messageLength = 0;
        for (int i = 0; i < lengthIndicatorSize; i++)
            messageLength |= buffer[i] << ((lengthIndicatorSize - 1 - i) * 8);
        // std::cout << "[DEBUG] Message length: " << std::to_string(messageLength) << std::endl;
        bufferSize = (int) messageLength;
    }

    char buffer[bufferSize];
    memset(buffer, NULL, sizeof(buffer));
    std::string reply;
    // Receives reply from the host
    // Keep reading from the buffer all required bytes are received
    int bytesRead = 0;
    int bytesToRead = bufferSize - bytesRead;
    do
    {
        bytesRead = read(sock, buffer, sizeof(buffer));
        bytesToRead -= bytesRead;
        if (bytesRead < 0)
            throw std::runtime_error("Failed to receive data from socket " + std::to_string(sock));
        for (int i = 0; i < bytesRead; i++)
            reply.push_back(buffer[i]);
    }
    while (bytesToRead > 0);
    return reply;
}