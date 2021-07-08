#version 330 core

in vec3 tc;
out vec4 fragColor;

uniform mat4 v_matrix;
uniform mat4 proj_matrix;
uniform samplerCube samp;

void main(void)
{
	fragColor = texture(samp,tc);
}
