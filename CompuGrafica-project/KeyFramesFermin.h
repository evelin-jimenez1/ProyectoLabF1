#pragma once
// Angulos actuales (grados) que lee DibujarFermin()
extern float fBrazoDer, fAntebrazoDer, fBrazoIzq, fAntebrazoIzq, fCabeza;

void CargarKeyFramesFermin();          // guarda los keyframes y empieza (en bucle)
void AnimationFermin(float deltaTime); // cada cuadro
