#version 330 core
// Papel picado: la textura es una mascara (blanco = papel, transparente = recorte)
in vec2 TexCoords;
out vec4 color;

uniform sampler2D texture_diffuse1;
uniform vec3 colorPapel;
uniform float brilloEscena;     // 1 = dia, menos de noche

void main()
{
    vec4 mascara = texture(texture_diffuse1, TexCoords);
    if (mascara.a < 0.5)
        discard;                    // aqui el papel esta recortado
    color = vec4(colorPapel * (0.75 + 0.25 * mascara.r) * brilloEscena, 1.0);
}
