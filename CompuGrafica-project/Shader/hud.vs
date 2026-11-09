#version 330 core
// HUD y textos LED: posicion 2D + color por vertice
layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;

out vec4 Color;
uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    Color = color;
}
