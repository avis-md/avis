#include "runcmd.h"

void RunCmd::Run(std::string cmd) {
#ifdef PLATFORM_WIN
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);
	cmd = "cmd /C \"" + cmd + "\"";
	if (!CreateProcess("C:\\Windows\\System32\\cmd.exe", &cmd[0], NULL, NULL, FALSE, 0, 0, 0, &si, &pi)) {
		return;
	}
	DWORD w;
	do {
		w = WaitForSingleObject(pi.hProcess, 100);
	} while (w == WAIT_TIMEOUT);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
#else
	system(&cmd[0]);
#endif
}