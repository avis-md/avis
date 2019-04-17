#pragma once
namespace glsl {
	const char colorPickerH2[] = R"(
#version 330 core
in vec2 UV;
uniform vec4 gradcols[3];
out vec4 color;
void main(){
	float f = UV.y;
	color.rgb = (gradcols[0].rgb * clamp(2 - 4 * f, 0, 1)
		+ gradcols[1].rgb * clamp(2 - abs(2 - 4 * f), 0, 1)
		+ gradcols[2].rgb * clamp(4 * f - 2, 0, 1));
	color.a = 1;
}
)"; }