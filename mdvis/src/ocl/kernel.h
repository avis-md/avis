#pragma once

namespace ocl {
	const char raykernel[] = R"(
#define EPSILON 0.0001f
#define PI 3.14159f
#define STRAN 4


static uint Rand(uint rnd) {
	rnd ^= rnd << 13;
	rnd ^= rnd >> 17;
	rnd ^= rnd << 5;
	return rnd;
}

static float Randf(uint* r) {
	*r = Rand(*r);
	return 0.0001f*(*r % 10000);
}

static float RandfStra(uint* r, int i, int n) {
	float rnd = Randf(r);
	i %= n;
	return (i + rnd) / n;
}

static float3 RandHemi(uint* r, int s) {
	float r1 = sqrt(RandfStra(r, s, STRAN));
	float r2 = RandfStra(r, s, STRAN + 1) * 2 * PI;
	return (float3)(r1 * cos(r2), r1 * sin(r2), sqrt(max(1 - r1, 0.0f)));
}


typedef struct _Ray {
	/// xyz - origin, w - max range
	float4 o;
	/// xyz - direction, w - time
	float4 d;
	/// x - ray mask, y - activity flag
	int2 extra;
	/// Padding
	float2 padding;
} Ray;

typedef struct _Mat {
	float4 a;
	float4 b;
	float4 c;
	float4 d;
} Mat;

typedef float4 Quat;

static Mat IMat() {
	Mat res;
	res.a = (float4)(1, 0, 0, 0);
	res.b = (float4)(0, 1, 0, 0);
	res.c = (float4)(0, 0, 1, 0);
	res.d = (float4)(0, 0, 0, 1);
	return res;
}

static Mat TMat(float3 v) {
	Mat res = IMat();
	res.d.xyz = v;
	return res;
}

static float4 MatVec(const Mat m, const float4 v) {
	float4 vec = (float4)(
		dot(m.a, v),
		dot(m.b, v),
		dot(m.c, v),
		dot(m.d, v)
	);
	return vec;
}


static Quat IQuat() {
	return (Quat)(0, 0, 0, 1);
}

static Quat AAQuat(float3 axis, float a) {
	float factor = sin(a / 2.0f);
	float x = axis.x * factor;
	float y = axis.y * factor;
	float z = axis.z * factor;
	float w = (float)cos(a / 2.0f);
	return normalize((Quat)(x, y, z, w));
}

static float3 QuatVec(Quat q, float3 v) {
	float3 uv = cross(q.xyz, v);
	float3 uuv = cross(q.xyz, uv);

	return v + ((uv * q.w) + uuv) * 2.0f;
}

typedef struct _Camera {
	Mat ip;
	// Near and far Z
	float2 zcap;
} Camera;

typedef struct _Intersection {
	// id of a shape
	int shapeid;
	// Primitive index
	int primid;
	// Padding elements
	int padding0;
	int padding1;

	// uv - hit barycentrics, w - ray distance
	float4 uvwt;
} Intersection;


__kernel void GenRays_Pin(
		__global Ray* rays,
		__global const Camera* cam,
		int width,
		int height
	) {
	int2 globalid;
	globalid.x = get_global_id(0);
	globalid.y = get_global_id(1);

	// Check borders
	if (globalid.x < width && globalid.y < height) {
		float x = -1.f + 2.f * (float)globalid.x / (float)width;
		float y = -1.f + 2.f * (float)globalid.y / (float)height;

		int k = globalid.y * width + globalid.x;

		float4 cn = MatVec(cam->ip, (float4)(x, y, -1, 1));
		cn.xyz /= cn.w;
		rays[k].o.xyz = cn.xyz;
		float4 cf = MatVec(cam->ip, (float4)(x, y, 1, 1));
		cf.xyz /= cf.w;
		rays[k].d.xyz = normalize(cf.xyz - cn.xyz);
		rays[k].o.w = cam->zcap.y;

		rays[k].extra.x = 0xFFFFFFFF;
		rays[k].extra.y = 0xFFFFFFFF;
	}
}

__kernel void GenRays_Dof(
		__global Ray* rays,
		__global Mat* IPs,
		float4 camPos,
		int width,
		int height,
		float plane,
		float tmax,
		int rng
	) {
	int2 globalid;
	globalid.x = get_global_id(0);
	globalid.y = get_global_id(1);

	// Check borders
	if (globalid.x < width && globalid.y < height) {
		float x = -1.f + 2.f * (float)globalid.x / (float)width;
		float y = -1.f + 2.f * (float)globalid.y / (float)height;

		int k = globalid.y * width + globalid.x;

		float4 cn = MatVec(IPs[0], MatVec(IPs[1], (float4)(x, y, -1, 1)));
		cn.xyz /= cn.w;
		rays[k].o.xyz = cn.xyz;
		float4 cf = MatVec(IPs[0], MatVec(IPs[1], (float4)(x, y, 1, 1)));
		cf.xyz /= cf.w;
		rays[k].d.xyz = normalize(cf.xyz - cn.xyz);
		rays[k].o.w = 1000;

		rays[k].extra.x = 0xFFFFFFFF;
		rays[k].extra.y = 0xFFFFFFFF;
	}
}

__kernel void GenRays_Dof2(
		__global Ray* rays,
		__global Mat* IPs,
		float4 camPos,
		int width,
		int height,
		float plane,
		float tmax,
		int rng
	) {
	int2 globalid;
	globalid.x = get_global_id(0);
	globalid.y = get_global_id(1);

	// Check borders
	if (globalid.x < width && globalid.y < height) {
		float x = -1.f + 2.f * (float)globalid.x / (float)width;
		float y = -1.f + 2.f * (float)globalid.y / (float)height;

		int k = globalid.y * width + globalid.x;

		float4 cf = MatVec(IPs[1], (float4)(x, y, 1, 1));
		cf.xyz /= cf.w;

		float3 rc = (float3)(0, 0, plane);

		float3 cv = -rc;
		float3 cd = normalize(cf.xyz);

		/*
		uint rnd = Rand((uint)(k + rng));
		
		float th = Randf(&rnd) * 2 * PI;
		float2 rot = (float2)(cos(th), sin(th)) * Randf(&rnd) * tmax;

		Quat rotx = AAQuat((float3)(1, 0, 0), rot.x);
		Quat roty = AAQuat((float3)(0, 1, 0), rot.y);

		cv = QuatVec(rotx, QuatVec(roty, cv));
		cd = QuatVec(rotx, QuatVec(roty, cd));
		*/
		float4 co;
		co.xyz = rc + cv;
		co.w = 1;

		float4 cd4;
		cd4.xyz = cd;
		cd4.w = 0;

		co = MatVec(IPs[0], co);
		cd4 = MatVec(IPs[0], cd4);

		rays[k].o.xyz = co.xyz / co.w;
		rays[k].d.xyz = normalize(cd4.xyz);
		rays[k].o.w = 1000;

		rays[k].extra.x = 0xFFFFFFFF;
		rays[k].extra.y = 0xFFFFFFFF;
	}
}


static float4 ConvertFromBarycentric(__global const float* vec,
	__global const int* ind,
	int prim_id,
	__global const float4* uvwt) {
	float4 a = (float4)(vec[ind[prim_id * 3] * 3],
		vec[ind[prim_id * 3] * 3 + 1],
		vec[ind[prim_id * 3] * 3 + 2], 0.f);

	float4 b = (float4)(vec[ind[prim_id * 3 + 1] * 3],
		vec[ind[prim_id * 3 + 1] * 3 + 1],
		vec[ind[prim_id * 3 + 1] * 3 + 2], 0.f);

	float4 c = (float4)(vec[ind[prim_id * 3 + 2] * 3],
		vec[ind[prim_id * 3 + 2] * 3 + 1],
		vec[ind[prim_id * 3 + 2] * 3 + 2], 0.f);
	return a * (1 - uvwt->x - uvwt->y) + b * uvwt->x + c * uvwt->y;
}


static void GetTans(float3 n, float3* t1, float3* t2) {
	*t1 = normalize(cross(n, (fabs(n.x) > 0.9999f) ? (float3)(0, 0, 1) : (float3)(1, 0, 0)));
	*t2 = normalize(cross(n, *t1));
}

static void Beckmann(float3* nrm, float rough, uint* rnd, int s) {
	if (rough == 0) return;
	float a = rough * rough;
	float t1 = atan(sqrt(-a * log(1 - RandfStra(rnd, s, STRAN))));
	float t2 = RandfStra(rnd, s, STRAN + 1) * 2 * PI;
	float3 n1, n2;
	GetTans(*nrm, &n1, &n2);
	*nrm = *nrm * cos(t1) + (n1 * cos(t2) + n2 * sin(t2)) * sin(t1);
}

static float3 SkyAt(float3 dir, __global float* bg, int bgw, int bgh) {
	float2 rf = -normalize((float2)(dir.x, dir.z));
	float cx = 0;
	if (rf.y <= -1)
		cx = 0.75f;
	else {
		cx = acos(clamp(rf.x, -0.9999f, 0.9999f)) / (PI * 2);
		cx = mix(1 - cx, cx, ceil(rf.y));
	}
	float sy = asin(clamp(dir.y, -0.9999f, 0.9999f)) / PI;
	int x = (int)(cx * bgw);
	//x = (x + 200) % bgw;
	int y = (int)((sy + 0.5f) * bgh);
	int off = clamp(x + y * bgw, 0, bgw * bgh);
	//return (float3)(1, 1, 1);
	return (float3)(bg[off * 3], bg[off * 3 + 1], bg[off * 3 + 2]);
}


__kernel void Shading(//scene
        __global float* positions,
        __global float* normals,
        __global int* ids,
        __global float4* colors,
        __global int* indents,
        __global Mat* matrices,
        __global Intersection* isect,
        int weight,
        int width, int height,
        __global float* out, //actual color
        __global float4* ocol, //ray color
        __global Ray* ray,
        int rng,
        __global float* accum,
        int smps,
        float specular, float roughness,
        __global float* bg,
		int bgw, int bgh,
        float bgMul, float bgMul2
	) {
    
	int2 globalid;
	globalid.x = get_global_id(0);
	globalid.y = get_global_id(1);

	// Check borders
	if (globalid.x < width && globalid.y < height) {
		int k = globalid.y * width + globalid.x;
		int shape_id = isect[k].shapeid;
		int prim_id = isect[k].primid;

		uint rnd = (uint)(k + rng);
		if (weight > 0) {
			ocol[k].w = 0;
			if (smps == 1) {
				accum[k * 3] = 0.f;
				accum[k * 3 + 1] = 0.f;
				accum[k * 3 + 2] = 0.f;
			}
		}
		float4 col = ocol[k];

		if (col.w >= 0) {
			if (shape_id != -1 && prim_id != -1) {
				// Calculate position and normal of the intersection point
				int ind = indents[shape_id];

				Mat mv = matrices[shape_id];

				float4 pos = ConvertFromBarycentric(positions + ind, ids + ind, prim_id, &isect[k].uvwt);
				pos.w = 1;
				float4 _norm = ConvertFromBarycentric(normals + ind, ids + ind, prim_id, &isect[k].uvwt);
				_norm.w = 0;
				pos = MatVec(mv, pos);
				pos /= pos.w;
				float3 norm = MatVec(mv, _norm).xyz;
				norm = normalize(norm);

				float3 diff_col = colors[shape_id].xyz;

				if (ray[k].o.w > 0.01f) {
					float3 ro = ray[k].o.xyz;
					float3 rd = normalize(ray[k].d.xyz);

					float frr = 0;//(1 - mat.ior) / (1 + mat.ior);
					frr *= frr;
					float fres = frr + (1 - frr)*pow(1 - dot(-rd, norm), 5);
					//if (Randf(&rnd) < (1 - ((1 - fres) * (1 - mat.specular)))) {
					if (Randf(&rnd) < (1 - ((1 - fres) * (1 - specular)))) {
						Beckmann(&norm, roughness, &rnd, rng);
						//Beckmann(&norm, 0.005f, &rnd, rng);
						norm = rd - 2 * dot(rd, norm.xyz) / (norm.x*norm.x + norm.y*norm.y + norm.z*norm.z) * norm;
					}
					else {
						float3 t1, t2;
						GetTans(norm, &t1, &t2);
						float3 rh = RandHemi(&rnd, rng);
						float3 nrm = t1 * rh.x + t2 * rh.y + norm * rh.z;
						//ocol[k].xyz *= dot(nrm, norm);
						norm = nrm;
						//diff_col = (float3)(1, 1, 1);
					}

					if (ocol[k].w < 0.5f)
						ocol[k].xyz = diff_col;
					else
						ocol[k].xyz *= diff_col;
					ocol[k].w = 1;


					ray[k].d.xyz = norm;
					ray[k].o.xyz = pos.xyz + norm * EPSILON;
				}
			}
			else {
				float3 bgc = SkyAt(normalize(ray[k].d.xyz), bg, bgw, bgh) * bgMul;

				if (col.w < 0.5f) {
					col.xyz = bgc * bgMul * bgMul2;
					//col.xyz = (float3)(1, 1, 1);
				}
				else
					col.xyz *= bgc;

				accum[k * 3] += max(col.x, 0.0f);
				accum[k * 3 + 1] += max(col.y, 0.0f);
				accum[k * 3 + 2] += max(col.z, 0.0f);
				ray[k].o = 0;
				ocol[k].w = -1;
			}
		}
		out[k * 4] = accum[k * 3] / smps;
		out[k * 4 + 1] = accum[k * 3 + 1] / smps;
		out[k * 4 + 2] = accum[k * 3 + 2] / smps;
		out[k * 4 + 3] = 1;
	}
}
)";
const int raykernel_sz = sizeof(raykernel);
}