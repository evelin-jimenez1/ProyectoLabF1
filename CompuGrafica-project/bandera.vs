#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Recibe el tiempo desde C++

void main()
{
    vec3 pos = position;
    
    // Genera la onda. Si ondea muy rápido, baja el 5.0. Si la onda es muy grande, baja el 0.5.
    float wave = sin(pos.x * 2.0 + time * 5.0) * 0.5;
    
    // Aplica la onda al eje Z (profundidad) enmascarando con la textura para no mover el asta
    pos.z += wave * texCoords.x;

    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}