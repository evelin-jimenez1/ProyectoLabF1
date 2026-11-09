#pragma once
// =============================================================
//  Piloto simplificado (modelado jerarquico con primitivas)
//
//      chasis
//       ├─ volante                 (gira con la direccion)
//       └─ cadera
//           └─ torso
//               ├─ cabeza + casco  (voltea hacia la curva)
//               ├─ brazo izq -> antebrazo izq -> guante
//               └─ brazo der -> antebrazo der -> guante
//
//  'chasis' es la matriz del chasis en coordenadas del .obj
//  (la misma que usa DibujarCarro), asi el piloto hereda todo
//  el movimiento del carro.
// =============================================================
#include <glm/glm.hpp>
#include "Shader.h"

// Como reacciona el piloto a lo que hace el carro (lo calcula Juego.cpp)
struct ReaccionPiloto
{
    float empuje = 0.0f;      // -1 frenando fuerte ... +1 acelerando fuerte (la cabeza se va hacia atras o adelante)
    float impacto = 0.0f;     // 1 justo despues de un choque, baja solo a 0 (sacudida)
    float velocidad = 0.0f;   // 0..1 respecto a la velocidad maxima (vibracion del carro)
    float tiempo = 0.0f;      // segundos de juego, para las oscilaciones
};

void PilotoInicializar();   // despues de PrimitivasInicializar()

// direccion = grados de las llantas delanteras (+ = izquierda)
// dibujarCabeza = false en la camara onboard (la camara va en el casco)
void DibujarPiloto(Shader& shader, const glm::mat4& chasis, float direccion,
                   bool dibujarCabeza, glm::vec3 colorCasco,
                   const ReaccionPiloto& reaccion = ReaccionPiloto());
