#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in mat4 instanceModelMat;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//out vec2 TexCoord;
out vec3 Normal;
out vec3 pos;

mat4 lookAt(vec3 dir, vec3 up) {
    dir = normalize(dir);
    up = normalize(up);
    vec3 s = cross(dir, up);
    vec3 u = cross(normalize(s), dir);

    return mat4(vec4(s, 0.0f),
                vec4(u, 0.0f),
                vec4(-dir, 0.0f),
                vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void main()
{
//    TexCoord = aTexCoord;
    Normal = (instanceModelMat * vec4(aNormal, 1.0)).xyz - (instanceModelMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz; // shift
    pos = (instanceModelMat * vec4(aPos, 1.0)).xyz;

    gl_Position = projection * view * model * instanceModelMat * vec4(aPos, 1.0);
}
