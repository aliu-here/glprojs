#version 460 core
out vec4 FragColor;

//in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

//uniform sampler2D ourTexture;
//uniform float ratio;

vec3 calcSpecular(float df)
{
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 reflectDir = reflect(-normalize(lightPos - FragPos), normalize(Normal));
    
    float c = float(df > 0.0) - float(df < 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32) * c;

    vec3 specular = specularStrength * spec * lightColor;  
    return specular;
}

vec3 phongShading()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
            
    vec3 result = (ambient + diffuse + calcSpecular(diff)) * objectColor;
    return result;
}

void main()
{
    vec3 shaded = phongShading();

    FragColor = vec4(shaded, 1.0);

 //   vec4 tex = texture(ourTexture, TexCoord);
 //   FragColor = mix(tex, vec4(shaded, 1.0f), ratio);
} 
