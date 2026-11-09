#version 330 core
out vec4 color;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform vec2 repetir; 

// Variables de luz
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;      // color e intensidad del sol (o de la luna)
uniform vec3 ambientColor;    // luz ambiental (cambia con dia / atardecer / noche)

// Sombras del sol (shadow mapping)
uniform sampler2D shadowMap;
uniform float sombrasActivas; // 0 = sin sombras (noche), 1 = con sombras

// Niebla (da profundidad y esconde el borde del pasto)
uniform vec3 fogColor;
uniform float fogDensity;

uniform float brilloPropio;   // 0 = usa la luz normal, 1 = se ve con su propio color

// Pintura de los autos rivales: 0 = textura normal, 1 = se pinta del color 'tinte'
// (conserva el dibujo de la textura, solo cambia el color)
uniform float usarTinte;
uniform vec3 tinte;

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

// Devuelve 1 si el punto recibe luz del sol y 0 si esta en sombra (PCF 3x3)
float CalcularLuzSol(vec3 norm, vec3 lightDir)
{
    vec3 p = FragPosLightSpace.xyz / FragPosLightSpace.w;
    p = p * 0.5 + 0.5;
    if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 1.0;

    float bias = max(0.0006 * (1.0 - dot(norm, lightDir)), 0.0002);
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float luz = 0.0;
    for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++)
        {
            float prof = texture(shadowMap, p.xy + vec2(x, y) * texel).r;
            luz += (p.z - bias > prof) ? 0.0 : 1.0;
        }
    return luz / 9.0;
}

void main()
{
    // 1. Componente Ambiental (Luz base para que no haya oscuridad total)
    vec3 ambient = ambientColor;
    
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
    if (texColor.a < 0.1) discard;   // PNG con transparencia (publico, rejas...)
    if (usarTinte > 0.5)
    {
        float brillo = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
        texColor.rgb = tinte * (0.35 + brillo * 1.4);
    }
    
    // Combinar luz con la textura
    vec3 luzFocos = vec3(0.0);
    for (int i = 0; i < NUM_FOCOS; i++)
        luzFocos += CalcularFoco(focos[i], norm, FragPos, viewDir);

    // El sol solo ilumina lo que no esta en sombra; el ambiente y los focos si
    float luzSol = mix(1.0, CalcularLuzSol(norm, lightDir), sombrasActivas);
    vec3 conLuz = (ambient + luzSol * (diffuse + specular) + luzFocos) * texColor.rgb;
    vec3 result = mix(conLuz, texColor.rgb, brilloPropio);

    // Niebla segun la distancia horizontal a la camara
    float dist = length((viewPos - FragPos).xz);
    float fog = 1.0 - exp(-pow(dist * fogDensity, 2.0));
    result = mix(result, fogColor, clamp(fog, 0.0, 1.0));

    color = vec4(result, 1.0);
    
}