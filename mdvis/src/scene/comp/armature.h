#pragma once
#include "SceneObjects.h"

#define ARMATURE_MAX_BONES 256

class ArmatureBone {
public:

	Transform* const tr;
	Vec3 const restPosition;
	Quat const restRotation;
	Vec3 const restScale;
	Vec3 tailPos() { return tr->position() + tr->forward()*length*tr->localScale().z; }
	float const length;
	bool const connected;
	const Mat4x4 restMatrix, restMatrixInv;
	const Mat4x4 restMatrixAInv;
	Mat4x4 newMatrix, animMatrix;
	string const name, fullName; // parent2/parent/me/
	uint const id;
	const std::vector<ArmatureBone*>& children() { return _children; }
	const ArmatureBone* parent;

	friend class Armature;
protected:
	ArmatureBone(uint id, Vec3 pos, Quat rot, Vec3 scl, float lgh, bool conn, Transform* tr, ArmatureBone* par);

	static const Vec3 boneVecs[6];
	static const uint boneIndices[24];
	static const Vec3 boneCol, boneSelCol;
	std::vector<ArmatureBone*> _children;
};
class Armature : public Component {
public:
	//Armature() : _anim(-1), Component("Armature", COMP_ARM, DRAWORDER_OVERLAY) {}
	~Armature();

	bool overridePos;
	Vec3 restPosition;
	Quat restRotation;
	Vec3 restScale;
	float animationScale = 1;
	const std::vector<ArmatureBone*>& bones() { return _bones; }

	virtual void OnPreRender() override;

	friend class SkinnedMeshRenderer;
	_allowshared(Armature);
protected:
	Armature(string s, SceneObject* o);
	Armature(std::ifstream& stream, SceneObject* o, long pos = -1);

	Animator* _anim;

	bool xray;
	std::vector<ArmatureBone*> _bones;
	std::vector<string> _allbonenames;
	std::vector<ArmatureBone*> _allbones;
	std::vector<int> _boneAnimIds;
	Mat4x4 _animMatrices[ARMATURE_MAX_BONES];
	ArmatureBone* MapBone(string nm);

	static void AddBone(std::ifstream&, std::vector<ArmatureBone*>&, std::vector<ArmatureBone*>&, SceneObject*, uint&);
	void GenMap(ArmatureBone* b = nullptr);
	void UpdateAnimIds();
	void Animate();
	void UpdateMats(ArmatureBone* b = nullptr);
};
