#pragma once
// =============================================================
//  Ambiente: dia / atardecer / noche
//
//  Cambia (con una transicion suave) el color del cielo, el sol,
//  la luz ambiental, la niebla y si hay sombras:
//    DIA       sol alto, cielo azul, con sombras
//    ATARDECER cielo naranja-morado, sol bajo y calido, sombras largas
//    NOCHE     luna azul, estrellas, sin sombras (solo reflectores)
//  La tecla N cambia de modo (ver main.cpp).
// =============================================================
#include <glm/glm.hpp>
#include "Shader.h"

enum ModoAmbiente { AMB_DIA = 0, AMB_ATARDECER = 1, AMB_NOCHE = 2 };

// Valores actuales (ya mezclados entre el modo anterior y el nuevo)
struct ParamsAmbiente
{
    glm::vec3 horizonte;     // color del cielo en el horizonte (tambien la niebla)
    glm::vec3 cenit;         // color del cielo arriba
    glm::vec3 solDir;        // hacia donde esta el sol (vector unitario)
    glm::vec3 solColor;      // color del sol
    glm::vec3 luz;           // color * intensidad de la luz directa
    glm::vec3 ambiente;      // luz ambiental
    float niebla;            // densidad de la niebla
    float sombras;           // 0..1
    float estrellas;         // 0..1
    float escena;            // brillo para lo que tiene shader propio (bandera, papel picado)
};

void AmbienteInicializar();             // despues de glewInit()
void AmbienteAlternar();                // dia -> atardecer -> noche -> dia
void AmbienteActualizar(float dt);      // cada cuadro (transicion suave)
ModoAmbiente AmbienteModo();
const ParamsAmbiente& AmbienteParams();

// Dibuja el cielo (antes que el resto de la escena)
void AmbienteDibujarCielo(const glm::mat4& proyeccion, const glm::mat4& vista);

// Manda al shader de la escena la luz, la niebla y el color de ambiente
void AmbienteEnviar(Shader& shader);

void AmbienteLiberar();
