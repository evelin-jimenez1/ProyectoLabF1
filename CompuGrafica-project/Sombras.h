#pragma once
// =============================================================
//  Sombras: shadow mapping direccional (la luz es el sol)
//
//  1) SombrasRender() dibuja los objetos que proyectan sombra
//     (los monoplazas) desde el punto de vista del sol y guarda
//     la profundidad en una textura.
//  2) El shader de la escena (pista.frag) compara cada punto con
//     esa textura para saber si esta en sombra.
//
//  El mapa de sombras sigue al auto del jugador (asi alcanza una
//  resolucion buena con una sola textura de 2048 x 2048).
// =============================================================
#include <functional>
#include <glm/glm.hpp>
#include "Shader.h"

void SombrasInicializar();   // despues de glewInit()

// Dibuja la escena desde el sol. 'dibujar' recibe el shader de profundidad.
// Regresa la matriz (proyeccion * vista del sol) que usa el shader de la escena.
glm::mat4 SombrasRender(glm::vec3 centro, glm::vec3 direccionSol,
                        const std::function<void(Shader&)>& dibujar);

// Manda al shader de la escena el mapa de sombras y su matriz
// (usa la unidad de textura 5 para no chocar con las texturas de los modelos).
void SombrasEnviar(Shader& shader, const glm::mat4& lightSpace, float intensidad);

void SombrasLiberar();
