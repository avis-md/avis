#include "Engine.h"

CameraEffect::CameraEffect(Material* mat) : AssetObject(ASSETTYPE_CAMEFFECT) {
	material = mat;
}

CameraEffect::CameraEffect(string p) : AssetObject(ASSETTYPE_CAMEFFECT) {

}