namespace glsl {
    const char bezierFrag[] = R"(
#version 330 core
in float t;

uniform vec4 col1;
uniform vec4 col2;

out vec4 color;
void main(){
	color = mix(col1, col2, t);
}
)";
}