#version 150

in vec3 position;
in vec3 color;
in vec2 texcoord;

out vec3 Color;
out vec2 Texcoord;

uniform mat4 model;
uniform mat4 worldViewProj;

uniform vec3 overrideColor;

void main()
{
    Color = overrideColor * color;
    Texcoord = texcoord;
	vec4 pos = model*vec4(position, 1.0);
    gl_Position = worldViewProj*pos;
}