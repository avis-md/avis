#include "web/anscript.h"
#include <Python.h>

class PyScript_I;
typedef std::shared_ptr<PyScript_I> pPyScript_I;

class PyScript : public AnScript {
public:
	PyScript();

	std::string scrpath;
	PyObject* lib, *func;

	PyObject* spawner;
	std::string funcNm;

	std::vector<PyVar> _inputs, _outputs;

	void Clear() override;
	pAnScript_I CreateInstance() override;

	void RegInstances(), UnregInstances();

	static std::unordered_map<std::string, std::weak_ptr<PyScript>> allScrs;
	std::vector<PyScript_I*> instances;
};

class PyScript_I : public AnScript_I {
public:
	~PyScript_I();

	PyObject* dict;

	void* Resolve(uintptr_t offset) override;
	int* GetDimValue(const CVar::szItem& i) override;

	void SetInput(int i, short val) override;
	void SetInput(int i, int val) override;
	void SetInput(int i, double val) override;
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override;
	void Set(int i, PyObject* o);

	void GetOutput(int i, int* val) override;

	void Execute() override;
	float GetProgress() override;

	void GetOutputVs();

	struct OutVal {
		VarVal val;
		std::vector<int> dims;
	};

	std::vector<OutVal> outputVs;

	std::vector<Texture> figures;
	int figCount;
};