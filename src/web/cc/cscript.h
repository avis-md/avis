#include "web/anscript.h"
#include "utils/dylib.h"

class CScript_I;
typedef std::shared_ptr<CScript_I> pCScript_I;

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

	void RegInstances(), UnregInstances();

	static std::unordered_map<std::string, std::weak_ptr<CScript>> allScrs;
	std::vector<CScript_I*> instances;
};

class CScript_I : public AnScript_I {
public:
	~CScript_I();

	void* Resolve(uintptr_t offset) override;
	int* GetDimValue(const CVar::szItem& i) override;

	void SetInput(int i, short val) override;
	void SetInput(int i, int val) override;
	void SetInput(int i, double val) override;
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override;
	void GetOutput(int i, int* val) override;

	float GetProgress() override;
};