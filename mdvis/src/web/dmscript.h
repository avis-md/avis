#include "anscript.h"

class DmScript : public AnScript {
public:
	std::shared_ptr<AnScript_I> CreateInstance() override;
};

class DmScript_I : public AnScript_I {
public:
	void SetInput(int i, int val) override {}
	void GetOutput(int i, int* val) override {}
};