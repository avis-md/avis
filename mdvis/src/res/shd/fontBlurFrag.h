namespace glsl {
	const char fontBlurFrag[] = R"(
#version 330

uniform sampler2D tex;
uniform int size;
uniform int rad;
uniform bool isY;

out vec4 fragCol;

void main () {
	int size2 = size + rad * 2 * 16;
	vec2 uv = gl_FragCoord.xy / size2;
	vec2 ct = (floor(uv * 16.0) + 0.5) / 16;
	vec2 dp = (uv - ct) * (1 + rad * 32.0 / size);
	float kernal[21] = float[]( 0.011, 0.0164, 0.023, 0.031, 0.04, 0.05, 0.06, 0.07, 0.076, 0.08, 0.0852, 0.08, 0.076, 0.07, 0.06, 0.05, 0.04, 0.031, 0.023, 0.0164, 0.011 );
	
	fragCol = vec4(0,0,0,1);
	for (int a = 0; a < 21; ++a) {
		float xx = (a-10) * rad / (10.0 * size);
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