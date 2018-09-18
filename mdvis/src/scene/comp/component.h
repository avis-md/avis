#pragma once
#include "SceneObjects.h"

typedef byte DRAWORDER;

class Component : public Object {
public:
	Component(string name, COMPONENT_TYPE t, DRAWORDER drawOrder = 0x00, SceneObject* o = nullptr, std::vector<COMPONENT_TYPE> dep = {});
	virtual  ~Component() {}

	const COMPONENT_TYPE componentType = COMP_UNDEF;
	const DRAWORDER drawOrder;
	bool active;
	rSceneObject object;

	virtual void OnPreUpdate() {}
	virtual void OnPreLUpdate() {}
	virtual void OnPreRender() {}

	friend int main(int argc, char **argv);
	friend class Scene;
	friend class SceneObject;
protected:
	std::vector<COMPONENT_TYPE> dependancies;
	std::vector<rComponent> dependacyPointers;

	//bool serializable;
	//std::vector<pair<void*, void*>> serializedValues;

	bool _expanded;

	static COMPONENT_TYPE Name2Type(string nm);

	virtual void LoadDefaultValues() {} //also loads assets

	virtual void Refresh() {}
};
