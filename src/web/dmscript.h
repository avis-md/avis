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