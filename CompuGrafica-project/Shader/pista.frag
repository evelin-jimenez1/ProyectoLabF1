#version 330 core
out vec4 color;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec2 repetir; 

// Variables de luz
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
    // 1. Componente Ambiental (Luz base para que no haya oscuridad total)
    float ambientStrength = 0.4f;
    vec3 ambient = ambientStrength * lightColor;
    
    // 2. Componente Difusa (Luz direccional directa)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    // 3. Componente Especular (Brillo BLINN-PHONG)
    float specularStrength = 0.3f; // Un valor bajo para asfalto/pasto
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0f), 32.0f); // 32 es el nivel de brillo
    vec3 specular = specularStrength * spec * lightColor;
    
    // Multiplicador de coordenadas UV
    vec2 finalTexCoords = vec2(TexCoords.x * repetir.x, TexCoords.y * repetir.y);
    vec4 texColor = texture(texture_diffuse1, finalTexCoords);
    
    // Combinar luz con la textura
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    color = vec4(result, texColor.a);
}