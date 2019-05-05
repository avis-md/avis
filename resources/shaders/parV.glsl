#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 col;

const float PI = 3.14159;
const float fov = 60 * PI / 180;

uniform mat4 _MV, _P;
uniform vec3 camPos;
uniform vec3 camFwd;

uniform float orthoSz;
uniform vec2 screenSize;
uniform samplerBuffer radTex;
uniform float radScl;
uniform float spriteScl;
uniform bool oriented;
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

void main(){
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
		gl_PointSize = 0;
		return;
	}
	float radTexel = texelFetch(radTex,gl_VertexID).r;
	if (radTexel < 0) {
		gl_PointSize = 0;
		return;
	}

	gl_Position = _P*wpos;
	v2f_pos = wpos.xyz;
	v2f_id = gl_VertexID + 1;
	if (radScl <= 0) v2f_rad = tubesize * 2;
	else if (radTexel == 0) v2f_rad = 0.17 * radScl;
	else v2f_rad = 0.1 * radTexel * radScl;

	vec4 unitVec = _MV*vec4(1, 0, 0, 0);
	v2f_scl = length(unitVec);

	if (radScl == 0) gl_PointSize = 1;
	else {
		float rad = v2f_rad * v2f_scl;
		if (orthoSz < 0) {
			vec3 wdir = wpos.xyz - camPos;
			float d = length(wdir);
			float ca = dot(wdir / d, camFwd);
			float z = d * ca;
			if (z < 0.1) gl_PointSize = 0;
			else {
				float ym = z * tan(fov/2);
				float xm = ym * screenSize.x / screenSize.y;

				float th1 = acos(ca);
				float th2 = asin(rad / d);

				float rl = tan(th1 + th2) * z;
				rl = rl - sqrt(d * d - z * z);

				gl_PointSize = spriteScl * rl * min(screenSize.x / xm, screenSize.y / ym);
			}
		}
		else {
			gl_PointSize = spriteScl * screenSize.x * rad / orthoSz;
		}
		if (oriented) {
			gl_PointSize *= orienScl;
		}
	}
}