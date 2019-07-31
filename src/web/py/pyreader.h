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

#pragma once
#include "web/anweb.h"

class PyReader {
public:
	static bool initd;

	static PyObject* mainModule;

	static void Init(), Deinit();
	
	static bool Read(PyScript* scr);
	static int ReadClassed(PyScript* scr, const std::string spath);
	static int ReadStatic(PyScript* scr, const std::string spath);

	static void Refresh(PyScript* scr);

protected:
	static void ParseDesc(std::istream& strm, std::string& ln, PyScript* scr);
	static bool ParseVar(std::istream& strm, std::string& ln, PyScript* scr, bool in, bool self);
	static bool ParseType(AnScript::Var& var);
};