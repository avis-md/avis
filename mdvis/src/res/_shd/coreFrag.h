namespace glsl {
    const char coreFrag[] = R"(
#version 330 core
in vec2 UV;
uniform sampler2D sampler;
uniform vec4 col;
uniform float level;
out vec4 color;
void main(){
	color = textureLod(sampler, UV, level)*col;
}
)";
}