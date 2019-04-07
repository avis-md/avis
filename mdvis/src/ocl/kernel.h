#pragma once

namespace ocl {
	const char raykernel[] = R"(
#define EPSILON 0.00001f

typedef struct _Ray
{
    /// xyz - origin, w - max range
    float4 o;
    /// xyz - direction, w - time
    float4 d;
    /// x - ray mask, y - activity flag
    int2 extra;
    /// Padding
    float2 padding;
} Ray;

typedef struct _Camera
    {
        // Camera coordinate frame
        float3 forward;
        float3 up;
        float3 p;
        
        // Near and far Z
        float2 zcap;
    } Camera;

typedef struct _Intersection
{
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

float4 ConvertFromBarycentric(__global const float* vec, 
                            __global const int* ind, 
                            int prim_id, 
                            __global const float4* uvwt)
{
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

static float3 RandVec(uint* r) {
	float th = 2 * 3.14159f * Randf(r);
	float z = -1 + 2 * Randf(r);
	float sz = sqrt(1 - z*z);
	return (float3)(sz * cos(th), sz * sin(th), z);
}

static float3 RandHemi(uint* r) {
    float r1 = sqrt(Randf(r));
    float r2 = Randf(r) * 2 * 3.14159f;
    return (float3)(r1 * cos(r2), r1 * sin(r2), sqrt(std::max(1 - r1, 0.f)));
}

static void GetTans(float3 n, float3* t1, float3* t2) {
    *t1 = normalize(cross(n, (fabs(n.x) > 0.9999f) ? (float3)(0, 0, 1) : (float3)(1, 0, 0)));
    *t2 = normalize(cross(n, *t1));
}

static void Beckmann(float3* nrm, float rough, uint* rnd) {
    if (rough == 0) return;
	float a = rough * rough;
	float t1 = atan(sqrt(-a*log(1 - Randf(rnd))));
	float t2 = Randf(rnd) * 2 * 3.14159f;
	float3 n1, n2;
    GetTans(*nrm, &n1, &n2);
	*nrm = *nrm * cos(t1) + (n1 * cos(t2) + n2 * sin(t2)) * sin(t1);
}

static float3 SkyAt(float3 dir, __global float* bg) {
    const int bgw = 3200;
    const int bgh = bgw/2;
    
	float2 rf = -(float2)(dir.x, dir.z);
	float cx = acos(clamp(rf.x, -0.9999f, 0.9999f))/(3.14159f*2);
	cx = mix(1-cx, cx, ceil(rf.y));
	float sy = asin(clamp(dir.y, -0.9999f, 0.9999f))/3.14159f;
    int x = (int)(cx * bgw);
    x = (x + 200) % bgw;
    int y = (int)((sy + 0.5f) * bgh);
    int off = clamp(x + y * bgw, 0, bgw * bgh);
	return (float3)(bg[off*3], bg[off*3 + 1], bg[off*3 + 2]);
}


__kernel void GeneratePerspectiveRays(__global Ray* rays,
                                    __global const Camera* cam,
                                    int width,
                                    int height)
{
    int2 globalid;
    globalid.x  = get_global_id(0);
    globalid.y  = get_global_id(1);

    // Check borders
    if (globalid.x < width && globalid.y < height)
    {
        const float xstep = 2.f / (float)width;
        const float ystep = 2.f / (float)height;
        float x = -1.f + xstep * (float)globalid.x;
        float y = ystep * (float)globalid.y;
        float z = cam->zcap.x;
        // Perspective view
        int k = globalid.y * width + globalid.x;
        rays[k].o.xyz = cam->p;
        rays[k].d.x = x - cam->p.x;
        rays[k].d.y = y - cam->p.y;
        rays[k].d.z = z - cam->p.z;
        rays[k].o.w = cam->zcap.y;

        rays[k].extra.x = 0xFFFFFFFF;
        rays[k].extra.y = 0xFFFFFFFF;
    }
}


__kernel void Shading(//scene
                __global float* positions,
                __global float* normals,
                __global int* ids,
                __global float* colors,
                __global int* indents,
                __global Intersection* isect,
                float weight,
                int width,
                int height,
                __global unsigned char* out, //actual color
                __global float4* ocol, //ray color
                __global Ray* ray,
                int rng,
                __global float* accum,
                int smps,
                __global float* bg)
{
    int2 globalid;
    globalid.x  = get_global_id(0);
    globalid.y  = get_global_id(1);

    // Check borders
    if (globalid.x < width && globalid.y < height)
    {
        int k = globalid.y * width + globalid.x;
        int shape_id = isect[k].shapeid;
        int prim_id = isect[k].primid;

		const float str = 1;

		uint rnd = (uint)(k + rng);
		float4 col = ocol[k];
	
        if (shape_id != -1 && prim_id != -1)// && occl[k] == -1)
        {
            // Calculate position and normal of the intersection point
            int ind = indents[shape_id];

            float4 pos = ConvertFromBarycentric(positions + ind*3, ids + ind, prim_id, &isect[k].uvwt);
            float3 norm = ConvertFromBarycentric(normals + ind*3, ids + ind, prim_id, &isect[k].uvwt).xyz;
            norm = normalize(norm);

            //triangle diffuse color
            int color_id = ind + prim_id*3;
            float4 diff_col = (float4)( colors[color_id],
                                        colors[color_id + 1],
                                        colors[color_id + 2], 1.f);

			if (ray[k].o.w > 0.01f) {
                float dif = diff_col.x + diff_col.y + diff_col.z;
                
                float3 ro = ray[k].o.xyz;
                float3 rd = normalize(ray[k].d.xyz);

                float frr = 0;//(1 - mat.ior) / (1 + mat.ior);
                frr *= frr;
                float fres = frr + (1 - frr)*pow(1 - dot(-rd, norm), 5);
                //if (Randf(&rnd) < (1 - ((1 - fres) * (1 - mat.specular)))) {

                if ((Randf(&rnd) < (1 - ((1 - fres) * 0.9f))) || dif > 0.1f) {
                    //Beckmann(&norm, 1 - mat.gloss, &rnd);
                    if (dif > 0.01f) Beckmann(&norm, 0.15f, &rnd);
                    else Beckmann(&norm, 0.05f, &rnd);
                    norm = rd - 2 * dot(rd, norm.xyz) / (norm.x*norm.x + norm.y*norm.y + norm.z*norm.z) * norm;
                    diff_col.xyz = (float3)(1, 1, 1);
                }
                else {
                    float3 t1, t2;
                    GetTans(norm, &t1, &t2);
                    float3 rh = RandHemi(&rnd);
                    float3 nrm = t1*rh.x + t2*rh.y + norm*rh.z;
                    //ocol[k].xyz *= dot(nrm, norm);
                    norm = nrm;
                    if (dif < 0.01f)
                        diff_col.xyz = (float3)(1, 1, 1);
                }
				diff_col.xyz = normalize(norm);

                if (ocol[k].w < 0.5f)
                    ocol[k].xyz = diff_col.xyz;
                else
                    ocol[k].xyz *= diff_col.xyz;
                ocol[k].w = 1;


                ray[k].d.xyz = norm;
                pos.xyz += norm * EPSILON;
                ray[k].o.xyz = pos.xyz;
			}
        }
        else {
            float3 bgc = SkyAt(normalize(ray[k].d.xyz), bg);
            if (col.w < 0.5f) {
                col.xyz = bgc;//(float3)(0, 0, 0);
            }
            else
                col.xyz *= bgc * (1 - acos(ray[k].d.y)/3.14159f);
            accum[k * 3] += std::max(col.x, 0.f);
            accum[k * 3 + 1] += std::max(col.y, 0.f);
            accum[k * 3 + 2] += std::max(col.z, 0.f);
            ray[k].o = 0;
		}
		out[k * 4] = clamp(accum[k * 3] / smps, 0.f, 1.f) * 255;
		out[k * 4 + 1] = clamp(accum[k * 3 + 1] / smps, 0.f, 1.f) * 255;
		out[k * 4 + 2] = clamp(accum[k * 3 + 2] / smps, 0.f, 1.f) * 255;
		out[k * 4 + 3] = 255;
    }
}
)";
const int raykernel_sz = sizeof(raykernel);
}