#include "web/anscript.h"
#include "utils/dylib.h"

class CScript : public AnScript {
public:
	CScript();

	std::string libpath;
	DyLib lib;

	uintptr_t progress;
	std::vector<CVar> _inputs, _outputs;

	typedef void (*clearFunc)();
	clearFunc stdioClr;
	std::mutex* stdioLock;
	void*** stdioPtr;
	int* stdioCnt;
	int stdioI;

	void Clear() override;
	pAnScript_I CreateInstance() override;

	static std::unordered_map<std::string, std::weak_ptr<CScript>> allScrs;
};

class CScript_I : public AnScript_I {
public:
	void SetInput(int i, short val) override;
	void SetInput(int i, int val) override;
	void SetInput(int i, double val) override;
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override;
	void GetOutput(int i, int* val) override;

	float GetProgress() override;
};

typedef std::shared_ptr<CScript_I> pCScript_I;