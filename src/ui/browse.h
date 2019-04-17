#include "Engine.h"
#include "browsetarget.h"

typedef void (*fileCallback) (std::string);

class Browse {
public:
	static enum class MODE {
		NONE,
		OPENFILE,
		OPENFD,
		SAVEFILE
	} mode;

	static BrowseTarget* system;

	static int selId;

	static fileCallback callback;

	static void Init();
	static void Draw();
};