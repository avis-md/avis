#pragma once
namespace glsl {
    const char dofFrag[] = R"(
#version 330 core

in vec2 uv;

uniform sampler2D mainTex;
uniform sampler2D depthTex;
uniform float plane;
uniform float focal;
uniform float aperture;
uniform vec2 screenSize;
uniform bool isOrtho;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;
	float kernel[25] = float[25](
		0.003765, 	0.015019, 	0.023792, 	0.015019, 	0.003765,
		0.015019, 	0.059912, 	0.094907, 	0.059912, 	0.015019,
		0.023792, 	0.094907, 	0.150342, 	0.094907, 	0.023792,
		0.015019, 	0.059912, 	0.094907, 	0.059912, 	0.015019,
		0.003765, 	0.015019, 	0.023792, 	0.015019, 	0.003765);

	float z = texture(depthTex, uv).r;
	const float nClip = 0.01;
	const float fClip = 500;
	float zLinear;
    if (isOrtho) zLinear = z;
    else zLinear = (2 * nClip) / (fClip + nClip - z * (fClip - nClip));

	float coc = abs(aperture * (focal * (zLinear - plane)) / (zLinear * (plane - focal)));

	fragCol = vec4(0,0,0,0);
	for (int a = 0; a < 5; ++a) {
		int xx = a-2;
		for (int b = 0; b < 5; ++b) {
			int yy = b-2;
			vec2 uv2 = uv + (vec2(xx, yy) / screenSize) * coc;
			fragCol += texture(mainTex, uv2) * kernel[a*5 + b];
		}
	}
}
)";
}