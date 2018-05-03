#pragma once
#include "Engine.h"
#include "Particles.h"

//GROMACS molecular data file parser
class Gromacs {
public:

	static void Read(const string& file);
	static void LoadFiles();
};