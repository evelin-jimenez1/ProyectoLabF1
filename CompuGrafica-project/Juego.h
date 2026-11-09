#pragma once
// =============================================================
//  Juego: mecanicas de la contrarreloj
//    - control arcade del monoplaza sobre la spline
//    - 3 rivales que se activan conforme avanzas
//    - colisiones auto-muro y auto-rival
//    - checkpoints, cronometro, mejor vuelta
//    - DRS (se abre con keyframes, ver KeyFramesDRS.cpp; solo en las rectas)
//
//  Todo se mide en metros recorridos sobre la spline (ver Spline.h)
// =============================================================
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"

const int NUM_RIVALES = 3;
// Contrarreloj de VUELTAS_CARRERA vueltas: un solo jugador contra el cronometro, con el tiempo de
// cada vuelta, el mejor y el total. Al terminar la ultima aparece el podio. (1 = un solo tramo.)
const int VUELTAS_CARRERA = 5;
const int NUM_CHECKPOINTS = 6;        // el ultimo es la meta

// Estado de un monoplaza (sirve igual para el jugador y los rivales)
struct Carro
{
    float dist = 0.0f;          // metros recorridos en la vuelta actual
    float vel = 0.0f;           // m/s
    float lateral = 0.0f;       // m a la derecha (+) o izquierda (-) del centro
    float giroRuedas = 0.0f;    // grados que han rodado las llantas
    float direccion = 0.0f;     // grados de las llantas delanteras (+ = izquierda)
    float aperturaDRS = 0.0f;   // grados que esta abierto el DRS
};

struct Rival
{
    Carro carro;
    bool  activo = false;
    float activarEn = 0.0f;     // metros que debe recorrer el jugador para que aparezca
    float velObjetivo = 0.0f;   // velocidad predefinida (m/s)
    glm::vec3 color;            // color de la pintura
};

enum EstadoCarrera { ESPERANDO_ARRANQUE, CORRIENDO };

struct Controles
{
    bool acelerar = false, frenar = false, izquierda = false, derecha = false;
};

struct EstadoJuego
{
    Carro jugador;
    Rival rivales[NUM_RIVALES];
    EstadoCarrera estado = ESPERANDO_ARRANQUE;

    // Cronometro
    int   vuelta = 1;
    float tiempoTotal = 0.0f;       // s acumulados de las vueltas ya terminadas
    float tiempoVuelta = 0.0f;      // s
    float ultimaVuelta = 0.0f;      // s (0 = todavia no hay)
    float mejorVuelta = 0.0f;       // s (0 = todavia no hay)
    int   sigCheckpoint = 1;        // siguiente checkpoint por cruzar (1..NUM_CHECKPOINTS)
    float tiempoParcial = 0.0f;     // tiempo en el ultimo checkpoint
    float tiempoDesdeMeta = 999.0f; // s desde que se cruzo la meta (para la bandera a cuadros)

    // Sectores (la pista se divide en 3; cada uno termina en el checkpoint 2, 4 y 6)
    float tiempoCP[NUM_CHECKPOINTS] = {};   // tiempo de la vuelta al cruzar cada checkpoint
    float sector[3] = {};                   // ultimo tiempo de cada sector (0 = todavia no hay)
    float mejorSector[3] = {};              // mejor tiempo de cada sector
    bool  sectorRecord[3] = {};             // true si el ultimo tiempo fue el mejor (se ve morado)

    // DRS
    bool  drsDisponible = false;    // estamos en zona de DRS
    bool  drsAbierto = false;

    // Contadores y mensajes
    int   choquesMuro = 0;
    int   choquesRival = 0;
    std::string mensaje;            // texto grande al centro de la pantalla
    float tiempoMensaje = 0.0f;     // s que le quedan al mensaje
};

extern EstadoJuego juego;

// true mientras el jugador va por el carril de boxes (ver Boxes.cpp):
// ahi no hay choque con el muro ni con los rivales y el carro sale de la pista.
extern bool juegoEnPits;

void JuegoInicializar();                                // despues de SplineInicializar()
void JuegoActualizar(float dt, const Controles& c);     // cada cuadro
void JuegoBotonDRS();                                   // tecla R
void JuegoReiniciar();                                  // tecla T

// Matriz del carro en el mundo (sobre la spline, viendo hacia donde avanza)
glm::mat4 MatrizCarroMundo(const Carro& c);

// Dibuja al jugador y a los rivales activos
void JuegoDibujarCarros(Shader& shader, bool dibujarCabezaJugador);

int   RivalesActivos();
float VelocidadKmh();

// Convierte segundos a texto "M:SS.CC"
std::string FormatoTiempo(float segundos);
