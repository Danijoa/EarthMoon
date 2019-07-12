#version 330

in  vec4 color;
in  vec4 pos;
out vec4 fColor;

uniform samplerCube uTexCube;

void
main()
{
	vec3 dir = pos.xyz;
	dir.y = -dir.y;
	fColor = texture(uTexCube, dir);
}
