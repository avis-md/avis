#pragma once
#include "AssetObjects.h"

class Mesh : public AssetObject {
public:
	//Mesh(); //until i figure out normal recalc algorithm
	bool loaded;
	Mesh(const std::vector<Vec3>& verts, const std::vector<Vec3>& norms, const std::vector<int>& tris, std::vector<Vec2> uvs = std::vector<Vec2>());

	std::vector<Vec3> vertices;
	std::vector<Vec3> normals, tangents;// , bitangents;
	std::vector<int> triangles;
	std::vector<Vec2> uv0, uv1;
	std::vector<std::vector<std::pair<byte, float>>> vertexGroupWeights;
	std::vector<string> vertexGroups;
	std::vector<std::pair<string, std::vector<Vec3>>> shapekeys;
	BBox boundingBox;

	uint vertexCount, triangleCount, materialCount;
	byte shapekeyCount;

	void RecalculateBoundingBox();

	friend class Engine;
	friend class MeshFilter;
	friend class MeshRenderer;
	_allowshared(Mesh);
protected:
	Mesh(std::istream& strm, uint offset = 0);
	Mesh(string path);
	Mesh(byte* mem);

	void CalcTangents();
	void InitVao();

	GLuint vao, vbos[4];

	std::vector<std::vector<int>> _matTriangles;
	std::vector<GLuint> _matIndicesBuffers;
};
