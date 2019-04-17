#include "Engine.h"

class MakeBg {
public:
	static void Init();

	void Do(const std::string& file);

	static void Draw();
private:
	static bool initd, draw;	
};