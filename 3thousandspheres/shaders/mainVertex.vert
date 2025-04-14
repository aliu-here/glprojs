#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


//out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
//    TexCoord = aTexCoord;
    Normal = aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0f));

    gl_Position = projection * view * model * (vec4(aPos + aOffset, 1.0));
}
