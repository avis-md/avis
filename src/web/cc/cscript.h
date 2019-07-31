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