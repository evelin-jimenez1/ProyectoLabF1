#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D texture_diffuse1; // Assimp usa este nombre por defecto
uniform float brilloEscena;         // 1 = dia, menos de noche

void main()
{
    FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb * brilloEscena, 1.0);
}