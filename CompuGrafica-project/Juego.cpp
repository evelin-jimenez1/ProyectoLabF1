// =============================================================
//  Juego.cpp - mecanicas de la contrarreloj
// =============================================================
#include "Juego.h"
#include "Spline.h"
#include "CargaModelos.h"
#include "KeyFramesDRS.h"
#include "Audio.h"

#include <cmath>
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>

EstadoJuego juego;

// ---------- Fisica arcade ----------
const float VEL_MAX = 45.0f;            // ~160 km/h
const float VEL_MAX_DRS = 52.0f;        // ~187 km/h con el DRS abierto
const float ACELERACION = 12.0f;        // m/s^2
const float FRENADO = 30.0f;            // m/s^2
const float FRICCION = 4.0f;            // m/s^2 sin acelerar
const float VEL_LATERAL = 5.0f;         // m/s al moverse de lado
const float LATERAL_MAX = ANCHO_PISTA / 2.0f - 1.1f;   // 1.1 = medio ancho del carro

// ---------- Tamano del carro para las colisiones ----------
// Caja (AABB) medida sobre la spline: largo x ancho
const float LARGO_CARRO = 4.9f;
const float ANCHO_CARRO = 2.0f;

// ---------- Zonas de DRS (metros sobre la spline) ----------
// Recta principal (cruza la meta) y recta larga despues de las eses
struct Zona { float inicio, fin; };
static const Zona ZONAS_DRS[] = { { 1240.0f, 1500.0f }, { 0.0f, 190.0f }, { 335.0f, 480.0f } };

// La animacion del DRS por keyframes esta en KeyFramesDRS.cpp

static float distCheckpoint[NUM_CHECKPOINTS];   // metros de cada checkpoint
static float esperaChoque = 0.0f;               // evita repetir el aviso del mismo choque


std::string FormatoTiempo(float s)
{
    if (s < 0.0f) s = 0.0f;
    int min = (int)(s / 60.0f);
    float seg = s - min * 60.0f;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d:%05.2f", min, seg);
    return buf;
}

static void Mensaje(const std::string& texto, float segundos)
{
    juego.mensaje = texto;
    juego.tiempoMensaje = segundos;
}

static bool EnZonaDRS(float s)
{
    float L = SplineLongitud();
    s = std::fmod(s, L);
    if (s < 0.0f) s += L;
    for (const Zona& z : ZONAS_DRS)
        if (s >= z.inicio && s <= z.fin) return true;
    return false;
}

// Angulo de las llantas delanteras segun la curva que viene
static float DireccionSegunPista(float s)
{
    float d = SplineAnguloY(s + 4.0f) - SplineAnguloY(s);
    if (d > 180.0f)  d -= 360.0f;
    if (d < -180.0f) d += 360.0f;
    return d * 2.0f;
}

// Avanza un carro: posicion, llantas y direccion
static void MoverCarro(Carro& c, float dt, float extraDireccion)
{
    float avance = c.vel * dt;
    c.dist += avance;
    c.giroRuedas = std::fmod(c.giroRuedas + glm::degrees(avance / RADIO_LLANTA), 360.0f);
    float objetivo = glm::clamp(DireccionSegunPista(c.dist) + extraDireccion, -25.0f, 25.0f);
    c.direccion += (objetivo - c.direccion) * glm::min(1.0f, 8.0f * dt);   // suave
}


void JuegoInicializar()
{
    float L = SplineLongitud();
    for (int i = 0; i < NUM_CHECKPOINTS; i++)
        distCheckpoint[i] = L * (i + 1) / NUM_CHECKPOINTS;   // el ultimo = L = meta

    // Rivales: aparecen 60 m adelante del jugador cuando este llega a 'activarEn'
    juego.rivales[0].activarEn = 120.0f;  juego.rivales[0].velObjetivo = 30.0f;
    juego.rivales[0].carro.lateral = -1.6f; juego.rivales[0].color = glm::vec3(0.95f, 0.10f, 0.55f);  // rosa mexicano
    juego.rivales[1].activarEn = 520.0f;  juego.rivales[1].velObjetivo = 27.0f;
    juego.rivales[1].carro.lateral = 1.6f;  juego.rivales[1].color = glm::vec3(0.10f, 0.75f, 0.80f);  // turquesa
    juego.rivales[2].activarEn = 950.0f;  juego.rivales[2].velObjetivo = 32.0f;
    juego.rivales[2].carro.lateral = -1.6f; juego.rivales[2].color = glm::vec3(1.00f, 0.55f, 0.05f);  // cempasuchil

    JuegoReiniciar();
}


