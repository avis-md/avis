#pragma once
#include "../annode.h"

class Node_Volume : public AnNode {
public:
	static const string sig;
	Node_Volume();

	static void Init();
	Vec2 DrawConn() override;
	void Draw() override;
	float DrawSide() override;
	void DrawScene() override;
	void Execute() override;
	
protected:
	static const Vec3 cubeVerts[8];
	static const uint cubeIndices[36];
	float ox, oy, oz, sx, sy, sz;
	int nx, ny, nz;
	Vec3 cutC, cutD;
	GLuint tex;
	static GLuint shad;
	static GLint shadLocs[10];
	static GLuint vao, vbo, veo;
};