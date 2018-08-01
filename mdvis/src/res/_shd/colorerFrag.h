namespace glsl {
	const char colererFrag[] = R"(
#version 330

uniform usampler2D idTex;
uniform vec2 screenSize;
uniform samplerBuffer id2col;
uniform sampler2D colList;
uniform int usegrad;
uniform vec4 gradcols[3];

layout (location = 0) out vec4 fragCol;

vec3 gradfill(float f) {
	if (f < 0.5) return mix(gradcols[0], gradcols[1], f*2).rgb;
	else return mix(gradcols[1], gradcols[2], f*2-1).rgb;
}

void main () {
	uvec4 tx = texture(idTex, gl_FragCoord.xy / screenSize);
	if (tx.y > 65535U) { //protein
		float v = float(tx.x);
		fragCol = vec4(gradfill(1 - (v / 65535)), 1);
		return;
	}
	int id = int(tx.x);
	if (id == 0) {
		fragCol = vec4(0.5, 0.5, 0.5, 1);
		return;
	}
	float cdf = texelFetch(id2col, id-1).r;
	int cd = int(cdf * 255);
	if (usegrad == 1) fragCol.rgb = gradfill(cd / 255.0);
	else fragCol = texture(colList, vec2((mod(cd, 16) + 0.5) / 16.0, ((cd / 16) + 0.5) / 16.0));
}
)";
}