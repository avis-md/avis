#pragma once
namespace glsl {
	const char voxelFrag[] = R"(
#version 330
in vec3 v2f_uvw;
in vec3 v2f_wpos;

uniform vec2 screenSize;
uniform mat4 _IP;
uniform sampler3D tex;
out vec4 outColor;
void main()
{
	vec2 uv = gl_FragCoord.xy / screenSize;
	vec4 cc = vec4(uv.x*2-1, uv.y*2-1, -1, 1);
	vec4 cp = _IP*cc;
	vec3 camPos = (cp / cp.w).xyz;
	
	vec3 eye = v2f_wpos - camPos;
	eye = normalize(eye);
	
	outColor = vec4(0, 0, 0, 1);
	for (int i = 0; i < 100; i++) {
		vec3 uvw = v2f_uvw + eye * i / 100.0;
		if (uvw.x < 0 || uvw.x > 1 || uvw.y < 0 || uvw.y > 1 || uvw.y < 0 || uvw.y > 1)
			return;
		else
			outColor.rgb += texture(tex, uvw).rgb / 100.0;
	}
	
	outColor.rgb = texture(tex, v2f_uvw).xyz;
}
)";
}