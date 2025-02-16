#pragma once

#pragma comment(lib, "ws2_32")
#include "Connectivity.h"
#include <string>

using namespace Connectivity;

void Client::connectToServer(std::string IP, std::string PORT)
{
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);

    inet_pton(AF_INET, IP.c_str(), &addr.sin_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::strtol(PORT.c_str(), nullptr, 10));

    connect(server, (SOCKADDR*)&addr, sizeof(addr));
}

void Client::sendMessage(std::string command, std::string message)
{
    int dataSize = message.size();

    std::string messageToSend = std::to_string(dataSize);

    int numberOfZeros = 5 - messageToSend.size();

    for (int i = 0; i < numberOfZeros; i++)
    {
        messageToSend = "0" + messageToSend;
    }

    messageToSend += command;

    numberOfZeros = 11 - command.size();

    for (int i = 0; i < numberOfZeros; i++)
    {
        messageToSend += "0";
    }

    messageToSend += ";" + message + "END";

    for (int i = 0; i < messageToSend.size(); i++)
    {
        this->buffer[i] = messageToSend[i];
    }

    send(server, buffer, sizeof(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
}

std::string Client::receive()
{

    std::string appendable = "";
    bool CompleteMessage = false;

    while (!CompleteMessage)
    {
        recv(server, buffer, sizeof(buffer), 0);
        //std::string buf = buffer;
        appendable += buffer;
        memset(buffer, 0, sizeof(buffer));

        if (std::count(appendable.begin(), appendable.end(), '{') != std::count(appendable.begin(), appendable.end(), '}') and std::count(appendable.begin(), appendable.end(), '{') > 1)
            continue;

        CompleteMessage = true;
    }

    return appendable;
}

void Client::sendBytes(byte Message[7168])
{
    const char* CharMessage = reinterpret_cast<const char*>(Message);
    send(server, CharMessage, 7168, 0);
}

void Client::receiveBytes(byte* Output)
{
    int totalReceived = 0;
    int received = recv(server, buffer, 7168, 0);
    short msgLength = 0;

    memcpy(&msgLength, &buffer[0], 2);

    memcpy(&Output[0], &buffer[0] + 2, received - 2);

    totalReceived += received - 2;

    //memset(buffer, 0, sizeof(buffer));

    while (totalReceived < msgLength)
    {
        received = recv(server, buffer, msgLength, 0);
        memcpy(&Output[0] + totalReceived, &buffer[0], received);
        //memset(buffer, 0, sizeof(buffer));
        totalReceived += received;
    }
}

void Client::close()
{
    closesocket(server);
    WSACleanup();
}