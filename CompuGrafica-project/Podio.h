#pragma once
// =============================================================
//  Podio: escena final al terminar las vueltas (tecla P para verla cuando quieras)
//
//  - Podio de 3 escalones (oro, plata, bronce) con los pilotos en el orden de llegada,
//    festejando con los brazos arriba (modelo jerarquico de Comisario.cpp)
//  - Trofeo, bandera de Mexico y confeti
//  - Camara orbital alrededor del podio y tabla de resultados en pantalla
//
//  El orden sale del tiempo total que hizo el jugador en el contrarreloj contra el tiempo
//  total de cada rival (vueltas x longitud de la pista / velocidad predefinida del rival).
//  Se sale con T (reinicia la carrera) o con P.
// =============================================================
#include <glm/glm.hpp>
#include "Shader.h"

void PodioInicializar();                  // despues de JuegoInicializar() (usa los colores de los rivales)
void PodioActivar();                      // calcula el orden y pasa a la escena del podio
void PodioSalir();
bool PodioActivo();

// Vista de la camara orbital y su posicion (para viewPos del shader)
glm::mat4 PodioVista(float tiempo, glm::vec3& posicionCamara);

void PodioDibujar(Shader& shader, float tiempo);
void PodioHUD(int ancho, int alto);       // tabla de resultados en pantalla
