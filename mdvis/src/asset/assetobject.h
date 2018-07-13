#pragma once
#include "AssetObjects.h"

class AssetObject : public Object {
public:
	AssetObject(ASSETTYPE t) : type(t), Object(), _changed(false) {}
	virtual  ~AssetObject() {}

	const ASSETTYPE type = ASSETTYPE_UNDEF;
	bool _changed;
};