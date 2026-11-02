#version 330 core

// Luz direccional (el sol)
struct LuzDireccional
{
    vec3 direccion;
    vec3 ambiente;
    vec3 difusa;
    vec3 especular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;   // mismo nombre que usa Model.h / Mesh.h
uniform LuzDireccional luz;
uniform vec3 viewPos;

void main()
{
    vec3 albedo = texture(texture_diffuse1, TexCoords).rgb;

    vec3 N = normalize(Normal);
    vec3 L = normalize(-luz.direccion);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);                 // Blinn-Phong

    vec3 ambiente  = luz.ambiente * albedo;
    vec3 difusa    = luz.difusa * max(dot(N, L), 0.0) * albedo;
    vec3 especular = luz.especular * pow(max(dot(N, H), 0.0), 32.0);

    color = vec4(ambiente + difusa + especular, 1.0);
}
