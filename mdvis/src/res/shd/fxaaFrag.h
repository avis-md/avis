#pragma once
namespace glsl {
	const char fxaaFrag[] = R"(
#version 330 core

uniform sampler2D tex;
uniform vec2 screenSize;

uniform float spanMax;
uniform float reduceMul;
uniform float reduceCutoff;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;

    vec3 rgbNW=texture(tex,uv+(vec2(-1.0,-1.0)/screenSize)).xyz;
    vec3 rgbNE=texture(tex,uv+(vec2(1.0,-1.0)/screenSize)).xyz;
    vec3 rgbSW=texture(tex,uv+(vec2(-1.0,1.0)/screenSize)).xyz;
    vec3 rgbSE=texture(tex,uv+(vec2(1.0,1.0)/screenSize)).xyz;
    vec3 rgbM=texture(tex,uv).xyz;

    vec3 luma=vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * reduceMul),
        reduceCutoff);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( spanMax,  spanMax),
          max(vec2(-spanMax, -spanMax),
          dir * rcpDirMin)) / screenSize;

    vec3 rgbA = (1.0/2.0) * (
        texture(tex, uv.xy + dir * (1.0/3.0 - 0.5)).xyz +
        texture(tex, uv.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture(tex, uv.xy + dir * (0.0/3.0 - 0.5)).xyz +
        texture(tex, uv.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)){
        fragCol.xyz=rgbA;
    }else{
        fragCol.xyz=rgbB;
    }
	fragCol.a = 1;
}
)";
}