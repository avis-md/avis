namespace glsl {
    const char bezierVert[] = R"(
#version 330 core
uniform vec2 pts[4];
uniform int count;
void main(){
	float t = gl_VertexID * 1.0 / count;
	float i = 1-t;
	gl_Position.xy = i*i*i*pts[0] + 3*i*i*t*pts[1] + 3*i*t*t*pts[2] + t*t*t*pts[3];
	gl_Position.z = gl_Position.w = 1.0;
}
)";
}