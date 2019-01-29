#include "anscript.h"
#include "utils/dylib.h"

class CScript : public AnScript {
public:
	std::string libpath;
	DyLib lib;

	std::shared_ptr<AnScript_I> CreateInstance() override;
};

class CScript_I : public AnScript_I {
public:
	void SetInput(int i, int val) override;
	void GetOutput(int i, int* val) override;
}