#pragma once

namespace ocl {
	const char* code = R"(
typedef struct {
	float rough;
	float specular;
	float gloss;
	float metallic;
    float ior;
} mat_st;

typedef struct {
	int w;
	int h;
	float IP[16];
	int rnd;
	float str;
    int bgw;
    int bgh;
	mat_st mat;
} info_st;

typedef struct {
	float3 pos;
	float3 dir;
} ray_st;

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

static float Len2(float3 vec) {
	return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}

static float Len(float3 vec) {
    return sqrt(Len2(vec));
}

static ray_st Refl(ray_st ray, float3 pos, float3 nrm) {
	ray_st res;
	res.pos = pos;
	res.dir = ray.dir - 2 * dot(ray.dir, nrm) * nrm;
	res.pos += res.dir * 0.001f;
	return res;
}

static float3 MatMul(float* mat, float3 vec) {
	float3 res;
	res.x = mat[0]*vec.x + mat[4]*vec.y + mat[8]*vec.z + mat[12];
	res.y = mat[1]*vec.x + mat[5]*vec.y + mat[9]*vec.z + mat[13];
	res.z = mat[2]*vec.x + mat[6]*vec.y + mat[10]*vec.z + mat[14];
	float w = mat[3]*vec.x + mat[7]*vec.y + mat[11]*vec.z + mat[15];
	return res / w;
}

static float2 LineNearest(float3 p1, float3 p2, float3 d1, float3 d2) {
	float dab = dot(d1, d2);
	if (dab > 0.9999f || dab < -0.9999f) return (float2)(-100, -100);
	
	float3 c = p1 - p2;
	float dac = dot(d1, c);
	float dbc = dot(d2, c);
	
	float num = dbc * dab - dac;
	float div = 1 - dab * dab;
	float resx = num / div;
	float resy = dbc + resx * dab;
	return (float2)(resx, resy);
}

//------------------------------

static ray_st Diffuse(float3 pos, float3 nrm, uint* rnd) {
	ray_st res;
	res.pos = pos;
	float r1 = Randf(rnd);
	float r2 = Randf(rnd);
	float r3 = Randf(rnd);
	res.dir = normalize((float3)(r1*2-1, r2*2-1, r3*2-1));
	if (dot(res.dir, nrm) < 0) res.dir *= -1.f;
	res.pos += res.dir * 0.0001f;
	return res;
}

/*
static float OrenNayar(float3 eye, float3 nrm, float3 out, float rough) {
	if (rough < 0.00001f) {
		return dot(out, nrm);
	}
	else {
		rough *= rough;
		float A = 1 - 0.5f*(rough / (rough + 0.33f));
		float B = 0.45f*(rough / (rough + 0.09f));
		float cs = dot(out, nrm);
		float t1 = acos(cs);
		float t2 = acos(dot(eye, nrm));
		float3 p1 = normalize(cross(eye, nrm));
		float3 p2 = normalize(cross(out, nrm));
		float cp = dot(p1, p2);
		float a = max(t1, t2);
		float b = min(t1, t2);
		return cs * (A + (B * max(0.f, cp) * sin(a) * tan(b)));
	}
}
*/
static void Beckmann(float3* nrm, float rough, uint* rnd) {
	float a = rough * rough;
	//float t1 = acos(sqrt(1/(1-a*log(1-Randf(rnd)))));
	float t1 = atan(sqrt(-a*log(1-Randf(rnd))));
	float t2 = Randf(rnd) * 2 * 3.14159f;
	float3 n1 = normalize(cross(*nrm, (nrm->x > 0.9999f)? (float3)(0, 0, 1) : (float3)(1, 0, 0)));
	float3 n2 = normalize(cross(*nrm, n1));
	*nrm = *nrm * cos(t1) + (n1 * cos(t2) + n2 * sin(t2)) * sin(t1);
}

static float Int_Ball(ray_st ray, float3 loc, float rad, float3* pos, float3* nrm) {
	float3 L = loc - ray.pos;
	float3 D = ray.dir;
	float lL2 = Len2(L);
	float lD2 = Len2(D);
	float lL = sqrt(lL2);
	L /= lL;
	D /= sqrt(lD2);
	float cs = dot(L, D);
	float isr = lL2*cs*cs - lL2 + rad*rad;
	if (isr < 0) return -1;
	float dst = lL*cs - sqrt(isr);
	if (dst < 0) return -1;
	*pos = ray.pos + ray.dir * dst;
	*nrm = normalize(*pos - loc);
	return dst;
}

static float Int_Tube(ray_st ray, float3 loc1, float3 loc2, float rad, float3* pos, float3* nrm, float* lrp) {
    float3 tdd = loc2-loc1;
    float tdl2 = Len2(tdd);
    float tdl = sqrt(tdl2);
    float3 td = tdd / tdl;
    float2 lts = LineNearest(ray.pos, loc1, ray.dir, td);
    if (lts.x < -99 && lts.y < -99) return -1;
	float3 p1 = loc1 + lts.y * td;
	float3 p2 = ray.pos + lts.x * ray.dir;
    
    float lp12 = Len(p1 - p2);
	if (lp12 > rad)
        return -1;
    else {
        float p232 = rad*rad - lp12*lp12;
		float p23 = sqrt(p232);
        float3 d1;
        if (lp12 < 0.0001) {
            float3 td1 = cross(ray.dir, td);
            d1 = normalize(cross(td, td1));
        }
        else {
            d1 = cross(td, normalize(p2 - p1));
        }
        
        float rtd = dot(ray.dir, td);
        float phi = 1;
        if (rtd < 0.9999f && rtd > -0.9999f) {
            phi = dot(ray.dir, d1);
            if (phi < 0) phi *= -1;
        }
        float lf = p23 / phi;
        
        *pos = p2 - ray.dir * lf;
        
        float l3f2 = lf * lf - p232;
        float l3f = 0;
        if (l3f2 > 0) l3f = sqrt(l3f2);
        
        float3 td2 = td;
        if (dot(td2, ray.dir) > 0) td2 *= -1;
        float3 cf = p1 + td2 * l3f;
        
        float lc2 = Len2(cf - loc1);
        if ((dot(cf - loc1, td) < 0) || (lc2 > tdl2))
            return -1;
        *lrp = sqrt(lc2 / tdl2);
        
        *nrm = normalize(*pos - cf);
        return lts.x - lf;
    }
}

static ray_st GetRay(info_st info, float2 uv) {
	ray_st ray;
	float3 pos;
	pos.x = uv.x;
	pos.y = uv.y;
	pos.z = -1;
	float3 pos2 = pos;
	pos2.z = 1;
	ray.pos = MatMul(info.IP, pos);
	ray.dir = normalize(MatMul(info.IP, pos2) - pos);
	return ray;
}

static float4 SkyAt(float3 dir, info_st info, __global float* bg) {
    //return (float4)(1, 1, 1, 1);
	float2 rf = -normalize((float2)(dir.x, dir.z));
	float cx = acos(clamp(rf.x, -0.9999f, 0.9999f))/(3.14159f*2);
	cx = mix(1-cx, cx, ceil(rf.y));
	float sy = asin(clamp(dir.y, -0.9999f, 0.9999f))/3.14159f;
	
    int x = (int)(cx * info.bgw);
    x = (x + 200) % info.bgw;
    int y = (int)((sy + 0.5f) * info.bgh);
    int off = clamp(x + y * info.bgw, 0, info.bgw * info.bgh);
	return (float4)(bg[off*3], bg[off*3 + 1], bg[off*3 + 2], 1) * info.str;//vec2(cx , sy + 0.5);
    //return (float4)(sy + 0.5f, 0, 0, 1);
}

static float4 Trace(
    info_st info, ray_st ray, uint rnd,
    __global float* bg,
    __global float* ballPoss, int ballCnt,
    __global int* connIds, int connCnt,
    __global uchar* ballIds,
    __global uchar* ballRads
) {
	ray_st rays[5];
    float str = 1;
	rays[0] = ray;
	float4 res = (float4)(1,1,1,1);
	for (int a = 0; a < 4; a++) {
		ray = rays[a];
		float3 hitpos, hitnrm, hp, hn;
        float its = -1;
        float its2 = -1;
        int thid = 0;
        for (int a = 0; a < ballCnt; a++) {
            its2 = Int_Ball(ray, (float3)(ballPoss[a*3], ballPoss[a*3+1], ballPoss[a*3+2]), ballRads[a] * 0.1f / 127, &hp, &hn);
            if ((its < 0) || ((its2 > 0) && (its2 < its))) {
                its = its2;
                hitpos = hp;
                hitnrm = hn;
                thid = a;
            }
        }
        for (int a = 0; a < connCnt; a++) {
            int i1 = connIds[a*2];
            int i2 = connIds[a*2+1];
            float ii = 0;
            its2 = Int_Tube(ray, (float3)(ballPoss[i1*3], ballPoss[i1*3+1], ballPoss[i1*3+2]), (float3)(ballPoss[i2*3], ballPoss[i2*3+1], ballPoss[i2*3+2]), 0.008f, &hp, &hn, &ii);
            if ((its < 0) || ((its2 > 0) && (its2 < its))) {
                its = its2;
                hitpos = hp;
                hitnrm = hn;
                thid = (ii>0.5f)? i2 : i1;
            }
        }
        
		if (its < 0) {
            if (a == 0)
                return (float4)(1, 1, 1, 1);
			else 
                return res * SkyAt(ray.dir, info, bg);
		}
        uchar colid = ballIds[thid];
        float4 col = (float4)(1, 1, 1, 1);
        if (colid == 1) col = (float4)(0.1f, 0.1f, 0.1f, 1);
        else if (colid == 2) col = (float4)(1, 0, 0, 1);
        else if (colid == 3) col = (float4)(0, 0, 1, 1);
        else if (colid == 4) col = (float4)(0, 1, 0, 1);
		mat_st mat = info.mat;
		//rays[a + 1] = Diffuse(hitpos, hitnrm, &rnd);//Refl(ray, hitpos, hitnrm);
        //str *= OrenNayar(-ray.dir, hitnrm, rays[a+1].dir, mat.rough);
		float uc = mat.metallic;
        float frr = (1-mat.ior)/(1+mat.ior);
        frr *= frr;
        float fres = frr + (1-frr)*pow(1-dot(-ray.dir, hitnrm), 5);
		if (Randf(&rnd) < (1 - ((1 - fres) * (1-mat.specular)))) {
			Beckmann(&hitnrm, 1-mat.gloss, &rnd);
			rays[a + 1] = Refl(ray, hitpos, hitnrm);
		}
		else {
			rays[a + 1] = Diffuse(hitpos, hitnrm, &rnd);
			uc = 1 - uc;
			res *= uc;
		}
		if (dot(hitnrm, rays[a+1].dir) < 0)
			return (float4)(0, 0, 0, 1);
		res *= str;
		if (uc > 0.5f) res *= col;
	}
	return (float4)(0, 0, 0, 1);
}

__kernel void _main_(
    __global float* res,
    info_st info,
    __global float* bg,
    __global float* ballPoss, int ballCnt,
    __global int* connIds, int connCnt,
    __global uchar* ballIds,
    __global uchar* ballRads
){
	float4 col = (float4)(0, 0, 0, 0);
    
	size_t id = get_global_id(0);
	float2 uv = (float2)((id % info.w + 0.5f)/info.w, (id / info.w + 0.5f)/info.h);
    
    uv = uv * 2 - 1;
	
	info.rnd += id;
    for (int a = 0; a < 4; a++) {
        info.rnd = Rand(info.rnd);
        float r1 = (info.rnd % 1000) * 0.002f - 1.f;
        info.rnd = Rand(info.rnd);
        float r2 = (info.rnd % 1000) * 0.002f - 1.f;
        ray_st ray = GetRay(info, uv + (float2)(0.33f * r1 / info.w, 0.33f * r2 / info.h));
        col += Trace(info, ray, info.rnd, bg, ballPoss, ballCnt, connIds, connCnt, ballIds, ballRads);
    }
    col /= 4;
    
	res[id * 4] = col.x;
	res[id * 4 + 1] = col.y;
	res[id * 4 + 2] = col.z;
	res[id * 4 + 3] = 1;//ceil(col.w);
}
)";
}