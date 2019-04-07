#define EPSILON 0.0001f
#define PI 3.14159f
#define STRAN 4

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

static float4 MatVec(const Mat m, const float4 v) {
	return (float4)(
		dot(m.a, v),
		dot(m.b, v),
		dot(m.c, v),
		dot(m.d, v)
	);
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


__kernel void GenerateCameraRays(__global Ray* rays,
	__global const Camera* cam,
	int width,
	int height) {
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

static float3 SkyAt(float3 dir, __global float* bg) {
	const int bgw = 2048;
	const int bgh = bgw / 2;

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
	int width,
	int height,
	__global unsigned char* out, //actual color
	__global float4* ocol, //ray color
	__global Ray* ray,
	int rng,
	__global float* accum,
	int smps,
	__global float* bg,
	float bgMul) {
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

				float4 pos = ConvertFromBarycentric(positions + ind * 3, ids + ind, prim_id, &isect[k].uvwt);
				pos.w = 1;
				float4 _norm = ConvertFromBarycentric(normals + ind * 3, ids + ind, prim_id, &isect[k].uvwt);
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
					if (Randf(&rnd) < (1 - ((1 - fres) * 0.5f))) {
						//Beckmann(&norm, 1 - mat.gloss, &rnd);
						Beckmann(&norm, 0.005f, &rnd, rng);
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
				float3 bgc = SkyAt(normalize(ray[k].d.xyz), bg) * bgMul;

				if (col.w < 0.5f) {
					col.xyz = bgc;
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
		out[k * 4] = clamp(accum[k * 3] / smps, 0.0f, 1.0f) * 255;
		out[k * 4 + 1] = clamp(accum[k * 3 + 1] / smps, 0.0f, 1.0f) * 255;
		out[k * 4 + 2] = clamp(accum[k * 3 + 2] / smps, 0.0f, 1.0f) * 255;
		out[k * 4 + 3] = 255;
	}
}