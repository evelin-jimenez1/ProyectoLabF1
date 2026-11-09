// =============================================================
//  Comisario.cpp - comisario de meta con bandera a cuadros
// =============================================================
#include "Comisario.h"
#include "Spline.h"
#include "Primitivas.h"
#include "Estadio.h"
#include "Juego.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------- Texturas de color ----------
static GLuint texPiel, texChaleco, texPantalon, texGorra, texAsta;
static GLuint texBanderaCuadros, texBanderaVerde, texBanderaAmarilla;

// ============================ KEYFRAMES ============================
// Igual que KeyFramesDRS.cpp: cada FRAME guarda los angulos y cuanto
// cambian por paso; la animacion los va sumando hasta el siguiente keyframe.
#define MAX_FRAMES 16
static const float PASOS_POR_SEGUNDO = 60.0f;

typedef struct _frame {
    float brazo, antebrazo, bandera;          // angulos (grados) en este keyframe
    float brazoInc, antebrazoInc, banderaInc; // cuanto cambian en cada paso
    int steps;                                // pasos que dura el tramo hacia el siguiente keyframe
} FRAME;

static FRAME KeyFrame[MAX_FRAMES];
static int FrameIndex = 0;
static bool play = false;
static int playIndex = 0;
static float i_curr_steps = 0.0f;

// Angulos actuales (los usa el dibujo)
static float anguloBrazo = 25.0f;      // brazo levantado hacia el lado
static float anguloAntebrazo = 150.0f; // codo (en reposo sostiene la bandera en vertical)
static float anguloBandera = 0.0f;     // giro de la tela alrededor del asta

static void saveFrame(float brazo, float ante, float bandera, int pasos)
{
    FRAME& f = KeyFrame[FrameIndex++];
    f.brazo = brazo; f.antebrazo = ante; f.bandera = bandera;
    f.brazoInc = f.antebrazoInc = f.banderaInc = 0.0f;
    f.steps = pasos;
}

static void resetElements()
{
    anguloBrazo = KeyFrame[0].brazo;
    anguloAntebrazo = KeyFrame[0].antebrazo;
    anguloBandera = KeyFrame[0].bandera;
}

// Incrementos lineales entre el keyframe actual y el siguiente
static void interpolation()
{
    FRAME& a = KeyFrame[playIndex];
    FRAME& b = KeyFrame[playIndex + 1];
    a.brazoInc = (b.brazo - a.brazo) / a.steps;
    a.antebrazoInc = (b.antebrazo - a.antebrazo) / a.steps;
    a.banderaInc = (b.bandera - a.bandera) / a.steps;
}

void ComisarioOndear()
{
    FrameIndex = 0;
    saveFrame(25.0f, 150.0f, 0.0f, 18);      // reposo (bandera en vertical) -> empieza a subir
    saveFrame(150.0f, 0.0f, 0.0f, 14);       // brazo arriba
    for (int i = 0; i < 4; i++)              // 4 ondeos
    {
        saveFrame(150.0f, -35.0f, -35.0f, 11);
        saveFrame(150.0f, 35.0f, 35.0f, 11);
    }
    saveFrame(150.0f, 0.0f, 0.0f, 14);       // se detiene
    saveFrame(25.0f, 150.0f, 0.0f, 1);       // regresa al reposo

    playIndex = 0;
    i_curr_steps = 0.0f;
    resetElements();
    interpolation();
    play = true;
}

