#pragma once
// =============================================================
//  KeyFramesDRS: animacion por keyframes del aleron movil (DRS)
//  Mismo sistema que la practica de KeyFrames:
//    FRAME, saveFrame(), resetElements(), interpolation(), Animation()
//
//  Hay dos animaciones guardadas en keyframes:
//    - abrir:  0 -> 6 -> 18 -> 25 grados
//    - cerrar: 25 -> 12 -> 0 grados
// =============================================================

// Angulo actual del flap del DRS (lo lee DibujarCarro)
extern float drsAngulo;

void CargarKeyFramesAbrirDRS();    // carga los keyframes de abrir y empieza a reproducir
void CargarKeyFramesCerrarDRS();   // carga los keyframes de cerrar y empieza a reproducir
void AnimationDRS(float deltaTime); // se llama cada cuadro (avanza la reproduccion)
