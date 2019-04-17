#include "anscript.h"

class DmScript : public AnScript {
public:
	DmScript(std::string nm) : AnScript(AnScript::TYPE::NONE) {
		ok = false;
		name = nm;
	}
	pAnScript_I CreateInstance() override;

	Var& AddInput(const std::string& name, AN_VARTYPE type, int dim = 0);
	Var& AddOutput(const std::string& name, AN_VARTYPE type, int dim = 0);
};

class DmScript_I : public AnScript_I {
public:
	std::vector<VarVal> outputVs;

	void* Resolve(uintptr_t offset) override;
	int* GetDimValue(const CVar::szItem& i) override;

	void SetInput(int i, short val) override {}
	void SetInput(int i, int val) override {}
	void SetInput(int i, double val) override {}
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override {}
	void GetOutput(int i, int* val) override {}
};