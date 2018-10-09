#include "Engine.h"

void Light::CalcShadowMatrix() {
	auto IP = glm::inverse(MVP::projection());
	auto LP = glm::inverse(QuatFunc::ToMatrix(object->transform.rotation()));
	MVP::Switch(true);
	MVP::Clear();
	if (lightType == LIGHTTYPE_SPOT || lightType == LIGHTTYPE_POINT) {
		Quat q = glm::inverse(object->transform.rotation());
		MVP::Mul(glm::perspectiveFov(angle * deg2rad, 1024.f, 1024.f, minDist, maxDist));
		MVP::Scale(1, 1, -1);
		MVP::Mul(QuatFunc::ToMatrix(q));
		Vec3 pos = -object->transform.position();
		MVP::Translate(pos.x, pos.y, pos.z);
	}
	else {
		float md = Clamp(maxDist / Camera::active->farClip, 0.f, 1.f) * 2 - 1;
	 	Vec4 edges[] = {
			Vec4(-1, -1, -1, 1), Vec4(-1, 1, -1, 1), Vec4(1, -1, -1, 1), Vec4(1, 1, -1, 1), 
			Vec4(-1, -1, md, 1), Vec4(-1, 1, md, 1), Vec4(1, -1, md, 1), Vec4(1, 1, md, 1) };
		
		Vec3 min, max;
		for (auto i = 0; i < 8; i++) {
			edges[i] = LP * IP * edges[i];
			edges[i] /= edges[i].w;
			if (!i) min = max = edges[i];
			else {
				for (byte b = 0; b < 3; b++) {
					if (min[b] > edges[i][b]) min[b] = edges[i][b];
					if (max[b] < edges[i][b]) max[b] = edges[i][b];
				}
			}
		}
		MVP::Mul(glm::ortho(min.x, max.x, min.y, max.y, min.z - maxDist, max.z));
		MVP::Mul(LP);
	}
}

Light::Light() : Component("Light", COMP_LHT), lightType(LIGHTTYPE_POINT), color(white()), cookie(0), hsvMap(0) {}

Light::Light(std::ifstream& stream, SceneObject* o, long pos) : Light() {
	if (pos >= 0)
		stream.seekg(pos);

	_Strm2Val(stream, lightType);
	drawShadow = (lightType & 0xf0) != 0;
	lightType = LIGHTTYPE(lightType & 0x0f);
	_Strm2Val(stream, intensity);
	_Strm2Val(stream, minDist);
	_Strm2Val(stream, maxDist);
	_Strm2Val(stream, angle);
	_Strm2Val(stream, shadowBias);
	_Strm2Val(stream, shadowStrength);
	_Strm2Val(stream, color.r);
	_Strm2Val(stream, color.g);
	_Strm2Val(stream, color.b);
	_Strm2Val(stream, color.a);
}