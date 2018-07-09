#pragma once
#include "SceneObjects.h"

class MeshFilter : public Component {
public:
	MeshFilter();

	rMesh mesh = 0;

	friend class MeshRenderer;
	_allowshared(MeshFilter);
protected:

	bool showBoundingBox = false;
	ASSETID _mesh = -1;
};
