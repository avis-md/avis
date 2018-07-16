namespace glsl {
    const char minVert[] = R"(
#version 330

void main() {
	float y = -1;
	if (gl_VertexID == 2 || gl_VertexID > 3) y = 1;
	gl_Position = vec4(mod(gl_VertexID, 2)*2-1, y, 0.5, 1.0);
}
)";
}