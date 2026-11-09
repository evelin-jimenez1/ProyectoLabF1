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

void PilotoInicializar();   // despues de PrimitivasInicializar()

// direccion = grados de las llantas delanteras (+ = izquierda)
// dibujarCabeza = false en la camara onboard (la camara va en el casco)
void DibujarPiloto(Shader& shader, const glm::mat4& chasis, float direccion,
                   bool dibujarCabeza, glm::vec3 colorCasco);
