namespace glsl {
    const char bezierVert[] = R"(
#version 330 core
uniform vec2 pts[4];
uniform int count;
uniform vec2 thick;

out float t;

vec2 ptat(float t) {
	float i = 1 - t;
	return i*i*i*pts[0] + 3*i*i*t*pts[1] + 3*i*t*t*pts[2] + t*t*t*pts[3];
}

void main(){
	int i = int(mod(gl_VertexID, 6));
	int ti = int(floor(gl_VertexID * 0.16667f)) + int(mod(i, 2));
	float dt = 1.0 / count;
	t = ti * dt;
	
	vec2 here = ptat(t);
	vec2 dir = ptat(t + dt) - ptat(t - dt);
	vec2 nrm = normalize(vec2(-dir.y, dir.x));

	if (i < 2 || i == 5) {
		gl_Position.xy = here + thick * nrm;
	}
	else {
		gl_Position.xy = here - thick * nrm;
	}
	gl_Position.z = gl_Position.w = 1.0;
}
)";
}