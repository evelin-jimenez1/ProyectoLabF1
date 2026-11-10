#pragma once
// =============================================================
//  Comisario de meta: personaje articulado (modelado jerarquico)
//
//    torso (nodo padre)
//     ├─ cabeza (con gorra)
//     ├─ piernas
//     └─ brazo derecho
//         └─ antebrazo
//             └─ asta ─ bandera a cuadros
//
//  La bandera a cuadros se anima por keyframes con interpolacion
//  lineal (mismo sistema que el DRS, ver KeyFramesDRS.cpp):
//  sube el brazo y ondea la bandera cuando el jugador llega a la meta.
//
//  Ademas hay dos comisarios de ambientacion con bandera verde y
//  amarilla (solo se mueven un poco, no tienen animacion por keyframes).
// =============================================================
#include "Shader.h"

void ComisarioInicializar();             // despues de SplineInicializar()
void ComisarioActualizar(float dt);      // cada cuadro (avanza los keyframes y detecta la meta)
void ComisarioOndear();                  // empieza la animacion de la bandera
void ComisarioDibujar(Shader& shader, float tiempo);

// Persona articulada (la usan el comisario, los mecanicos de boxes y los pilotos del podio).
// 'base' = en el piso, mirando hacia -Z. brazo / antebrazo = brazo derecho (grados);
// brazoIzq = brazo izquierdo levantado (grados, 0 = pegado al cuerpo).
// texGorraColor = textura de la gorra (0 = blanca). Los pilotos del podio llevan gorra de su color;
// el casco solo se usa en la carrera (ver Piloto.cpp).
// texChaleco = textura de la camisa. texBandera = textura de la bandera (0 = sin asta ni bandera).
// bandera = giro de la tela alrededor del asta.
// adelante = los dos brazos hacia adelante (grados); juntar = los dos brazos hacia el centro del cuerpo
// (para sostener algo con las dos manos). variante = cambia piel, pelo, pantalon y zapatos.
// conGorra = false deja el pelo a la vista. Todas las personas tienen cara y manos.
#include <glm/glm.hpp>
#include <GL/glew.h>
void DibujarPersona(Shader& shader, const glm::mat4& base, float brazo, float antebrazo,
                    float bandera, GLuint texChaleco, GLuint texBandera, float brazoIzq = 0.0f,
                    GLuint texGorraColor = 0,
                    float adelante = 0.0f, float juntar = 0.0f, int variante = 0, bool conGorra = true);
