#pragma once
// =============================================================
//  DiaDeMuertos: ambientacion del Gran Premio en Dia de Muertos
//    - Calaveras gigantes que brincan cuando pasa el carro
//      (maquina de estados, como la practica del perro)
//    - Camino de petalos de cempasuchil en la recta principal
//    - Arreglos de cempasuchil a los lados de la pista
//    - Tiras de papel picado que se mueven con el viento
// =============================================================
#include <glm/glm.hpp>
#include "Shader.h"

void DiaDeMuertosInicializar();      // despues de PrimitivasInicializar() y SplineInicializar()

// Avanza la maquina de estados de las calaveras.
// posCarro = posicion del carro del jugador en el mundo.
void AnimarCalaveras(float deltaTime, glm::vec3 posCarro);

// Calaveras, petalos y cempasuchil (con el shader de la pista)
void DibujarDiaDeMuertos(Shader& shader);

// Papel picado (con su propio shader, se mueve con el viento)
void DibujarPapelPicado(const glm::mat4& projection, const glm::mat4& view, float tiempo);
