// =============================================================
//  Boxes.cpp - pit stop
// =============================================================
#include "Boxes.h"
#include "Spline.h"
#include "Primitivas.h"
#include "Texturas.h"
#include "CargaModelos.h"
#include "Comisario.h"
#include "KeyFramesDRS.h"
#include "Audio.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------- Geometria del carril (metros sobre la spline desde la meta) ----------
static const float S_ENTRA = 15.0f;         // aqui empieza a salirse de la pista
static const float S_RAMPA_FIN = 40.0f;     // ya esta dentro del carril
static const float S_BOX = 75.0f;           // lugar donde se detiene (frente a la zona de neumaticos)
static const float S_SALE_INI = 108.0f;     // empieza a regresar a la pista
static const float S_SALE_FIN = 135.0f;     // ya esta en la pista
static const float LATERAL_PIT = 7.5f;      // centro del carril (a la derecha de la pista)
static const float ANCHO_CARRIL = 5.0f;
static const float VEL_LIMITE = 22.2f;      // 80 km/h

enum EstadoPit { PIT_NORMAL, PIT_ENTRANDO, PIT_PARADO, PIT_SALIENDO };
static EstadoPit estado = PIT_NORMAL;
static bool pedido = false;
static float sPrev = 0.0f;
static float tParada = 0.0f;
static float tiempoEnBoxes = 0.0f;

// ---------- Texturas ----------
static GLuint texAsfaltoPit, texLineaAmarilla, texLineaBlanca, texEdificio, texPuerta[4];
static GLuint texOverol, texLlantaPit;

// ============================ KEYFRAMES DEL CAMBIO DE LLANTAS ============================
// Igual que KeyFramesDRS.cpp, pero con 3 canales:
//   aprox = que tan cerca estan los mecanicos del carro (0 lejos, 1 junto a la llanta)
//   brazo = angulo del brazo con la pistola de tuercas
//   viejo = llanta vieja saliendo (0 en el carro, 1 en el piso)
//   nuevo = llanta nueva entrando (0 afuera, 1 puesta)
#define MAX_FRAMES 12
static const float PASOS_POR_SEGUNDO = 60.0f;

typedef struct _frame {
    float aprox, brazo, viejo, nuevo;
    float aproxInc, brazoInc, viejoInc, nuevoInc;
    int steps;
} FRAME;

static FRAME KeyFrame[MAX_FRAMES];
static int FrameIndex = 0;
static bool play = false;
static int playIndex = 0;
static float i_curr_steps = 0.0f;
static float aprox = 0, brazo = 10, viejo = 0, nuevo = 0;    // valores actuales

static void saveFrame(float a, float b, float v, float n, int pasos)
{
    FRAME& f = KeyFrame[FrameIndex++];
    f.aprox = a; f.brazo = b; f.viejo = v; f.nuevo = n;
    f.aproxInc = f.brazoInc = f.viejoInc = f.nuevoInc = 0.0f;
    f.steps = pasos;
}

static void resetElements()
{
    aprox = KeyFrame[0].aprox; brazo = KeyFrame[0].brazo;
    viejo = KeyFrame[0].viejo; nuevo = KeyFrame[0].nuevo;
}

static void interpolation()
{
    FRAME& a = KeyFrame[playIndex];
    FRAME& b = KeyFrame[playIndex + 1];
    a.aproxInc = (b.aprox - a.aprox) / a.steps;
    a.brazoInc = (b.brazo - a.brazo) / a.steps;
    a.viejoInc = (b.viejo - a.viejo) / a.steps;
    a.nuevoInc = (b.nuevo - a.nuevo) / a.steps;
}

static void CargarKeyFramesParada()
{
    FrameIndex = 0;
    saveFrame(0, 10, 0, 0, 30);     // los mecanicos caminan hacia el carro
    saveFrame(1, 10, 0, 0, 16);     // llegan a las llantas
    saveFrame(1, 80, 0, 0, 18);     // pistola: aflojan las tuercas
    saveFrame(1, 40, 1, 0, 24);     // sale la llanta vieja
    saveFrame(1, 40, 1, 1, 24);     // entra la llanta nueva
    saveFrame(1, 80, 1, 1, 18);     // pistola: aprietan
    saveFrame(1, 10, 1, 1, 16);     // levantan la mano (listo)
    saveFrame(0, 10, 1, 1, 30);     // se alejan
    saveFrame(0, 10, 1, 1, 1);

    playIndex = 0;
    i_curr_steps = 0.0f;
    resetElements();
    interpolation();
    play = true;
}

