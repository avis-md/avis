#pragma once
#include "Engine.h"

//GROMACS molecular data file parser
class Gromacs {
public:
	class Particle {
	public:
		uint residueNumber, atomNumber;
		string residueName, atomName;
		ushort atomId;
		Vec3 position, velocity;
	};
	class Frame {
	public:
		Frame() : particles(), conns(), connSzs() {}

		float time;
		string name;
		uint count;
		std::vector<Particle> particles;
		std::vector<glm::i32vec4> conns;
		std::vector<byte> connSzs;
	};

	Gromacs(const string& file);
	static void LoadFiles();
	void ReloadCols();
	
	std::vector<Frame> frames;
	Vec3 boundingBox;

	static std::unordered_map<ushort, Vec3> _type2color;
	static std::unordered_map<uint, float> _bondLengths;

	GLuint _vao, _vboV, _vboC;
};