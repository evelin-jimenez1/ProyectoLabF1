#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;

    // Sombreado sencillo para que se note el relieve (bordes de la pista)
    vec3 sol = normalize(vec3(0.3, 1.0, 0.4));
    float luz = 0.45 + 0.55 * max(dot(normalize(Normal), sol), 0.0);

    FragColor = vec4(color * luz, 1.0);
}