static float DuracionParada()
{
    float t = 0.0f;
    for (int i = 0; i < FrameIndex - 1; i++) t += KeyFrame[i].steps;
    return t / PASOS_POR_SEGUNDO;
}

static void AnimationParada(float dt)
{
    if (!play) return;

    FRAME& f = KeyFrame[playIndex];
    float avance = dt * PASOS_POR_SEGUNDO;
    float resta = f.steps - i_curr_steps;
    if (avance > resta) avance = resta;
    aprox += f.aproxInc * avance;
    brazo += f.brazoInc * avance;
    viejo += f.viejoInc * avance;
    nuevo += f.nuevoInc * avance;
    i_curr_steps += avance;

    if (i_curr_steps >= f.steps - 0.0001f)
    {
        playIndex++;
        if (playIndex > FrameIndex - 2)
        {
            const FRAME& u = KeyFrame[FrameIndex - 1];
            aprox = u.aprox; brazo = u.brazo; viejo = u.viejo; nuevo = u.nuevo;
            playIndex = 0;
            play = false;
        }
        else
        {
            i_curr_steps = 0.0f;
            interpolation();
        }
    }
}

// ============================ LOGICA ============================
static float Suave(float a, float b, float x)
{
    float t = glm::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static void Mensaje(const std::string& texto, float segundos)
{
    juego.mensaje = texto;
    juego.tiempoMensaje = segundos;
}

void BoxesInicializar()
{
    texAsfaltoPit = CrearTexturaColor(0.24f, 0.24f, 0.26f);
    texLineaAmarilla = CrearTexturaColor(1.0f, 0.82f, 0.05f);
    texLineaBlanca = CrearTexturaColor(0.95f, 0.95f, 0.95f);
    texEdificio = CrearTexturaColor(0.22f, 0.22f, 0.27f);
    texPuerta[0] = CrearTexturaColor(0.85f, 0.10f, 0.12f);
    texPuerta[1] = CrearTexturaColor(0.10f, 0.35f, 0.80f);
    texPuerta[2] = CrearTexturaColor(1.00f, 0.55f, 0.05f);
    texPuerta[3] = CrearTexturaColor(0.10f, 0.60f, 0.30f);
    texOverol = CrearTexturaColor(0.88f, 0.12f, 0.12f);
    texLlantaPit = CargarTextura("images/llanta.png");
}

void BoxesPedir()
{
    if (estado != PIT_NORMAL) return;
    pedido = !pedido;
    Mensaje(pedido ? "BOX BOX! ENTRAS A BOXES EN LA META" : "PARADA CANCELADA", 2.5f);
}

bool BoxesPedido() { return pedido; }

void BoxesReiniciar()
{
    estado = PIT_NORMAL;
    pedido = false;
    juegoEnPits = false;
    play = false;
    aprox = 0; viejo = 0; nuevo = 0;
    sPrev = 0.0f;
}

// Velocidad a la que debe ir en cada tramo del carril
static float VelocidadObjetivo(float s)
{
    if (estado == PIT_ENTRANDO)
        return glm::min(VEL_LIMITE, 4.0f + 1.0f * (S_BOX - s));   // frena para llegar al box
    return VEL_LIMITE;
}

void BoxesFiltrarControles(Controles& c)
{
    if (estado == PIT_NORMAL) return;

    // En boxes el carro se maneja solo (acelera o frena para respetar el limite)
    const Carro& j = juego.jugador;
    c.izquierda = c.derecha = false;
    if (estado == PIT_PARADO)
    {
        c.acelerar = c.frenar = false;
        return;
    }
    float objetivo = VelocidadObjetivo(j.dist);
    c.acelerar = j.vel < objetivo;
    c.frenar = j.vel > objetivo + 1.0f;
}

void BoxesActualizar(float dt)
{
    Carro& j = juego.jugador;
    float s = j.dist;

    if (juego.estado != CORRIENDO) { sPrev = s; return; }

    switch (estado)
    {
    case PIT_NORMAL:
        // Entra solo si lo pidio y acaba de pasar la entrada del carril
        if (pedido && sPrev < S_ENTRA && s >= S_ENTRA)
        {
            estado = PIT_ENTRANDO;
            juegoEnPits = true;
            pedido = false;
            if (juego.drsAbierto) { juego.drsAbierto = false; CargarKeyFramesCerrarDRS(); }
            Mensaje("ENTRANDO A BOXES - LIMITE 80 KM/H", 2.5f);
        }
        break;

    case PIT_ENTRANDO:
        j.lateral = LATERAL_PIT * Suave(S_ENTRA, S_RAMPA_FIN, s);
        j.vel = glm::min(j.vel, VelocidadObjetivo(s) + 1.0f);
        if (s >= S_BOX - 0.25f)
        {
            j.dist = S_BOX;
            j.vel = 0.0f;
            estado = PIT_PARADO;
            tParada = 0.0f;
            tiempoEnBoxes = 0.0f;
            CargarKeyFramesParada();
            AudioReproducir(SONIDO_CHECKPOINT);
            Mensaje("CAMBIANDO LLANTAS...", 99.0f);
        }
        break;

    case PIT_PARADO:
        j.lateral = LATERAL_PIT;
        j.vel = 0.0f;
        tParada += dt;
        tiempoEnBoxes += dt;
        AnimationParada(dt);
        if (tParada >= DuracionParada() + 0.3f)
        {
            estado = PIT_SALIENDO;
            AudioReproducir(SONIDO_DRS);
            Mensaje("LISTO! SALES A PISTA", 2.0f);
        }
        break;

    case PIT_SALIENDO:
        tiempoEnBoxes += dt;
        j.lateral = LATERAL_PIT * (1.0f - Suave(S_SALE_INI, S_SALE_FIN, s));
        j.vel = glm::min(j.vel, VEL_LIMITE + 1.0f);
        if (s >= S_SALE_FIN)
        {
            estado = PIT_NORMAL;
            juegoEnPits = false;
            j.lateral = 0.0f;
            Mensaje("PARADA EN BOXES: " + FormatoTiempo(tiempoEnBoxes), 3.0f);
        }
        break;
    }
    sPrev = s;
}

// ============================ DIBUJO ============================
// Posicion y giro en el piso de un punto de la pista (como en Estadio.cpp)
static glm::mat4 MatrizEnPista(float s, float lateral, float giroExtra)
{
    glm::vec3 p = SplinePuntoPista(s, lateral);
    p.y = 0.0f;
    glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
    return glm::rotate(m, glm::radians(SplineAnguloY(s) + giroExtra), glm::vec3(0.0f, 1.0f, 0.0f));
}

// Losa plana entre dos puntos (s1, lat1) -> (s2, lat2), de ancho 'ancho' y tope a ALTURA_PISTA
static void DibujarLosa(Shader& shader, float s1, float lat1, float s2, float lat2, float ancho, GLuint tex, float extraAlto)
{
    float sm = (s1 + s2) * 0.5f, lm = (lat1 + lat2) * 0.5f;
    float largo = std::sqrt((s2 - s1) * (s2 - s1) + (lat2 - lat1) * (lat2 - lat1));
    // en el marco local: +X = derecha, -Z = avance
    float giro = glm::degrees(std::atan2(lat2 - lat1, -(s2 - s1)));
    glm::mat4 m = MatrizEnPista(sm, lm, giro);
    float alto = ALTURA_PISTA + extraAlto;
    m = glm::translate(m, glm::vec3(0.0f, alto * 0.5f, 0.0f));
    DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(ancho, alto, largo)), tex);
}

