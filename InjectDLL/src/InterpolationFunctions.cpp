#include "dllmain_Functions.h"
#include <chrono>
#include <thread>
#include <cmath>

void Main::PlayerUpdater()
{
    using namespace std::chrono;

    float cycleTime = 1000.0f / targetFPS; // In milliseconds
    float DefaultPosition[3] = { -1124.3922f, 270.0f, 1914.9768f };
    float Position[4][3] = { 
        { -1124.3922f, 270.0f, 1914.9768f }, 
        { -1124.3922f, 270.0f, 1914.9768f }, 
        { -1124.3922f, 270.0f, 1914.9768f }, 
        { -1124.3922f, 270.0f, 1914.9768f } 
    };
    float LastGlyphPosition[3] = { -10000.0f, -10000.0f, -10000.0f };
    auto LastGlyphUpdate = steady_clock::now();

    while (true)
    {
        auto t0 = steady_clock::now();

        for (int i = 0; i < 4; i++)
        {
            for (int m = 0; m < 3; m++)
            {
                if (!Main::JugadoresQueues[i][m].empty())
                {
                    Position[i][m] = Main::JugadoresQueues[i][m].back();

                    if (Main::JugadoresQueues[i][m].size() > 1)
                    {
                        Main::JugadoresQueues[i][m].pop_back();
                    }
                }
            }

            Main::Jugadores[i]->setPosition(Position[i]);

            if (isGlyphSync)
            {
                if (Position[i][0] != DefaultPosition[0])
                {
                    if (i == 0)
                    {
                        if (isHvsSR)
                        {
                            float TimeSinceLastUpdate = duration<float>(steady_clock::now() - LastGlyphUpdate).count();

                            if (TimeSinceLastUpdate > GlyphUpdateTime || 
                                std::sqrt(std::pow(std::abs(Position[i][0] - LastGlyphPosition[0]), 2) + 
                                          std::pow(std::abs(Position[i][2] - LastGlyphPosition[2]), 2)) > GlyphDistance)
                            {
                                LastGlyphUpdate = steady_clock::now();
                                Main::Jugadores[i]->setGlyph(Position[i]);

                                for (int m = 0; m < 3; m++)
                                {
                                    LastGlyphPosition[m] = Position[i][m];
                                }
                            }
                            else
                            {
                                Main::Jugadores[i]->setGlyph(LastGlyphPosition);
                            }
                        }
                        else
                        {
                            Main::Jugadores[i]->setGlyph(Position[i]);

                            for (int m = 0; m < 3; m++)
                            {
                                LastGlyphPosition[m] = Position[i][m];
                            }
                        }
                    }
                    else
                    {
                        Main::Jugadores[i]->setGlyph(Position[i]);
                    }
                }
            }
        }

        auto t1 = steady_clock::now();
        float elapsedTime = duration<float, std::milli>(t1 - t0).count();

        if (cycleTime > elapsedTime)
        {
            std::this_thread::sleep_for(milliseconds(static_cast<int>(cycleTime - elapsedTime)));
        }
    }
}
