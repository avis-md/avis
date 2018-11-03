#include "Engine.h"

class ChangeLog {
public:
	static void Init();

	static void Draw();

	static bool show;
private:
	static std::vector<std::string> logs;
};