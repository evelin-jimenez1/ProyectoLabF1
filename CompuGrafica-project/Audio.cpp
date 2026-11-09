// =============================================================
//  Audio.cpp - reproduccion con mciSendString (Windows)
//
//  Cada archivo se abre una vez con un alias y despues solo se
//  le dice "play alias from 0". El tipo "mpegvideo" permite usar
//  "repeat" para el sonido de ambiente en ciclo.
// =============================================================
#include "Audio.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <iostream>
#pragma comment(lib, "winmm.lib")

static const char* ARCHIVOS[NUM_SONIDOS] = {
    "audio/meta.wav",
    "audio/checkpoint.wav",
    "audio/choque.wav",
    "audio/drs.wav",
};
static const char* ALIAS[NUM_SONIDOS] = { "meta", "checkpoint", "choque", "drs" };
static bool abierto[NUM_SONIDOS] = { false };
static bool ambienteAbierto = false;

static bool Mci(const std::string& comando)
{
    MCIERROR err = mciSendStringA(comando.c_str(), NULL, 0, NULL);
    if (err != 0)
    {
        char texto[256];
        mciGetErrorStringA(err, texto, sizeof(texto));
        std::cout << "Audio: " << comando << " -> " << texto << std::endl;
        return false;
    }
    return true;
}

void AudioInicializar()
{
    for (int i = 0; i < NUM_SONIDOS; i++)
        abierto[i] = Mci(std::string("open \"") + ARCHIVOS[i] + "\" type mpegvideo alias " + ALIAS[i]);

    ambienteAbierto = Mci("open \"audio/ambiente.wav\" type mpegvideo alias ambiente");
    if (ambienteAbierto) Mci("setaudio ambiente volume to 350");   // 0..1000
    AudioAmbiente(true);
}

void AudioReproducir(Sonido s)
{
    if (s < 0 || s >= NUM_SONIDOS || !abierto[s]) return;
    Mci(std::string("play ") + ALIAS[s] + " from 0");
}

void AudioAmbiente(bool encendido)
{
    if (!ambienteAbierto) return;
    if (encendido) Mci("play ambiente from 0 repeat");
    else           Mci("stop ambiente");
}

void AudioCerrar()
{
    mciSendStringA("close all", NULL, 0, NULL);
}

#else
// Sin Windows: sin sonido (para poder compilar en otros sistemas)
void AudioInicializar() {}
void AudioReproducir(Sonido) {}
void AudioAmbiente(bool) {}
void AudioCerrar() {}
#endif