static void Animation(float dt)
{
    if (!play) return;

    FRAME& f = KeyFrame[playIndex];
    float avance = dt * PASOS_POR_SEGUNDO;
    float resta = f.steps - i_curr_steps;
    if (avance > resta) avance = resta;            // no pasarse del keyframe
    anguloBrazo += f.brazoInc * avance;
    anguloAntebrazo += f.antebrazoInc * avance;
    anguloBandera += f.banderaInc * avance;
    i_curr_steps += avance;

    if (i_curr_steps >= f.steps - 0.0001f)         // termino el tramo?
    {
        playIndex++;
        if (playIndex > FrameIndex - 2)            // termino toda la animacion?
        {
            anguloBrazo = KeyFrame[FrameIndex - 1].brazo;
            anguloAntebrazo = KeyFrame[FrameIndex - 1].antebrazo;
            anguloBandera = KeyFrame[FrameIndex - 1].bandera;
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

// ============================ DETECCION DE META ============================
void ComisarioActualizar(float dt)
{
    Animation(dt);

    // Cuando el jugador esta por llegar a la meta (30 m antes) empieza a ondear
    static bool cerca = false;
    bool ahoraCerca = juego.jugador.dist > SplineLongitud() - 30.0f;
    if (ahoraCerca && !cerca) ComisarioOndear();
    cerca = ahoraCerca;
}

// ============================ DIBUJO ============================
void ComisarioInicializar()
{
    texPiel = CrearTexturaColor(0.85f, 0.62f, 0.48f);
    texChaleco = CrearTexturaColor(1.0f, 0.45f, 0.05f);     // chaleco naranja de comisario
    texPantalon = CrearTexturaColor(0.10f, 0.11f, 0.18f);
    texGorra = CrearTexturaColor(0.96f, 0.96f, 0.96f);
    texAsta = CrearTexturaColor(0.55f, 0.55f, 0.58f);
    texBanderaCuadros = CrearTexturaDamero(6, 4);
    texBanderaVerde = CrearTexturaColor(0.05f, 0.65f, 0.20f);
    texBanderaAmarilla = CrearTexturaColor(1.0f, 0.85f, 0.05f);
}

void DibujarPersona(Shader& shader, const glm::mat4& base, float brazo, float antebrazo,
                    float bandera, GLuint texChaleco, GLuint texBandera, float brazoIzq, GLuint texGorraColor)
{
    GLuint gorra = texGorraColor ? texGorraColor : texGorra;
    glm::mat4 m;

    // ---------- Torso (nodo padre de todo lo de arriba) ----------
    glm::mat4 torso = glm::translate(base, glm::vec3(0.0f, 1.25f, 0.0f));
    DibujarMalla(shader, mallaCubo, glm::scale(torso, glm::vec3(0.52f, 0.72f, 0.30f)), texChaleco);

    // Piernas
    for (int lado = -1; lado <= 1; lado += 2)
    {
        m = glm::translate(base, glm::vec3(lado * 0.13f, 0.45f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.20f, 0.90f, 0.22f)), texPantalon);
    }
    // Cabeza y gorra (hijas del torso)
    m = glm::translate(torso, glm::vec3(0.0f, 0.62f, 0.0f));
    DibujarMalla(shader, mallaEsfera, glm::scale(m, glm::vec3(0.30f)), texPiel);
    m = glm::translate(torso, glm::vec3(0.0f, 0.80f, 0.0f));
    DibujarMalla(shader, mallaCilindro, glm::scale(m, glm::vec3(0.32f, 0.10f, 0.32f)), gorra);
    m = glm::translate(torso, glm::vec3(0.0f, 0.78f, -0.20f));      // visera
    DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.26f, 0.03f, 0.18f)), gorra);

    // Brazo izquierdo (hijo del torso): pegado al cuerpo o levantado (festejo)
    glm::mat4 hombroIzq = glm::translate(torso, glm::vec3(-0.34f, 0.32f, 0.0f));
    hombroIzq = glm::rotate(hombroIzq, glm::radians(-brazoIzq), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::translate(hombroIzq, glm::vec3(0.0f, -0.31f, 0.0f));
    DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.14f, 0.62f, 0.14f)), texChaleco);

    // ---------- Brazo derecho (hijo del torso): pivote en el hombro ----------
    glm::mat4 hombro = glm::translate(torso, glm::vec3(0.34f, 0.32f, 0.0f));
    hombro = glm::rotate(hombro, glm::radians(brazo), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::translate(hombro, glm::vec3(0.0f, -0.25f, 0.0f));
    DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.14f, 0.52f, 0.14f)), texChaleco);

    // ---------- Antebrazo (hijo del brazo): pivote en el codo ----------
    glm::mat4 codo = glm::translate(hombro, glm::vec3(0.0f, -0.50f, 0.0f));
    codo = glm::rotate(codo, glm::radians(antebrazo), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::translate(codo, glm::vec3(0.0f, -0.22f, 0.0f));
    DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.12f, 0.46f, 0.12f)), texPiel);

    if (texBandera == 0) return;     // sin bandera (mecanicos, pilotos del podio)

    // ---------- Asta (hija del antebrazo): se sostiene en la mano ----------
    glm::mat4 mano = glm::translate(codo, glm::vec3(0.0f, -0.44f, 0.0f));
    m = glm::translate(mano, glm::vec3(0.0f, -0.65f, 0.0f));
    DibujarMalla(shader, mallaCilindro, glm::scale(m, glm::vec3(0.05f, 1.9f, 0.05f)), texAsta);

    // ---------- Bandera (hija del asta): gira alrededor del asta ----------
    glm::mat4 tela = glm::translate(mano, glm::vec3(0.0f, -1.15f, 0.0f));
    tela = glm::rotate(tela, glm::radians(bandera), glm::vec3(0.0f, 1.0f, 0.0f));
    tela = glm::translate(tela, glm::vec3(0.0f, 0.0f, 0.62f));
    DibujarMalla(shader, mallaCubo, glm::scale(tela, glm::vec3(0.03f, 0.80f, 1.20f)), texBandera);
}

// Posicion de un comisario junto a la pista
static glm::mat4 BaseComisario(float s, float lateral)
{
    glm::vec3 p = SplinePuntoPista(s, lateral);
    p.y = 0.0f;
    glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
    // Gira para mirar hacia la pista (a la derecha el comisario mira a la izquierda y viceversa)
    float giro = SplineAnguloY(s) + (lateral > 0.0f ? 90.0f : -90.0f);
    return glm::rotate(m, glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f));
}

void ComisarioDibujar(Shader& shader, float tiempo)
{
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);

    // Comisario de meta: bandera a cuadros animada por keyframes
    DibujarPersona(shader, BaseComisario(0.0f, 6.5f),
                   anguloBrazo, anguloAntebrazo, anguloBandera, texChaleco, texBanderaCuadros);

    // Comisarios de ambientacion (bandera verde y amarilla), solo se balancean un poco
    float onda = 20.0f * std::sin(tiempo * 3.0f);
    DibujarPersona(shader, BaseComisario(60.0f, 6.5f), 140.0f, onda, onda, texChaleco, texBanderaVerde);
    DibujarPersona(shader, BaseComisario(120.0f, -6.5f), 140.0f, -onda, -onda, texChaleco, texBanderaAmarilla);
}
