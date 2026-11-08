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
void DibujarGradas(Shader& shader);
void DibujarMarcador(Shader& shader, float tiempo);
void DibujarBandera(Shader shaderEstatico, Shader shaderViento);
void DibujarFermin(Shader shader);
void DibujarBarreras(Shader shader, float x, float y, float z, float rotY);
void DibujarTecpro_F1(Shader shader, float x, float y, float z, float rotY);
void DibujarPoste(Shader shader, float x, float y, float z, float rotY);
void DibujarAltar(Shader shader, float x, float y, float z, float rotY, float escala);
void DibujarFilaBarreras(Shader shader, float xIni, float zIni, float xFin, float zFin, bool voltear);
void DibujarPantalla(Shader& shader, GLuint textura, float x, float y, float z, float rotY);

// Libera la memoria al cerrar el programa
void LiberarModelos();
