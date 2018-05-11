#pragma once
#include "Engine.h"
#include "Particles.h"

//GROMACS molecular data file parser
class Gromacs {
public:
	static std::unordered_map<uint, float> _bondLengths;
	
	static void Read(const string& file);
	static void LoadFiles();
};