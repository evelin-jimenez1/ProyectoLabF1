#pragma once
// =============================================================
//  CargaModelos: modelos .obj (con Assimp) y geometria hecha a mano
// =============================================================
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Piloto.h"

// Carga todos los modelos y crea la geometria.
// Se llama UNA vez, despues de glewInit().
void CargarModelos();

// Dibuja cada objeto. Antes de llamarlas, activen su textura
// con UsarTextura(...).
void DibujarPasto(Shader& shader);
void DibujarGradas(Shader& shader);
// Una grada (con su publico) colocada con la matriz 'model'. El origen del modulo
// es el centro del frente; el frente mira hacia +Z y el publico sube hacia -Z.
void DibujarGradasEn(Shader& shader, const glm::mat4& model);
void DibujarMarcador(Shader& shader, float tiempo);
void DibujarBandera(Shader shaderEstatico, Shader shaderViento);
void DibujarFermin(Shader shader);
void DibujarBarreras(Shader shader, float x, float y, float z, float rotY);
void DibujarTecpro_F1(Shader shader, float x, float y, float z, float rotY);
void DibujarPoste(Shader shader, float x, float y, float z, float rotY);
void DibujarAltar(Shader shader, float x, float y, float z, float rotY, float escala);
void DibujarFilaBarreras(Shader shader, float xIni, float zIni, float xFin, float zFin, bool voltear);
void DibujarPantalla(Shader& shader, GLuint textura, float x, float y, float z, float rotY);

// ---------- Monoplaza (modelado jerarquico) ----------
// base           = matriz del carro en el mundo (posicion + rotacion).
//                  El origen de 'base' queda en el PISO, al centro del carro,
//                  y el carro mira hacia -Z (igual que SplineAnguloY).
// giroRuedas     = grados que han rodado las llantas (avance / radio)
// anguloDireccion= grados que giran las llantas delanteras (+ = izquierda)
// aperturaDRS    = grados que se abre el aleron movil (0 = cerrado)
// pintura        = color de los rivales; (1,1,1) = textura original del jugador
// dibujarCabeza  = false en la camara onboard (la camara va dentro del casco)
// reaccion       = como se mueve el piloto segun lo que hace el carro (acelera, frena, gira, choca)
void DibujarCarro(Shader& shader, glm::mat4 base, float giroRuedas,
                  float anguloDireccion, float aperturaDRS,
                  glm::vec3 pintura = glm::vec3(1.0f), bool dibujarCabeza = true,
                  const ReaccionPiloto& reaccion = ReaccionPiloto());

// Radio de las llantas (m), para calcular cuanto giran
const float RADIO_LLANTA = 0.344f;

// Calavera de azucar con su matriz ya armada (ver DiaDeMuertos.cpp)
void DibujarModeloCalavera(Shader& shader, glm::mat4 model);

// Libera la memoria al cerrar el programa
void LiberarModelos();
