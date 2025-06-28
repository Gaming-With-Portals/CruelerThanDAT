#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

out vec4 FragColor;

uniform vec3 lightDir;        
uniform vec3 viewPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main()
{
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    normal.g = 1.0 - normal.g; 
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    vec4 diffuseSample = texture(diffuseMap, fs_in.TexCoords);
    vec3 diffuseColor = diffuseSample.rgb;

    vec3 light = normalize(-lightDir);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    float diff = max(dot(normal, light), 0.0);
    vec3 diffuse = diff * diffuseColor * 2.0;

    vec3 halfwayDir = normalize(light + viewDir);

    vec3 ambient = 0.1 * diffuseColor;

    FragColor = vec4(ambient + diffuse, 1.0);
}