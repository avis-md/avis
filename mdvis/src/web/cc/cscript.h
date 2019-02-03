#include "web/anscript.h"
#include "utils/dylib.h"

class CScript : public AnScript {
public:
	CScript() : AnScript(AnScript::TYPE::C) {}

	std::string libpath;
	DyLib lib;

	std::vector<CVar> _inputs, _outputs;

	pAnScript_I CreateInstance() override;

	static std::unordered_map<std::string, std::weak_ptr<CScript>> allScrs;
};

class CScript_I : public AnScript_I {
public:
	void SetInput(int i, int val) override;
	void GetOutput(int i, int* val) override;
};

typedef std::shared_ptr<CScript_I> pCScript_I;