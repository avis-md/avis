#include "lj256.h"
#include "Engine.h"

float* pss;
float* vls;
const float dt = 0.01f;

bool LJ256::Init(SyncInfo* info) {
	info->num = 256;
	info->resname = new char[info->num * info->namesz]{};
	info->name = new char[info->num * info->namesz]{};
	info->type = new uint16_t[info->num]{};
	info->resId = new uint16_t[info->num]{};
	info->bounds[1] = info->bounds[3] = info->bounds[5] = 4;
	info->pos = new float[info->num * 3];
	info->vel = new float[info->num * 3]{};
	for (uint a = 0; a < 4; a++) {
		for (uint b = 0; b < 4; b++) {
			for (uint c = 0; c < 4; c++) {
				uint off = a * 64 + b * 16 + c * 4;
				float* p = info->pos + off * 3;
				p[0] = p[9] = a + 0.25f;
				p[1] = p[7] = b + 0.25f;
				p[2] = p[5] = c + 0.25f;
				p[3] = p[6] = p[0] + 0.5f;
				p[4] = p[10] = p[1] + 0.5f;
				p[8] = p[11] = p[2] + 0.5f;
				uint16_t* t = info->type + off;
				t[0] = t[1] = t[2] = t[3] = (uint16_t)'C';
				char* n = info->resname + off * info->namesz;
				n[0] = n[info->namesz] = n[info->namesz * 2] = n[info->namesz * 3] = 'C';
				n = info->name + off * info->namesz;
				n[0] = n[info->namesz] = n[info->namesz * 2] = n[info->namesz * 3] = 'C';
			}
		}
	}
	pss = new float[info->num * 3];
	vls = new float[info->num * 3];
	float tx = 0, ty = 0, tz = 0;
	float T = 0.6f;
	for (uint a = 0; a < info->num; a++) {
		vls[a * 3] = Random::Range(-T, T);
		vls[a * 3 + 1] = Random::Range(-T, T);
		vls[a * 3 + 2] = Random::Range(-T, T);
		tx += vls[a * 3];
		ty += vls[a * 3 + 1];
		tz += vls[a * 3 + 2];
	}
	tx /= info->num;
	ty /= info->num;
	tz /= info->num;
	for (uint a = 0; a < info->num; a++) {
		vls[a * 3] -= tx;
		vls[a * 3 + 1] -= ty;
		vls[a * 3 + 2] -= tz;
	}
	memcpy(pss, info->pos, info->num * 3 * sizeof(float));
	return true;
}

void DP(float* p1, float* p2, float* dp) {
	dp[0] = p2[0] - p1[0];
	dp[1] = p2[1] - p1[1];
	dp[2] = p2[2] - p1[2];
	dp[0] /= 4; dp[1] /= 4; dp[2] /= 4;
	dp[0] -= roundf(dp[0]);
	dp[1] -= roundf(dp[1]);
	dp[2] -= roundf(dp[2]);
	dp[0] *= 4; dp[1] *= 4; dp[2] *= 4;
}

float DS2(float* dp) {
	return dp[0] * dp[0] + dp[1] * dp[1] + dp[2] * dp[2];
}

int sign(float f) {
	return (f >= 0) ? 1 : -1;
}

float W(float p) {
	if (p > 4) return p - 4;
	else if (p < 0) return p + 4;
	else return p;
}

void WL(float* p) {
	p[0] = W(p[0]);
	p[1] = W(p[1]);
	p[2] = W(p[2]);
}

bool LJ256::Loop(SyncInfo* info) {
	float ff[256 * 3]{};
	for (uint a = 0; a < info->num; a++) {
		float* p = pss + a * 3;
		float* f = ff + a * 3;
		for (uint b = 0; b < info->num; b++) {
			if (b == a) continue;
			float* p2 = pss + b * 3;
			float dp[3];
			DP(p, p2, dp);
			for (uint a = 0; a < 3; a++) dp[a] *= 10;
			float ds2 = DS2(dp);
			float fm = 48 * powf(1 / ds2, 4)*(powf(1 / ds2, 3) - 0.5f);
			f[0] += dp[0] * fm;
			f[1] += dp[1] * fm;
			f[2] += dp[2] * fm;
		}
	}
	float m = 1;
	for (uint a = 0; a < info->num; a++) {
		float* p = pss + a * 3;
		float* v = vls + a * 3;
		float* f = ff + a * 3;
		v[0] += f[0] * dt * m;
		v[1] += f[1] * dt * m;
		v[2] += f[2] * dt * m;
		p[0] += v[0] * dt + 0.5f * f[0] * dt * dt * m;
		p[1] += v[1] * dt + 0.5f * f[1] * dt * dt * m;
		p[2] += v[2] * dt + 0.5f * f[2] * dt * dt * m;
		WL(p);
	}
	memcpy(info->pos, pss, info->num * 3 * sizeof(float));
	info->fill = true;
	return true;
}