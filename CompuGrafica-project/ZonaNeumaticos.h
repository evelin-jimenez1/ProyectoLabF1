#pragma once
// =============================================================
//  Zona de neumaticos (zona de apoyo junto a la recta principal)
//  Neumaticos apilados (algunos con mantas termicas), barrera de
//  llantas, conos y extintores, todo hecho con primitivas.
// =============================================================
#include <glm/glm.hpp>
#include "Shader.h"

void ZonaNeumaticosInicializar();     // despues de PrimitivasInicializar() y SplineInicializar()
void DibujarZonaNeumaticos(Shader& shader);

// Matriz del origen de la zona (para poner su letrero)
// Ejes locales: -Z = sentido de la pista, +X = alejandose de la pista
glm::mat4 MatrizZonaNeumaticos();
