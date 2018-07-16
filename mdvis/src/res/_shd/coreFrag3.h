namespace glsl {
    const char coreFrag3[] = R"(
#version 330 core
in vec2 UV;
uniform vec4 col;
out vec4 color;
void main(){
	color = col;
}
)";
}