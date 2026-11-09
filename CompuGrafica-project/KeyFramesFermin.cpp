// =============================================================
//  KeyFramesFermin.cpp
//  Animacion por keyframes de Fermin, el comisario de la pista.
//
//  Esta animacion se repite en bucle:
//  al llegar al ultimo keyframe vuelve a empezar desde el primero.
//
//  Secuencia: saluda con el brazo derecho, luego con el izquierdo,
//  se lleva la mano a la frente y regresa a la posicion de reposo.
// =============================================================
#include "KeyFramesFermin.h"

float fBrazoDer = 0, fAntebrazoDer = 0, fBrazoIzq = 0, fAntebrazoIzq = 0, fCabeza = 0;

#define MAX_FRAMES_FERMIN 20
const float PASOS_SEG_FERMIN = 60.0f;

typedef struct _frameFermin {
    float brazoDer, brazoDerInc;
    float anteDer, anteDerInc;
    float brazoIzq, brazoIzqInc;
    float anteIzq, anteIzqInc;
    float cabeza, cabezaInc;
    int steps;                       // pasos hasta el siguiente keyframe
} FRAME_FERMIN;

FRAME_FERMIN KeyFrameFermin[MAX_FRAMES_FERMIN];
int FrameIndexFermin = 0;
int playIndexFermin = 0;
float i_curr_steps_Fermin = 0;
bool playFermin = false;

void saveFrameFermin(float bD, float aD, float bI, float aI, float cab, int pasos)
{
    FRAME_FERMIN& f = KeyFrameFermin[FrameIndexFermin];
    f.brazoDer = bD; f.anteDer = aD; f.brazoIzq = bI; f.anteIzq = aI; f.cabeza = cab;
    f.brazoDerInc = f.anteDerInc = f.brazoIzqInc = f.anteIzqInc = f.cabezaInc = 0;
    f.steps = pasos;
    FrameIndexFermin++;
}

void resetElementsFermin()
{
    FRAME_FERMIN& f = KeyFrameFermin[0];
    fBrazoDer = f.brazoDer; fAntebrazoDer = f.anteDer;
    fBrazoIzq = f.brazoIzq; fAntebrazoIzq = f.anteIzq; fCabeza = f.cabeza;
}

void interpolationFermin()
{
    FRAME_FERMIN& a = KeyFrameFermin[playIndexFermin];
    FRAME_FERMIN& b = KeyFrameFermin[playIndexFermin + 1];
    a.brazoDerInc = (b.brazoDer - a.brazoDer) / a.steps;
    a.anteDerInc = (b.anteDer - a.anteDer) / a.steps;
    a.brazoIzqInc = (b.brazoIzq - a.brazoIzq) / a.steps;
    a.anteIzqInc = (b.anteIzq - a.anteIzq) / a.steps;
    a.cabezaInc = (b.cabeza - a.cabeza) / a.steps;
}

void CargarKeyFramesFermin()
{
    FrameIndexFermin = 0;
    //               bDer  aDer  bIzq  aIzq  cab  pasos
    saveFrameFermin(0, 0, 0, 0, 0, 30); // 0 reposo
    saveFrameFermin(80, 0, 0, 0, 0, 25); // 1 brazo der. al lado
    saveFrameFermin(150, -30, 0, 0, 0, 12); // 2 arriba, saluda
    saveFrameFermin(150, 30, 0, 0, 0, 12); // 3
    saveFrameFermin(150, -30, 0, 0, 0, 12); // 4
    saveFrameFermin(150, 30, 0, 0, 0, 15); // 5
    saveFrameFermin(150, 0, 0, 0, 0, 30); // 6 baja
    saveFrameFermin(0, 0, 0, 0, 0, 30); // 7 reposo
    saveFrameFermin(0, 0, 150, -30, 6, 12); // 8 brazo izq. saluda, ladea cabeza
    saveFrameFermin(0, 0, 150, 30, 6, 12); // 9
    saveFrameFermin(0, 0, 150, -30, 6, 15); // 10
    saveFrameFermin(0, 0, 150, 0, 6, 25); // 11
    saveFrameFermin(0, 0, 80, 120, -8, 40); // 12 mano a la frente
    saveFrameFermin(0, 0, 80, 120, -8, 30); // 13 se queda (pausa)
    saveFrameFermin(0, 0, 0, 0, 0, 1); // 14 reposo (= frame 0)

    playIndexFermin = 0;
    i_curr_steps_Fermin = 0;
    resetElementsFermin();
    interpolationFermin();
    playFermin = true;
}

void AnimationFermin(float deltaTime)
{
    if (!playFermin) return;

    FRAME_FERMIN& f = KeyFrameFermin[playIndexFermin];
    float avance = deltaTime * PASOS_SEG_FERMIN;
    float resta = f.steps - i_curr_steps_Fermin;
    if (avance > resta) avance = resta;

    fBrazoDer += f.brazoDerInc * avance;
    fAntebrazoDer += f.anteDerInc * avance;
    fBrazoIzq += f.brazoIzqInc * avance;
    fAntebrazoIzq += f.anteIzqInc * avance;
    fCabeza += f.cabezaInc * avance;
    i_curr_steps_Fermin += avance;

    if (i_curr_steps_Fermin >= f.steps - 0.0001f)
    {
        playIndexFermin++;
        if (playIndexFermin > FrameIndexFermin - 2)   // acabo -> vuelve a empezar
        {
            playIndexFermin = 0;
            resetElementsFermin();
        }
        i_curr_steps_Fermin = 0;
        interpolationFermin();
    }
}