void JuegoReiniciar()
{
    juego.jugador = Carro();
    for (Rival& r : juego.rivales) { r.activo = false; r.carro.vel = 0.0f; }
    juego.estado = ESPERANDO_ARRANQUE;
    juego.vuelta = 1;
    juego.tiempoVuelta = 0.0f;
    juego.sigCheckpoint = 1;
    juego.tiempoParcial = 0.0f;
    juego.drsAbierto = false;
    juego.choquesMuro = juego.choquesRival = 0;
    CargarKeyFramesCerrarDRS();
    Mensaje("PRESIONA W PARA ARRANCAR", 9999.0f);
}


void JuegoBotonDRS()
{
    if (juego.drsAbierto)               // cerrar a mano
    {
        juego.drsAbierto = false;
        CargarKeyFramesCerrarDRS();
    }
    else if (juego.drsDisponible && juego.estado == CORRIENDO)
    {
        juego.drsAbierto = true;
        CargarKeyFramesAbrirDRS();
        AudioReproducir(SONIDO_DRS);
    }
    else
        Mensaje("DRS SOLO EN LAS RECTAS", 1.5f);
}


// ---------------- Colisiones ----------------
// Se revisan en coordenadas de la pista (metros recorridos y lateral),
// donde cada carro es una caja de LARGO_CARRO x ANCHO_CARRO.
static bool Chocan(const Carro& a, const Carro& b)
{
    return std::fabs(a.dist - b.dist) < LARGO_CARRO && std::fabs(a.lateral - b.lateral) < ANCHO_CARRO;
}

static void ColisionesRivales()
{
    Carro& j = juego.jugador;
    for (Rival& r : juego.rivales)
    {
        if (!r.activo || !Chocan(j, r.carro)) continue;

        bool jugadorAtras = j.dist < r.carro.dist;
        if (jugadorAtras)
        {
            // El jugador le pega por detras: rebota hacia atras y pierde velocidad
            j.dist = r.carro.dist - LARGO_CARRO - 0.5f;
            j.vel = glm::min(j.vel, r.carro.vel * 0.6f);
        }
        else
        {
            // El rival nos alcanza: se queda detras y frena
            r.carro.dist = j.dist - LARGO_CARRO - 0.5f;
            r.carro.vel = glm::min(r.carro.vel, j.vel * 0.6f);
        }
        // Empujon hacia los lados para separarlos
        float lado = (j.lateral >= r.carro.lateral) ? 1.0f : -1.0f;
        j.lateral = glm::clamp(j.lateral + 0.4f * lado, -LATERAL_MAX, LATERAL_MAX);

        if (esperaChoque <= 0.0f)   // un solo aviso por choque
        {
            juego.choquesRival++;
            Mensaje("CHOQUE CON RIVAL - REBASA POR UN LADO", 2.0f);
            AudioReproducir(SONIDO_CHOQUE);
        }
        esperaChoque = 1.0f;
    }
}


static void ActualizarRivales(float dt)
{
    const Carro& j = juego.jugador;
    for (int i = 0; i < NUM_RIVALES; i++)
    {
        Rival& r = juego.rivales[i];

        // Activacion progresiva
        if (!r.activo)
        {
            if (juego.estado == CORRIENDO && j.dist >= r.activarEn && j.dist < r.activarEn + 50.0f)
            {
                r.activo = true;
                r.carro.dist = j.dist + 60.0f;
                r.carro.vel = r.velObjetivo;
                Mensaje("RIVAL A LA VISTA", 1.5f);
            }
            continue;
        }

        // Si hay otro rival justo adelante en el mismo carril, no lo atraviesa: lo sigue
        float velDeseada = r.velObjetivo;
        for (int k = 0; k < NUM_RIVALES; k++)
        {
            const Rival& o = juego.rivales[k];
            if (k == i || !o.activo) continue;
            float d = o.carro.dist - r.carro.dist;
            if (d > 0.0f && d < LARGO_CARRO + 4.0f && std::fabs(o.carro.lateral - r.carro.lateral) < ANCHO_CARRO)
                velDeseada = glm::min(velDeseada, o.carro.vel);
        }
        // Recupera su velocidad poco a poco (despues de un choque)
        if (r.carro.vel < velDeseada) r.carro.vel = glm::min(velDeseada, r.carro.vel + 6.0f * dt);
        else                          r.carro.vel = velDeseada;

        MoverCarro(r.carro, dt, 0.0f);
    }
}


static void CruzarMeta()
{
    float L = SplineLongitud();
    juego.ultimaVuelta = juego.tiempoVuelta;
    bool record = (juego.mejorVuelta == 0.0f || juego.ultimaVuelta < juego.mejorVuelta);
    if (record) juego.mejorVuelta = juego.ultimaVuelta;

    Mensaje(std::string(record ? "NUEVO RECORD " : "VUELTA COMPLETADA ") + FormatoTiempo(juego.ultimaVuelta), 4.0f);
    AudioReproducir(SONIDO_META);
    juego.tiempoDesdeMeta = 0.0f;

    // Nueva vuelta: las distancias se cuentan otra vez desde la meta
    juego.vuelta++;
    juego.tiempoVuelta = 0.0f;
    juego.sigCheckpoint = 1;
    juego.jugador.dist -= L;
    for (Rival& r : juego.rivales)
    {
        if (!r.activo) continue;
        r.carro.dist -= L;
        // Los que se quedaron muy atras vuelven a aparecer mas adelante
        if (r.carro.dist < juego.jugador.dist - 100.0f) r.activo = false;
    }
}


