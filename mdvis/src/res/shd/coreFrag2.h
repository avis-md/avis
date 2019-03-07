namespace glsl {
    const char coreFrag2[] = R"(
#version 330 core
in vec2 UV;
uniform sampler2D sampler;
uniform vec4 col;
uniform float level;
out vec4 color;
void main(){
	color = vec4(1, 1, 1, textureLod(sampler, UV, level).r)*col;
}
)";
}