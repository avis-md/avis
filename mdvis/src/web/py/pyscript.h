#include "web/anscript.h"
#include <Python.h>

class PyScript : public AnScript {
public:
	PyScript();

	std::string scrpath;
	PyObject* lib;

	PyObject* spawner;
	std::string funcNm;

	std::vector<PyVar> _inputs, _outputs;

	void Clear() override;
	pAnScript_I CreateInstance() override;

	static std::unordered_map<std::string, std::weak_ptr<PyScript>> allScrs;
};

class PyScript_I : public AnScript_I {
public:
	~PyScript_I();

	PyObject* func;
	std::vector<PyObject*> outputs;

	void SetInput(int i, short val) override;
	void SetInput(int i, int val) override;
	void SetInput(int i, double val) override;
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override;
	void GetOutput(int i, int* val) override;

	void Execute() override;
	float GetProgress() override;
};

typedef std::shared_ptr<PyScript_I> pPyScript_I;