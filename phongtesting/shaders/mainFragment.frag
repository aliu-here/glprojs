#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D ourTexture;
uniform float ratio;

void main()
{
    vec4 tex = texture(ourTexture, TexCoord);
    FragColor = mix(tex, vec4(lightColor * objectColor, 1.0f), ratio);
} 
