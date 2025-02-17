#pragma once

#define BUFF_SIZE 2048
#define RAPIDJSON_HAS_STDSTRING 1

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
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
        int server;
        struct sockaddr_in addr;
        char buffer[7168];

    public:
        void connectToServer(std::string IP, std::string PORT);
        void sendMessage(std::string command, std::string message);
        void sendBytes(uint8_t Message[7168]);
        std::string receive();
        void receiveBytes(uint8_t* Output);
        void closeConnection();
    };

    ////////////////// NamedPipes.cpp //////////////////

    class namedPipeClass
    {
    public:
        int pipe_fd;
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
