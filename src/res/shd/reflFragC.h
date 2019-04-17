#pragma once
namespace glsl {
	const char reflFragC[] = R"(
#version 330

uniform mat4 _IP;
uniform vec2 screenSize;
uniform sampler2D inColor;
uniform sampler2D inNormal;
uniform sampler2D inDepth;

uniform float skyStrength;
uniform float skyStrDecay;
uniform float specStr;
uniform vec3 bgCol;

out vec4 fragCol;

void main () {
    const float _f = 0.57735;
    const vec3 ldir = vec3(-_f, _f, -_f);

    vec2 uv = gl_FragCoord.xy / screenSize;
    vec4 dCol = texture(inColor, uv);
    vec4 nCol = texture(inNormal, uv);
    float z = texture(inDepth, uv).x;
    float nClip = 0.01;
    float fClip = 500.0;

    float zLinear = (2 * nClip) / (fClip + nClip - z * (fClip - nClip));

    vec4 dc = vec4(uv.x*2-1, uv.y*2-1, z*2-1, 1);
    vec4 wPos = _IP*dc;
    wPos /= wPos.w;
    vec4 wPos2 = _IP*vec4(uv.x*2-1, uv.y*2-1, -1, 1);
    wPos2 /= wPos2.w;
    vec3 fwd = normalize(wPos.xyz - wPos2.xyz);
	
    if (z >= 1) {
        fragCol.rgb = bgCol;
    }
    else if (length(nCol.xyz) == 0) {
        fragCol.rgb = bgCol;
    }
    else {
        float dif = (max(dot(ldir, nCol.xyz), 0)*0.9 + 0.1);
        fragCol.rgb = dCol.xyz * dif * skyStrength * (1 - skyStrDecay * zLinear);
        fragCol.rgb += vec3(1,1,1) * dif * pow(max(dot(normalize(ldir - fwd), nCol.xyz), 0), 50) * 5 * specStr;
    }
    fragCol.a = 1;
}
)";
}