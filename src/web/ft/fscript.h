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

class FScript_I;
typedef std::shared_ptr<FScript_I> pFScript_I;

class FScript : public AnScript {
public:
	typedef void(*emptyFunc)();
	typedef char*(*wrapFunc)();
	
	FScript();

	std::string libpath;
	DyLib lib;
	wrapFunc funcLoc;

	std::vector<CVar> _inputs, _outputs;
	std::vector<emptyFunc> _inarr_pre, _outarr_post;
	int pre, post;

	int32_t** arr_in_shapeloc;
	void** arr_in_dataloc;
	int32_t** arr_out_shapeloc;
	void** arr_out_dataloc;

	void Clear() override;
	pAnScript_I CreateInstance() override;

	static std::unordered_map<std::string, std::weak_ptr<FScript>> allScrs;
};

class FScript_I : public AnScript_I {
public:
	~FScript_I();

	void* Resolve(uintptr_t offset) override;
	int* GetDimValue(const CVar::szItem& i) override;

	void SetInput(int i, short val) override;
	void SetInput(int i, int val) override;
	void SetInput(int i, double val) override;
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override;
	void GetOutput(int i, int* val) override;

	void GetOutputArrs();

	struct OutVal {
		VarVal val;
		std::vector<int> dims;
	};

	std::vector<OutVal> outputArrs;

	void Execute() override;
	float GetProgress() override;
};