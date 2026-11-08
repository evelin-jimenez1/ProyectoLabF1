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

uniform float brilloPropio;   // 0 = usa la luz normal, 1 = se ve con su propio color

#define NUM_FOCOS 19 // 18 de los postes + 1 del letrero (letrero siempre encendido)
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 color;
    float intensidad;
};
uniform SpotLight focos[NUM_FOCOS];

vec3 CalcularFoco(SpotLight f, vec3 norm, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(f.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

    float distancia = length(f.position - fragPos);
    float atenuacion = 1.0 / (f.constant + f.linear * distancia + f.quadratic * distancia * distancia);

    // Cono: 1 dentro de cutOff, 0 fuera de outerCutOff, suave en medio
    float theta = dot(lightDir, normalize(-f.direction));
    float epsilon = f.cutOff - f.outerCutOff;
    float cono = clamp((theta - f.outerCutOff) / epsilon, 0.0, 1.0);

    return (diff + 0.3 * spec) * f.color * f.intensidad * atenuacion * cono;
}

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
    vec3 luzFocos = vec3(0.0);
    for (int i = 0; i < NUM_FOCOS; i++)
        luzFocos += CalcularFoco(focos[i], norm, FragPos, viewDir);

    vec3 conLuz = (ambient + diffuse + specular + luzFocos) * texColor.rgb;
    vec3 result = mix(conLuz, texColor.rgb, brilloPropio);
    color = vec4(result, texColor.a);
    
}