#include "dialog.h"

#ifdef PLATFORM_WIN
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#endif

std::vector<string> Dialog::OpenFile() {
	auto res = std::vector<string>();
#if defined(PLATFORM_WIN)
	OPENFILENAME fn = {};
	char buf[1024]{};

	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = glfwGetWin32Window(Display::window);
	fn.lpstrFile = buf;
	fn.nMaxFile = sizeof(buf);
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrInitialDir = NULL;
	fn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	if (GetOpenFileName(&fn)) {
		char* p = buf;
		while (!!*p) {
			res.push_back(string(p));
			p += res.back().size() + 1;
		}
		if (res.size() > 1) {
			auto s = res[0];
			res.erase(res.begin());
			for (uint i = 0; i < res.size(); i++) {
				res[i] = s + "\\" + res[i];
			}
		}
	}
#elif defined(PLATFORM_LNX) || defined(PLATFORM_OSX)
	FILE* f = popen("zenity --file-selection --multiple --separator=|", "r");
	char buf[1024];
	fgets(buf, 1024, f);
	res = string_split(string(buf, 1024), '|', true);
#endif
	return res;
}