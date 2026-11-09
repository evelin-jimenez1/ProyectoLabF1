#pragma once
// =============================================================
//  Boxes (pit stop)
//
//  B = pedir entrar a boxes. Al pasar la meta el carro se desvia
//  solo hacia el carril de boxes (derecha de la recta principal,
//  junto a la zona de neumaticos), baja a 80 km/h, se detiene en su
//  box y 4 mecanicos cambian las llantas (animacion por keyframes
//  con interpolacion lineal, igual que el DRS). Despues sale a pista.
//  El tiempo parado y el limite de velocidad son la "penalizacion".
//
//  Flujo del carro:  NORMAL -> ENTRANDO -> PARADO -> SALIENDO -> NORMAL
// =============================================================
#include "Shader.h"
#include "Juego.h"

void BoxesInicializar();                    // despues de SplineInicializar() y PrimitivasInicializar()
void BoxesPedir();                          // tecla B (pide o cancela la parada)
void BoxesReiniciar();                      // tecla T
void BoxesFiltrarControles(Controles& c);   // ANTES de JuegoActualizar: en boxes maneja solo
void BoxesActualizar(float dt);             // DESPUES de JuegoActualizar
void BoxesDibujar(Shader& shader);          // carril, edificio de boxes y mecanicos
bool BoxesPedido();                         // true si ya pidio la parada (para el HUD)
