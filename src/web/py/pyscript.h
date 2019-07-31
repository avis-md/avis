// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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