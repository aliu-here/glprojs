#version 460 core
out vec4 FragColor;

//in vec2 TexCoord;

uniform vec3 lightColor;
uniform vec3 objectColor;
//uniform sampler2D ourTexture;

void main()
{
    FragColor = vec4(lightColor * objectColor, 1.0f);
//    vec4 tex = texture(ourTexture, TexCoord);
//    FragColor = tex;
//    FragColor = mix(tex, vec4(lightColor * objectColor, 1.0f), 1.0f);
} 
