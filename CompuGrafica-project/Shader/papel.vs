#version 330 core
// Papel picado: igual que pista.vs pero sin luz
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec3 pos = position;
    // La parte de abajo de la banderita ondea un poco (arriba esta amarrada al cable)
    float abajo = -position.y;                       // 0 arriba, 1 abajo
    pos.z += sin(position.x * 4.0 + time * 6.0) * 0.08 * abajo;
    gl_Position = projection * view * model * vec4(pos, 1.0);
    TexCoords = texCoords;
}
