#pragma once

#include <string>
#include "dllmain_Variables.h"

namespace Main
{

	bool connectToServer(std::string serverMessage);
	void disconnectFromServer(std::string reason);
	void playerQueueUpdate();
	void HelperThread();
	void glyphScan();
	void characterSpawner();
	void QuestSync();
	std::string getSendString();
	void mainServerLoop();
	void oldServerLoop(); //TODO: remove
	void CalculatePVPDamage();
	void AddBigMessage(std::string Message);
	void SendTimerMessage(bool start, std::string countMode, int startTime, int maxTime);
	void SendMessageToOverlay(std::string Message);
	void PauseChecker();
	bool CheckIfPaused();
	float GetDistance(int playerNumber, bool includeZAxis);
	void QueueBomb(std::string bombType, float Position[3]);
	void Setup();
	void SetupAssemblyPatches();
	bool ExternIsPaused();
	
	// Interpolation functions

	void PlayerUpdater();

}