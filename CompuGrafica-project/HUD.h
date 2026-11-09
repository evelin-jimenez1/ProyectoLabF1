#pragma once
// =============================================================
//  HUD: texto e indicadores dibujados con rectangulos
//  (fuente de 5x7 puntos hecha a mano, como un tablero LED).
//  No necesita librerias de fuentes.
//
//  Se usa para:
//    - el HUD en pantalla (cronometro, velocidad, DRS, mensajes)
//    - el tiempo en el marcador del Foro Sol
//    - el letrero de la zona de neumaticos
// =============================================================
#include <string>
#include <glm/glm.hpp>

void HUDInicializar();   // despues de glewInit(); carga Shader/hud.vs y hud.frag

// ---------- Construir lo que se va a dibujar ----------
// Coordenadas en un plano 2D (x a la derecha, y hacia abajo en
// pantalla). 'alto' = alto de una letra.
void HUDRect(float x, float y, float w, float h, glm::vec4 color);
void HUDTexto(const std::string& texto, float x, float y, float alto, glm::vec4 color);
float HUDAnchoTexto(const std::string& texto, float alto);

// Dibuja todo lo agregado con la matriz 'mvp' y vacia la lista.
// enPantalla = true: sin profundidad y con transparencia (HUD)
void HUDDibujar(const glm::mat4& mvp, bool enPantalla);

// ---------- Pantallas armadas ----------
// HUD del juego (cronometro, velocidad, DRS, mensajes, controles)
void DibujarHUDJuego(int ancho, int alto, int modoCamara);

// Tiempo del jugador en el marcador del Foro Sol
// (proj * view del cuadro actual)
void DibujarTiempoMarcador(const glm::mat4& projView);

// Letrero de la zona de neumaticos
void DibujarLetreroZona(const glm::mat4& projView);
