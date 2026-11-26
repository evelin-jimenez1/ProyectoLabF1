#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;   // posicion del fragmento visto desde el sol (sombras)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

// Animacion del publico cuadrado de las gradas (solo cuando animarPublico = 1)
uniform int   animarPublico;
uniform float tiempoPublico;
uniform float faseModulo;

void main()
{
    vec3 pos = position;
    if (animarPublico == 1)
    {
        // Cada muneco es una celda de la grada: 13 por fila (0.8 m) y 6 filas (1.5 m); sus vertices comparten fase
        float col = floor((position.x + 5.35) / 0.8);
        float fila = clamp(floor((position.z + 8.0) / 1.5), 0.0, 5.0);
        float h  = fract(sin(col * 12.9898 + fila * 78.233 + faseModulo) * 43758.5453);
        float h2 = fract(h * 7.13 + 0.37);
        float vel = 4.0 + 5.0 * h2;                       // ritmo propio de cada muneco
        float s = sin(tiempoPublico * vel + h * 6.2831);
        float altura = max(position.y - (2.01 - 0.4 * fila), 0.0);   // altura sobre los pies (2.01 m en la fila de atras)
        pos.y += max(s, 0.0) * (0.06 + 0.10 * h2) * smoothstep(0.0, 0.5, altura);   // rebote (celebracion)
        pos.x += sin(tiempoPublico * vel * 0.5 + h * 6.2831) * 0.07 * altura;       // balanceo del cuerpo
        pos.z += cos(tiempoPublico * vel * 0.5 + h2 * 6.2831) * 0.03 * altura;
    }
    gl_Position = projection * view * model * vec4(pos, 1.0f);
    // Posición del vértice en el mundo real
    FragPos = vec3(model * vec4(pos, 1.0f));
    // Matriz normal para evitar deformación de la luz al escalar objetos
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}