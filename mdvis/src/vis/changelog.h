#include "Engine.h"

class ChangeLog {
public:
	static void Init();

	static void Draw();

private:
	static bool show;
	static std::vector<std::string> logs;
};