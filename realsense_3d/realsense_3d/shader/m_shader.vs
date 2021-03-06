#version 330 core
layout (location = 0) in vec3 aPos;

out vec4 outColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	outColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
