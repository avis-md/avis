namespace glsl {
	const char marchVert[] = R"(#version 330 core
uniform ivec3 shp;

flat out ivec3 id;

void main() {
	int yz = (shp.y-1) * (shp.z-1);
	id.x = gl_VertexID / yz;
	int ty = gl_VertexID - id.x * yz;
	id.y = ty / (shp.z-1);
	id.z = int(mod(ty, shp.z-1));
})";}