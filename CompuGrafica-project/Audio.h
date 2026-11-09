#pragma once
// =============================================================
//  Audio: sonidos del juego con la API MCI de Windows (winmm),
//  que ya viene con Windows: no hay que instalar ninguna libreria.
//  MCI permite que suenen varios archivos al mismo tiempo
//  (el ambiente en ciclo + bocina, choques, etc.).
//
//  Los .wav estan en la carpeta audio/ junto al proyecto.
//  En otros sistemas (Linux/Mac) las funciones no hacen nada.
// =============================================================

enum Sonido
{
    SONIDO_META,         // bocina al cruzar la meta
    SONIDO_CHECKPOINT,   // "bip" al pasar un checkpoint / arrancar
    SONIDO_CHOQUE,       // golpe contra muro o rival
    SONIDO_DRS,          // se abre el DRS
    NUM_SONIDOS
};

void AudioInicializar();              // abre los archivos y arranca el ambiente
void AudioReproducir(Sonido s);       // reproduce un efecto (desde el inicio)
void AudioAmbiente(bool encendido);   // prende / apaga el ambiente del estadio
void AudioCerrar();                   // al salir del programa
