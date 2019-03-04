namespace glsl {
	const char colererFragPro[] = R"(
#version 330

uniform usampler2D idTex;
uniform vec2 screenSize;
uniform int proId;
uniform vec4 col;

layout (location = 0) out vec4 fragCol;

void main () {
	uvec4 tx = texture(idTex, gl_FragCoord.xy / screenSize);
	if (int(tx.y) == (proId + 65536)) {
		fragCol = col;
		return;
	}
	else discard;
}
)";
}