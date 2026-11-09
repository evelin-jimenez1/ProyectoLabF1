#version 330 core
// Triangulo gigante que cubre la pantalla (sin buffers, usa gl_VertexID)
out vec3 dir;                 // direccion de la vista en el mundo

uniform mat4 invProjection;
uniform mat4 invView;

void main()
{
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));   // (0,0) (2,0) (0,2)
    vec2 ndc = p * 2.0 - 1.0;
    gl_Position = vec4(ndc, 1.0, 1.0);

    vec4 v = invProjection * vec4(ndc, 1.0, 1.0);
    dir = mat3(invView) * normalize(v.xyz / v.w);
}
