#include "web/anscript.h"
#include "utils/dylib.h"

class CScript : public AnScript {
public:
	std::string libpath;
	DyLib lib;

	std::vector<CVar> _inputs, _outputs;

	std::shared_ptr<AnScript_I> CreateInstance() override;
};

class CScript_I : public AnScript_I {
public:
	void SetInput(int i, int val) override;
	void GetOutput(int i, int* val) override;
};