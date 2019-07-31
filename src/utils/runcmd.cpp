// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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