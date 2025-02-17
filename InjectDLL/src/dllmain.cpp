#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dllmain_Variables.h"
#include "dllmain_Functions.h"
#include "Connectivity.h"
#include "Memory.h"
#include "LoggerService.h"

using namespace Main;

////////////////// Players definitions //////////////////

Memory::Link_class* Main::Link = new Memory::Link_class();
Memory::World_class* Main::World = new Memory::World_class();

Memory::OtherPlayer_class* Main::Jugador1 = new Memory::OtherPlayer_class(1);
Memory::OtherPlayer_class* Main::Jugador2 = new Memory::OtherPlayer_class(2);
Memory::OtherPlayer_class* Main::Jugador3 = new Memory::OtherPlayer_class(3);
Memory::OtherPlayer_class* Main::Jugador4 = new Memory::OtherPlayer_class(4);

Memory::OtherPlayer_class* Main::Jugadores[] = { Jugador1, Jugador2, Jugador3, Jugador4 };

Memory::BombSyncer* Main::BombSync = new Memory::BombSyncer();

////////////////// Parameter initialization //////////////////

uint64_t Main::baseAddr = Memory::getBaseAddress();

std::vector<std::vector<float>> Main::Jugador1Queue = { {}, {}, {} };
std::vector<std::vector<float>> Main::Jugador2Queue = { {}, {}, {} };
std::vector<std::vector<float>> Main::Jugador3Queue = { {}, {}, {} };
std::vector<std::vector<float>> Main::Jugador4Queue = { {}, {}, {} };
std::vector<std::vector<float>> Main::JugadoresQueues[] = {Jugador1Queue, Jugador2Queue, Jugador3Queue, Jugador4Queue};

Connectivity::namedPipeClass* Main::namedPipe = new Connectivity::namedPipeClass();
Connectivity::Client* Main::client = new Connectivity::Client();

std::shared_mutex WeaponChangeMutex;
std::shared_mutex BombExplodeMutex;

float Main::targetFPS = 1000;
float Main::serializationRate = 20;
float Main::ping = 0;

int Main::playerNumber = 0;
bool Main::IsProp = false;
bool Main::IsPropHuntStopped = true;
bool Main::HidePlayer32 = true;
std::vector<byte> Main::FoundPlayers = {};
std::string Main::serverName = "";
std::vector<bool> Main::ConnectedPlayers = { false, false, false, false };

auto Main::t0 = std::chrono::steady_clock::now();
auto Main::t1 = std::chrono::steady_clock::now();

std::string Main::serverData = "";

bool Main::isEnemySync = false;
bool Main::isGlyphSync = false;
bool Main::isQuestSync = false;
bool Main::isHvsSR = false;
bool Main::isDeathSwap = false;

int Main::GlyphUpdateTime = 60;
int Main::GlyphDistance = 250;

std::vector<std::string> Main::questServerSettings;

bool Main::isPaused = true;

std::vector<float> Main::oldLocations[] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

bool Main::QuestSyncReady = false;

void startServerLoop()
{
    std::cout << "Start of the console" << std::endl;
    std::thread(Main::mainServerLoop).detach();
}

void readInstruction()
{
    bool started = false;
    while (!started)
    {
        char chBuff[1024] = {0};
        int pipe_fd = open("/tmp/my_named_pipe", O_RDONLY);
        read(pipe_fd, chBuff, sizeof(chBuff));
        close(pipe_fd);

        std::string command(chBuff);
        std::string response;

        if (command.find("!connect") != std::string::npos)
        {
            if (Main::connectToServer(chBuff))
            {
                response = "Succeeded";
                Logging::LoggerService::LogInformation("Connected to server successfully");
            }
            else
            {
                response = "Failed";
            }
        }
        else if (command.find("!startServerLoop") != std::string::npos)
        {
            Logging::LoggerService::LogInformation("Start server loop requested...");
            response = "Succeeded";
            started = true;
        }

        int pipe_out = open("/tmp/my_named_pipe", O_WRONLY);
        write(pipe_out, response.c_str(), response.size());
        close(pipe_out);
    }

    startServerLoop();
}

__attribute__((constructor))
void init()
{
    Main::SetupAssemblyPatches();
    Logging::LoggerService::StartLoggerService();
    mkfifo("/tmp/my_named_pipe", 0666);
    std::thread(readInstruction).detach();
}

__attribute__((destructor))
void cleanup()
{
    unlink("/tmp/my_named_pipe");
}
