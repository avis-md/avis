#pragma once
#include "AssetObjects.h"

class CameraEffect : public AssetObject {
public:
	CameraEffect(Material* mat);

	bool enabled = true;

	friend class Camera;
	friend class Engine;
	_allowshared(CameraEffect);
protected:
	Material* material;
	int _material = -1;
	bool expanded; //editor only
	string mainTex;

	CameraEffect(string s);
};
