#include "runcmd.h"

int RunCmd::Run(std::string cmd) {
	Debug::Message("Cmd", cmd);
	int ret;
#ifdef PLATFORM_WIN
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);
	cmd = "cmd /C \"" + cmd + "\"";
	if (!CreateProcess("C:\\Windows\\System32\\cmd.exe", &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
		Debug::Warning("Cmd", "Could not create Windows process!");
		return -1;
	}
	DWORD w;
	do {
		w = WaitForSingleObject(pi.hProcess, 100);
	} while (w == WAIT_TIMEOUT);

	DWORD dret;
	GetExitCodeProcess(pi.hProcess, &dret);
	ret = (int)dret;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
#else
	ret = (system(&cmd[0]));
#endif
	Debug::Message("Cmd", "cmd exit with code " + std::to_string(ret));
	return ret;
}