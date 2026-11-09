// =============================================================
//  KeyFramesDRS.cpp
//  Igual que la practica de KeyFrames: cada FRAME guarda el valor
//  y su incremento; interpolation() calcula el incremento entre
//  un keyframe y el siguiente, y Animation() lo va sumando paso
//  a paso hasta llegar al siguiente keyframe.
// =============================================================
#include "KeyFramesDRS.h"
#include <cstdio>

float drsAngulo = 0.0f;          // angulo actual del flap (grados)

// =========================== KEYFRAMES ===========================
#define MAX_FRAMES 10
const float PASOS_POR_SEGUNDO = 60.0f;   // los pasos avanzan a 60 por segundo

typedef struct _frame {
	float drs;        // angulo del flap en este keyframe
	float drsInc;     // cuanto sube o baja en cada paso

	int steps;        // cuantos pasos dura este tramo
} FRAME;

FRAME KeyFrameDRS[MAX_FRAMES];
int FrameIndexDRS = 0;
bool playDRS = false;
int playIndexDRS = 0;
float i_curr_steps_DRS = 0;

void saveFrameDRS(float angulo, int pasos)
{
	KeyFrameDRS[FrameIndexDRS].drs = angulo;
	KeyFrameDRS[FrameIndexDRS].drsInc = 0.0f;
	KeyFrameDRS[FrameIndexDRS].steps = pasos;
	FrameIndexDRS++;
}

void resetElementsDRS(void)
{
	drsAngulo = KeyFrameDRS[0].drs;
}

void interpolationDRS(void)
{
	FRAME& a = KeyFrameDRS[playIndexDRS];
	FRAME& b = KeyFrameDRS[playIndexDRS + 1];
	a.drsInc = (b.drs - a.drs) / a.steps;
}

// Empieza a reproducir desde el keyframe 0
static void ReproducirDRS()
{
	playIndexDRS = 0;
	i_curr_steps_DRS = 0;
	resetElementsDRS();
	interpolationDRS();
	playDRS = true;
}

// Abrir: arranca lento, se abre rapido y llega suave a 25 grados
void CargarKeyFramesAbrirDRS()
{
	float desde = drsAngulo;     // si se estaba cerrando, abre desde donde va
	FrameIndexDRS = 0;
	if (desde < 6.0f)
	{
		saveFrameDRS(desde, 6);
		saveFrameDRS(6.0f, 7);
	}
	else
		saveFrameDRS(desde, 7);
	saveFrameDRS(18.0f, 5);
	saveFrameDRS(25.0f, 1);
	ReproducirDRS();
}

// Cerrar: baja rapido y se asienta en 0
void CargarKeyFramesCerrarDRS()
{
	float desde = drsAngulo;
	FrameIndexDRS = 0;
	saveFrameDRS(desde, 5);
	saveFrameDRS(desde * 0.5f, 6);
	saveFrameDRS(0.0f, 1);
	ReproducirDRS();
}

void AnimationDRS(float deltaTime)
{
	if (!playDRS) return;

	FRAME& f = KeyFrameDRS[playIndexDRS];
	float avance = deltaTime * PASOS_POR_SEGUNDO;
	float resta = f.steps - i_curr_steps_DRS;
	if (avance > resta) avance = resta;           // no pasarse del keyframe
	drsAngulo += f.drsInc * avance;
	i_curr_steps_DRS += avance;

	if (i_curr_steps_DRS >= f.steps - 0.0001f)    // termino el tramo?
	{
		playIndexDRS++;
		if (playIndexDRS > FrameIndexDRS - 2)     // termino toda la animacion?
		{
			drsAngulo = KeyFrameDRS[FrameIndexDRS - 1].drs;
			playIndexDRS = 0;
			playDRS = false;
		}
		else
		{
			i_curr_steps_DRS = 0;
			interpolationDRS();
		}
	}
}
