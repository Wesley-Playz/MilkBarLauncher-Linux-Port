#pragma once

#define _WINSOCKAPI_
#define BUFF_SIZE 2048
#define RAPIDJSON_HAS_STDSTRING 1

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <tchar.h>
#include <map>
#include <any>
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "Memory.h"

namespace Connectivity
{

    ////////////////// Client.cpp //////////////////

    class Client {

    private:
        WSADATA WSAData;
        SOCKET server;
        SOCKADDR_IN addr;
        char buffer[7168];

    public:
        void connectToServer(std::string IP, std::string PORT);
        void sendMessage(std::string command, std::string message);
        void sendBytes(byte Message[7168]);
        std::string receive();
        void receiveBytes(byte* Output);
        void close();

    };

    ////////////////// NamedPipes.cpp //////////////////

    class namedPipeClass
    {
    public:
        HANDLE hPipe;
        void createServer();
    };

    ////////////////// Interpretation.cpp //////////////////

    std::map<int, std::map<std::string, std::any>> convertData(std::string data, bool printData = false);

    ////////////////// Serializer.cpp //////////////////

    rapidjson::Value addValueToJsonDocument(rapidjson::Document::AllocatorType& allocator, std::string data, std::string dataType);
    rapidjson::Value addMapToJsonDocument(rapidjson::Document::AllocatorType& allocator, std::map<std::string, std::string> data, std::string dataTypes);
    rapidjson::Value addVectorToJsonDocument(rapidjson::Document::AllocatorType& allocator, std::vector<std::string> data, std::string dataTypes);

    ////////////////// Deserializer.cpp //////////////////

    rapidjson::Document deserializeServerData(std::string message);

}