#include "runcmd.h"

void RunCmd::Run(string cmd) {
#ifdef PLATFORM_WIN
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);
	cmd = "cmd /C \"" + cmd + "\"";
	if (!CreateProcess("C:\\Windows\\System32\\cmd.exe", &cmd[0], NULL, NULL, FALSE, 0, 0, 0, &si, &pi)) {
		return;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
#endif
}