// Llanta (cilindro con eje en X). 'inclinacion' 0 = de pie, 90 = acostada
static void DibujarLlanta(Shader& shader, const glm::mat4& base, glm::vec3 pos, float acostada)
{
    glm::mat4 m = glm::translate(base, pos);
    m = glm::rotate(m, glm::radians(90.0f * (1.0f - acostada)), glm::vec3(0.0f, 0.0f, 1.0f));
    DibujarMalla(shader, mallaCilindro, glm::scale(m, glm::vec3(0.70f, 0.38f, 0.70f)), texLlantaPit);
}

void BoxesDibujar(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);

    // ---------- Carril de boxes: rampa de entrada, carril y rampa de salida ----------
    DibujarLosa(shader, S_ENTRA, 3.2f, S_RAMPA_FIN, LATERAL_PIT, ANCHO_CARRIL, texAsfaltoPit, 0.005f);
    DibujarLosa(shader, S_RAMPA_FIN, LATERAL_PIT, S_SALE_INI, LATERAL_PIT, ANCHO_CARRIL, texAsfaltoPit, 0.005f);
    DibujarLosa(shader, S_SALE_INI, LATERAL_PIT, S_SALE_FIN, 3.2f, ANCHO_CARRIL, texAsfaltoPit, 0.005f);

    // Lineas del carril: amarilla del lado de la pista, blanca del lado de los garajes
    for (int lado = -1; lado <= 1; lado += 2)
    {
        glm::mat4 m = MatrizEnPista((S_RAMPA_FIN + S_SALE_INI) * 0.5f, LATERAL_PIT + lado * (ANCHO_CARRIL * 0.5f - 0.1f), 0.0f);
        m = glm::translate(m, glm::vec3(0.0f, ALTURA_PISTA + 0.02f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.14f, 0.02f, S_SALE_INI - S_RAMPA_FIN)),
                     lado < 0 ? texLineaAmarilla : texLineaBlanca);
    }
    // Marca del box donde se detiene: rectangulo blanco
    {
        glm::mat4 b = MatrizEnPista(S_BOX, LATERAL_PIT, 0.0f);
        b = glm::translate(b, glm::vec3(0.0f, ALTURA_PISTA + 0.025f, 0.0f));
        for (int k = -1; k <= 1; k += 2)
        {
            DibujarMalla(shader, mallaCubo, glm::scale(glm::translate(b, glm::vec3(k * 2.1f, 0.0f, 0.0f)), glm::vec3(0.12f, 0.02f, 6.4f)), texLineaBlanca);
            DibujarMalla(shader, mallaCubo, glm::scale(glm::translate(b, glm::vec3(0.0f, 0.0f, k * 3.2f)), glm::vec3(4.3f, 0.02f, 0.12f)), texLineaBlanca);
        }
    }

    // ---------- Edificio de boxes (detras de la zona de neumaticos) ----------
    {
        glm::mat4 m = MatrizEnPista(S_BOX, 26.0f, 0.0f);
        m = glm::translate(m, glm::vec3(0.0f, 2.8f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(8.0f, 5.6f, 112.0f)), texEdificio);
        // Puertas de los garajes (una por equipo) mirando hacia la pista
        for (int i = 0; i < 9; i++)
        {
            float s = 25.0f + i * 12.5f;
            glm::mat4 p = MatrizEnPista(s, 21.9f, 0.0f);
            p = glm::translate(p, glm::vec3(0.0f, 1.8f, 0.0f));
            DibujarMalla(shader, mallaCubo, glm::scale(p, glm::vec3(0.2f, 3.4f, 9.0f)), texPuerta[i % 4]);
        }
    }

    // ---------- Mecanicos y llantas (solo durante la parada) ----------
    if (estado != PIT_PARADO) return;

    glm::mat4 carro = MatrizCarroMundo(juego.jugador);
    // Llantas en el marco del carro: x = +-0.72, z = -1.74 (delanteras) / +1.74 (traseras)
    for (int i = 0; i < 4; i++)
    {
        float lado = (i % 2 == 0) ? -1.0f : 1.0f;       // izquierda / derecha
        float z = (i < 2) ? -1.743f : 1.743f;           // delante / atras
        glm::vec3 rueda(lado * 0.72f, RADIO_LLANTA, z);

        // Mecanico: afuera de la llanta, mirando al carro; se acerca con 'aprox'
        float dist = 1.45f + 3.2f * (1.0f - aprox);
        glm::mat4 base = glm::translate(carro, glm::vec3(lado * dist, 0.0f, z));
        base = glm::rotate(base, glm::radians(-lado * 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));   // mira hacia el carro
        // Los de enfrente usan el brazo derecho; los de atras el izquierdo (para que apunten a la llanta)
        if (i < 2) DibujarPersona(shader, base, brazo, 0.0f, 0.0f, texOverol, 0, 0.0f);
        else       DibujarPersona(shader, base, 0.0f, 0.0f, 0.0f, texOverol, 0, brazo);

        // Llanta vieja: sale de la llanta y queda acostada en el piso
        if (viejo > 0.01f)
        {
            glm::vec3 p = rueda + glm::vec3(lado * 1.6f * viejo, (0.19f - RADIO_LLANTA) * viejo + 0.15f * std::sin(3.14159f * viejo), 0.0f);
            DibujarLlanta(shader, carro, p, viejo);
        }
        // Llanta nueva: viene de afuera y se coloca en la llanta
        if (nuevo < 0.99f)
        {
            glm::vec3 p = rueda + glm::vec3(lado * 1.6f * (1.0f - nuevo), 0.0f, 0.55f * (1.0f - nuevo) * (i < 2 ? 1.0f : -1.0f));
            DibujarLlanta(shader, carro, p, 0.0f);
        }
    }
}
