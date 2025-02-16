#include "Connectivity.h"

using namespace Connectivity;

void namedPipeClass::createServer()
{
	bool pipeOpened = false;
	int tries = 0;
	HANDLE hPipeTemp;
	Logging::LoggerService::LogDebug("Connecting to named pipe");

	while (!pipeOpened)
	{
		hPipeTemp = CreateFile("\\\\.\\pipe\\languageConnectionPipe", GENERIC_ALL, 0, nullptr, OPEN_EXISTING, 0, nullptr);

		if (hPipeTemp == INVALID_HANDLE_VALUE)
		{
			tries++;
			continue;
		}

		pipeOpened = true;

		DWORD mode = PIPE_READMODE_MESSAGE;

		SetNamedPipeHandleState(hPipeTemp, &mode, nullptr, nullptr);

		this->hPipe = hPipeTemp;
	}
}