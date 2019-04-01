namespace glsl {
	const char fontBlurFrag[] = R"(
#version 330

uniform sampler2D tex;
uniform int size;
uniform int rad;
uniform bool isY;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / size;
	vec2 ct = (floor(uv * 16.0) + 0.5) / 16;
	vec2 dp = (uv - ct);
	float kernal[7] = float[]( 0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598);
	fragCol = vec4(0,0,0,1);
	for (int a = 0; a < 7; ++a) {
		float xx = (a-3.0) / size;
		float dd;
		if (isY) {
			dd = dp.y + xx;
		}
		else {
			dd = dp.x + xx;
		}
		float col = 0;
		if ((dd > (-0.5 / 16)) && (dd < (0.5 / 16))) {
			if (isY)
				col = texture(tex, vec2(uv.x, ct.y + dd)).r;
			else
				col = texture(tex, vec2(ct.x + dd, uv.y)).r;
		}
		
		fragCol.r += (col * kernal[a]);
	}
}
)";
}