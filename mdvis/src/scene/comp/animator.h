#pragma once
#include "SceneObjects.h"

class Animator : public Component {
public:
	Animator(Animation* anim = nullptr);
	~Animator();

	Animation* animation;

	int IdOf(const string& s) {
		return animation ? animation->IdOf(s) : -1;
	}

	Vec4 Get(const string& name) {
		return animation ? animation->Get(name) : Vec4();
	}
	Vec4 Get(uint id) {
		return animation ? animation->Get(id) : Vec4();
	}

	float fps = 24;

	void OnPreLUpdate() override;

	friend class Engine;
	_allowshared(Animator);
protected:
	ASSETID _animation = -1;

	static void _SetAnim(void* v);
};