void JuegoActualizar(float dt, const Controles& c)
{
    Carro& j = juego.jugador;
    if (juego.tiempoMensaje > 0.0f) juego.tiempoMensaje -= dt;
    if (esperaChoque > 0.0f) esperaChoque -= dt;
    juego.tiempoDesdeMeta += dt;
    AnimationDRS(dt);            // avanza los keyframes del DRS (KeyFramesDRS.cpp)
    j.aperturaDRS = drsAngulo;

    // ----- Arranque -----
    if (juego.estado == ESPERANDO_ARRANQUE)
    {
        if (!c.acelerar) return;
        juego.estado = CORRIENDO;
        Mensaje("YA!", 1.0f);
        AudioReproducir(SONIDO_CHECKPOINT);
    }
    juego.tiempoVuelta += dt;

    // ----- DRS -----
    juego.drsDisponible = EnZonaDRS(j.dist);
    if (juego.drsAbierto && (!juego.drsDisponible || c.frenar))   // se cierra solo
    {
        juego.drsAbierto = false;
        CargarKeyFramesCerrarDRS();
    }


    // ----- Acelerar / frenar -----
    float velMax = juego.drsAbierto ? VEL_MAX_DRS : VEL_MAX;
    if (c.acelerar)     j.vel += ACELERACION * dt;
    else if (c.frenar)  j.vel -= FRENADO * dt;
    else                j.vel -= FRICCION * dt;
    if (j.vel > velMax) j.vel = glm::max(velMax, j.vel - FRENADO * dt);   // al cerrar el DRS baja suave
    j.vel = glm::max(j.vel, 0.0f);

    // ----- Lateral y colision con el muro -----
    float mover = 0.0f;
    if (c.izquierda) mover -= 1.0f;
    if (c.derecha)   mover += 1.0f;
    float antes = j.lateral;
    j.lateral = glm::clamp(j.lateral + mover * VEL_LATERAL * dt, -LATERAL_MAX, LATERAL_MAX);
    bool tocaMuro = std::fabs(j.lateral) >= LATERAL_MAX && std::fabs(antes) < LATERAL_MAX;
    if (tocaMuro && j.vel > 8.0f)
    {
        j.vel *= 0.65f;
        juego.choquesMuro++;
        Mensaje("CONTACTO CON EL MURO", 1.5f);
        AudioReproducir(SONIDO_CHOQUE);
    }

    MoverCarro(j, dt, mover * -8.0f);   // al moverse a la derecha, las llantas giran a la derecha

    ActualizarRivales(dt);
    ColisionesRivales();

    // ----- Checkpoints y meta -----
    while (juego.sigCheckpoint <= NUM_CHECKPOINTS && j.dist >= distCheckpoint[juego.sigCheckpoint - 1])
    {
        if (juego.sigCheckpoint == NUM_CHECKPOINTS)
        {
            CruzarMeta();
            break;
        }
        juego.tiempoParcial = juego.tiempoVuelta;
        Mensaje("CHECKPOINT " + std::to_string(juego.sigCheckpoint) + "/" + std::to_string(NUM_CHECKPOINTS - 1) +
                "  " + FormatoTiempo(juego.tiempoParcial), 2.0f);
        AudioReproducir(SONIDO_CHECKPOINT);
        juego.sigCheckpoint++;
    }
}


glm::mat4 MatrizCarroMundo(const Carro& c)
{
    glm::mat4 m = glm::translate(glm::mat4(1.0f), SplinePuntoPista(c.dist, c.lateral));
    return glm::rotate(m, glm::radians(SplineAnguloY(c.dist)), glm::vec3(0.0f, 1.0f, 0.0f));
}


void JuegoDibujarCarros(Shader& shader, bool dibujarCabezaJugador)
{
    const Carro& j = juego.jugador;
    DibujarCarro(shader, MatrizCarroMundo(j), j.giroRuedas, j.direccion, j.aperturaDRS,
                 glm::vec3(1.0f), dibujarCabezaJugador);

    for (const Rival& r : juego.rivales)
        if (r.activo)
            DibujarCarro(shader, MatrizCarroMundo(r.carro), r.carro.giroRuedas, r.carro.direccion,
                         0.0f, r.color, true);
}


int RivalesActivos()
{
    int n = 0;
    for (const Rival& r : juego.rivales) if (r.activo) n++;
    return n;
}

float VelocidadKmh()
{
    return juego.jugador.vel * 3.6f;
}
