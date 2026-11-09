#version 330 core
// Cielo con degradado horizonte -> cenit, resplandor del sol y estrellas
in vec3 dir;
out vec4 color;

uniform vec3 horizonte;
uniform vec3 cenit;
uniform vec3 solDir;
uniform vec3 solColor;
uniform float estrellas;      // 0 = dia, 1 = noche

void main()
{
    vec3 d = normalize(dir);
    float h = clamp(d.y, 0.0, 1.0);
    vec3 c = mix(horizonte, cenit, pow(h, 0.55));

    // resplandor alrededor del sol (y disco)
    float s = max(dot(d, normalize(solDir)), 0.0);
    c += solColor * (pow(s, 6.0) * 0.30 + pow(s, 64.0) * 0.45 + pow(s, 1200.0) * 2.0);

    // debajo del horizonte se funde con la niebla
    c = mix(c, horizonte, clamp(-d.y * 8.0, 0.0, 1.0));

    // estrellas
    if (estrellas > 0.0 && d.y > 0.02)
    {
        vec3 q = floor(d * 260.0);
        float r = fract(sin(dot(q, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
        c += vec3(step(0.9988, r)) * estrellas * smoothstep(0.02, 0.3, d.y);
    }
    color = vec4(c, 1.0);
}
