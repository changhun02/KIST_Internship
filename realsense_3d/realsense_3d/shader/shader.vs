#version 330 core
layout (location = 0) in vec3 aPos;

void main(){
	float ratio = 1.777778f;
	gl_Position = vec4(aPos.x, aPos.y * ratio, aPos.z, 1.0);
}
