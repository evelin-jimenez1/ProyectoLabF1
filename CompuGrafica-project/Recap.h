#pragma once
// =============================================================
//  Recap: repeticion de la carrera en las pantallas del circuito
//
//  Mientras corres, el juego graba lo que se ve en pantalla (sin el HUD) a baja
//  resolucion y 10 cuadros por segundo. Al terminar cada vuelta, esa grabacion
//  pasa a reproducirse en un ciclo en TODAS las pantallas del circuito, como la
//  repeticion de la vuelta anterior en una transmision de TV. Si todavia no hay
//  ninguna vuelta grabada, las pantallas siguen con su video de siempre.
//
//  Se guarda en memoria del CPU (192 x 108 x RGB, maximo 90 s por vuelta) y cada
//  cuadro se sube a UNA textura que usan las pantallas.
// =============================================================
#include <GL/glew.h>

void RecapInicializar();                       // despues de glewInit()

// Graba un cuadro (a 10 por segundo) de lo que ya se dibujo en pantalla.
// Llamarla despues de dibujar la escena y ANTES de dibujar el HUD.
void RecapGrabar(int ancho, int alto, float dt, bool activo);

void RecapVueltaTerminada();                   // la vuelta grabada pasa a reproducirse
void RecapReiniciar();                         // tecla T: se borra todo
void RecapActualizar(float dt);                // avanza el video y sube el cuadro a la textura
GLuint RecapTextura();                         // 0 si todavia no hay repeticion que mostrar
