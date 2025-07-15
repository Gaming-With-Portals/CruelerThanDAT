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
    FragColor = vec4(0.0, fs_in.TexCoords.x, fs_in.TexCoords.y, 1.0);
}