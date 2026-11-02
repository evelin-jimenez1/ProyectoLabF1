#pragma once
// =============================================================
//  CargaModelos: modelos .obj (con Assimp) y geometria hecha a mano
// =============================================================
#include <GL/glew.h>
#include "Shader.h"

// Carga todos los modelos y crea la geometria.
// Se llama UNA vez, despues de glewInit().
void CargarModelos();

// Dibuja cada objeto. Antes de llamarlas, activen su textura
// con UsarTextura(...).
void DibujarPasto(Shader& shader);
void DibujarPista(Shader& shader);

// Libera la memoria al cerrar el programa
void LiberarModelos();
