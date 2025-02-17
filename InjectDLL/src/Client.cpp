#pragma once

#include "Connectivity.h"
#include <string>
#include <cstring>   // For memset
#include <unistd.h>  // For close()
#include <arpa/inet.h> // For inet_pton, htons
#include <sys/socket.h>

using namespace Connectivity;

void Client::connectToServer(std::string IP, std::string PORT)
{
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1) {
        perror("Socket creation failed");
        return;
    }

    memset(&addr, 0, sizeof(addr)); // Clear structure before using it
    inet_pton(AF_INET, IP.c_str(), &addr.sin_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::strtol(PORT.c_str(), nullptr, 10));

    if (connect(server, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Connection failed");
        close(server);
    }
}

void Client::sendMessage(std::string command, std::string message)
{
    int dataSize = message.size();
    std::string messageToSend = std::to_string(dataSize);
    
    int numberOfZeros = 5 - messageToSend.size();
    messageToSend.insert(0, numberOfZeros, '0');

    messageToSend += command;
    numberOfZeros = 11 - command.size();
    messageToSend.append(numberOfZeros, '0');

    messageToSend += ";" + message + "END";

    std::memcpy(buffer, messageToSend.c_str(), messageToSend.size());

    send(server, buffer, sizeof(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
}

std::string Client::receive()
{
    std::string appendable = "";
    bool completeMessage = false;

    while (!completeMessage)
    {
        recv(server, buffer, sizeof(buffer), 0);
        appendable += buffer;
        memset(buffer, 0, sizeof(buffer));

        if (std::count(appendable.begin(), appendable.end(), '{') != std::count(appendable.begin(), appendable.end(), '}') &&
            std::count(appendable.begin(), appendable.end(), '{') > 1)
            continue;

        completeMessage = true;
    }

    return appendable;
}

void Client::sendBytes(uint8_t Message[7168])
{
    const char* CharMessage = reinterpret_cast<const char*>(Message);
    send(server, CharMessage, 7168, 0);
}

void Client::receiveBytes(uint8_t* Output)
{
    int totalReceived = 0;
    int received = recv(server, buffer, 7168, 0);
    short msgLength = 0;

    memcpy(&msgLength, &buffer[0], 2);
    memcpy(&Output[0], &buffer[2], received - 2);
    totalReceived += received - 2;

    while (totalReceived < msgLength)
    {
        received = recv(server, buffer, msgLength - totalReceived, 0);
        memcpy(&Output[totalReceived], buffer, received);
        totalReceived += received;
    }
}

void Client::closeConnection()
{
    close(server); // Linux uses close() instead of closesocket()
}
