#pragma once
namespace glsl {
    const char parV[] = R"(
const float PI = 3.14159;
const float fov = 60 * PI / 180;

uniform samplerBuffer posTex;

uniform mat4 _MV, _P;
uniform vec3 camPos;
uniform vec3 camFwd;

uniform float orthoSz;
uniform vec2 screenSize;
uniform samplerBuffer radTex;
uniform float radScl;
uniform float spriteScl;
//uniform bool oriented;
uniform float orienScl;

uniform float tubesize;

uniform vec3 bbox;
uniform ivec3 imgCnt;
uniform ivec3 imgOff;

layout (std140) uniform clipping {
    vec4 clip_planes[6];
};
bool clipped(vec3 pos) {
    for (int a = 0; a < 6; ++a)  {
        if (dot(pos, clip_planes[a].xyz) > clip_planes[a].w)
            return true;
    }
    return false;
}

flat out int v2f_id;
out vec3 v2f_pos;
out float v2f_scl;
out float v2f_rad;

float maxrad(float rad, vec3 wpos) {
	float psz = 0;
	if (orthoSz < 0) {
		vec3 wdir = wpos - camPos;
		float d = length(wdir);
		float ca = dot(wdir / d, camFwd);
		float z = d * ca;
		
		float th1 = acos(ca);
		if (th1 > fov) {		
			return -1.0;
		}
		float ym = z * tan(fov/2);
		float xm = ym * screenSize.x / screenSize.y;

		float th2 = asin(rad / d);

		float rl = tan(th1 + th2) * z;
		rl = rl - sqrt(d * d - z * z);

		return spriteScl * rl * min(screenSize.x / xm, screenSize.y / ym);
	}
	else {
		return spriteScl * screenSize.x * rad / orthoSz;
	}
}

void setsprite(float w, float h, int vid) {
	//0  1  2  3  2  1
	w = w / screenSize.x;
	h = h / screenSize.y;
	if (mod(vid, 2) == 0) w = -w;
	if (vid > 1 && vid < 5) h = -h;
	gl_Position.xyz = gl_Position.xyz / gl_Position.w + vec3(w, h, 0);
	gl_Position.w = 1;
}

void main(){
	int pid = gl_VertexID / 6;
	int vid = gl_VertexID - pid * 6;

	vec3 pos = texelFetch(posTex, pid).xyz;

    vec3 ppos = pos;
    if (imgCnt.x > 0) {
        int iz = int(mod(gl_InstanceID, imgCnt.z));
        int gi2 = int(gl_InstanceID / imgCnt.z);
        int iy = int(mod(gi2, imgCnt.y));
        int ix = int(gi2 / imgCnt.y);
        ppos += bbox * (vec3(ix, iy, iz) - imgOff);
    }
	vec4 wpos = _MV*vec4(ppos, 1);
	wpos /= wpos.w;
	if (clipped(wpos.xyz)) {
		gl_Position = vec4(-2, -2, -2, 1);
		return;
	}
	float radTexel = texelFetch(radTex,pid).r;
	if (radTexel < 0) {
		gl_Position = vec4(-2, -2, -2, 1);
		return;
	}

	gl_Position = _P*wpos;
	v2f_pos = wpos.xyz;
	v2f_id = pid + 1;
	if (radScl <= 0) v2f_rad = tubesize * 2;
	else if (radTexel == 0) v2f_rad = 0.17 * radScl;
	else v2f_rad = 0.1 * radTexel * radScl;

	vec4 unitVec = _MV*vec4(1, 0, 0, 0);
	v2f_scl = length(unitVec);

	if (radScl == 0) setsprite(1, 1, vid);
	else {
		float psz = maxrad(v2f_rad * v2f_scl, wpos.xyz);
		if (psz < 0) {
			gl_Position = vec4(-2, -2, -2, 1);
			return;
		}
#ifdef oriented
			psz *= orienScl;
#endif
		setsprite(psz, psz, vid);
	}
}
)";
}