#include "anscript.h"

class DmScript : public AnScript {
public:
	DmScript(std::string nm) : AnScript(AnScript::TYPE::NONE) {
		ok = true;
		name = nm;
	}
	pAnScript_I CreateInstance() override;
};

class DmScript_I : public AnScript_I {
public:
	void SetInput(int i, int val) override {}
	void GetOutput(int i, int* val) override {}
};