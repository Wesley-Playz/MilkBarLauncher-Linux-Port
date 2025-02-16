#include "dllmain_Functions.h"

void Main::PlayerUpdater()
{

	float cycleTime = 1 / targetFPS * 1000; // In milliseconds   || FPS = updates per second -> seconds per update
	int sleepTime;

	DWORD t0 = GetTickCount();
	float DefaultPosition[3] = { -1124.3922, 270, 1914.9768 };
	float Position[4][3] = { { -1124.3922, 270, 1914.9768 }, { -1124.3922, 270, 1914.9768 }, { -1124.3922, 270, 1914.9768 }, { -1124.3922, 270, 1914.9768 } };
	float LastGlyphPosition[3] = { -10000, -10000, -10000 };
	DWORD LastGlyphUpdate = GetTickCount();

	while (true)
	{

		for (int i = 0; i < 4; i++)
		{

			for (int m = 0; m < 3; m++)
			{

				if (Main::JugadoresQueues[i][m].size() > 0)
				{

					Position[i][m] = JugadoresQueues[i][m].back();

					if (Main::JugadoresQueues[i][m].size() > 1)
					{
						Main::JugadoresQueues[i][m].pop_back();
					}

				}

			}

			Jugadores[i]->setPosition(Position[i]);

			if (isGlyphSync)
			{

				if (Position[i][0] != DefaultPosition[0])
				{

					if (i == 0)
					{

						if (isHvsSR)
						{

							float TimeSinceLastUpdate = float(GetTickCount() - LastGlyphUpdate) / 1000;

							if (TimeSinceLastUpdate > GlyphUpdateTime || std::sqrt(pow(std::abs(Position[i][0] - LastGlyphPosition[0]), 2) + pow(std::abs(Position[i][2] - LastGlyphPosition[2]), 2)) > GlyphDistance)
							{

								LastGlyphUpdate = GetTickCount();

								Jugadores[i]->setGlyph(Position[i]);

								for (int m = 0; m < 3; m++)
								{
									LastGlyphPosition[m] = Position[i][m];
								}
							}
							else
							{
								Jugadores[i]->setGlyph(LastGlyphPosition);
							}
						}
						else
						{
							Jugadores[i]->setGlyph(Position[i]);

							for (int m = 0; m < 3; m++)
							{
								LastGlyphPosition[m] = Position[i][m];
							}
						}
					}
					else
					{

						Jugadores[i]->setGlyph(Position[i]);

					}

				}

			}

		}

		if (cycleTime > float(GetTickCount() - t0))
		{
			Sleep(cycleTime - float(GetTickCount() - t0));
		}

		t0 = GetTickCount();

	}

}