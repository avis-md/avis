#pragma once
#include "web/anweb.h"

class PyReader {
public:
	static bool initd;

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