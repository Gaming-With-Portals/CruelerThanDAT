#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 lightDir;        
uniform vec3 viewPos;

uniform sampler2D diffuseMap;

void main()
{
    vec4 diffuseSample = texture(diffuseMap, fs_in.TexCoords);
    vec3 diffuseColor = diffuseSample.rgb;

    if (diffuseSample.a < 0.1)
        discard;

    vec3 normal = normalize(fs_in.Normal); 
    vec3 light = normalize(-lightDir);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(light + viewDir);

    float diff = max(dot(normal, light), 0.0);
    vec3 diffuse = diff * diffuseColor * 2.0;

    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0);

    vec3 ambient = 0.1 * diffuseColor;

    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(finalColor, diffuseSample.a);
}