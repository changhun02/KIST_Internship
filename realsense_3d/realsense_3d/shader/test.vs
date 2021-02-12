/*#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 ourColor;
out vec3 Normal;
//out vec3 FragPos;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{	 
	FragPos = vec3(model * vec4(aPos, 1.0));
	//gl_Position = projection * view * model * vec4(aPos, 1.0);
	ourColor = vec3(1.0f, 0.5f, 0.31f);
	Normal = aNormal;

	gl_Position = projection * view * vec4(FragPos, 1.0);
}*/


/*
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;  
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}*/

#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
   gl_Position = projection * view * model * vec4(aPos.x, aPos.y + 0.02, aPos.z, 1.0);
   //gl_Position = vec4((aPos.x + 0.1), -(aPos.y + 0.1), aPos.z, 1.0);
}