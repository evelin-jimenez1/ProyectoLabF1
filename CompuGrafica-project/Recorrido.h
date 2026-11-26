#pragma once
// =============================================================
//  Recorrido.h - camara automatica para grabar el video (tecla V)
//
//  Pasa por todas las zonas del proyecto con tomas de camara fijas
//  (cada toma dura unos segundos) y cambia el ambiente dia / atardecer /
//  noche solo. No hay que agregar este archivo al proyecto: se incluye
//  desde main.cpp.
//
//  V = iniciar el recorrido (o cancelarlo en cualquier momento)
// =============================================================
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Spline.h"
#include "Ambiente.h"
#include "Juego.h"

extern bool lucesActivas;     // definida en main.cpp (F prende / apaga los postes)

// Una toma: la camara va de posA a posB mirando de tgtA a tgtB en 'dur' segundos.
// Si 'orbita' > 0, la camara da vueltas alrededor de tgtA (radio 'orbita', altura posA.y, de angA a angB grados).
// 'ambiente' > 0: al empezar la toma cambia a ese ambiente (1 atardecer, 2 noche, 3 dia)
struct TomaRecorrido
{
    float dur;
    glm::vec3 posA, tgtA, posB, tgtB;
    float orbita, angA, angB;
    int ambiente;
};

static std::vector<TomaRecorrido> tomasRec;
static bool  recActivo = false;
static int   recToma = 0;
static float recT = 0.0f;
static bool  recBucle = false;   // true: la toma se repite hasta volver a pulsar la tecla (camara de la grada)

inline bool RecorridoActivo() { return recActivo; }

static glm::vec3 RecP(float s, float lateral, float altura)
{
    return SplinePuntoPista(s, lateral) + glm::vec3(0.0f, altura, 0.0f);
}

static void RecAmbiente(int modo)   // 1 atardecer, 2 noche, 3 dia
{
    int meta = (modo == 3) ? AMB_DIA : modo;
    for (int i = 0; i < 3 && (int)AmbienteModo() != meta; i++) AmbienteAlternar();
    lucesActivas = (AmbienteModo() != AMB_DIA);
}

// Centro de las 6 personas animadas en el mundo (grada nueva)
static const glm::vec3 GRADA_6(-212.0f, 3.8f, -51.0f);

static void RecConstruir()
{
    tomasRec.clear();
    const float L = SplineLongitud();
    auto T = [&](float d, glm::vec3 pa, glm::vec3 ta, glm::vec3 pb, glm::vec3 tb, int amb = 0)
        { tomasRec.push_back({ d, pa, ta, pb, tb, 0.0f, 0.0f, 0.0f, amb }); };

    // 1. Meta y recta principal (dia)
    T(10, RecP(-45, 0, 9),      RecP(35, 0, 1),   RecP(8, 0, 5),    RecP(70, 0, 1), 3);
    // 2. Boxes
    T(12, RecP(28, 13, 4),      RecP(62, 7.5f, 1.5f), RecP(95, 13, 4), RecP(105, 7.5f, 1.0f));
    // 3. Grada llena de munecos cuadrados
    T(14, glm::vec3(-188, 6.0f, -90), glm::vec3(-207, 3.0f, -84), glm::vec3(-188, 6.0f, -66), glm::vec3(-207, 3.0f, -68));
    // 4. Desde el fondo de la grada hacia la pista
    T(14, glm::vec3(-211.2f, 4.7f, -92), glm::vec3(-168, 1.0f, -78), glm::vec3(-211.2f, 4.7f, -70), glm::vec3(-168, 1.0f, -90));
    // 5. Fermin
    T(12, glm::vec3(-176, 7.0f, -118), glm::vec3(-196, 4.5f, -137), glm::vec3(-204, 7.0f, -118), glm::vec3(-196, 4.5f, -137));
    // 6. Marcador
    T(10, glm::vec3(-205, 10.5f, -91), glm::vec3(-191, 10.2f, -94.4f), glm::vec3(-205, 11.5f, -98), glm::vec3(-191, 10.2f, -94.4f));
    // 7. Altar de muertos (de frente, desde el lado de la pista, camara baja que sube un poco)
    T(12, glm::vec3(-184, 2.5f, -97), glm::vec3(-160, 4.5f, -105), glm::vec3(-179, 5.0f, -112), glm::vec3(-160, 5.5f, -105));
    // 8. Bandera de viento (camara lejos, sube de la base a la tela)
    T(12, glm::vec3(-136, 8.0f, -57), glm::vec3(-161, 12.0f, -82), glm::vec3(-141, 17.0f, -63), glm::vec3(-162.8f, 18.0f, -80));
    // 9. Alebrije decorativo (s = 1300, izquierda): camara desde la pista, sin postes ni cables de por medio
    T(12, RecP(1286, 3, 3.0f),  RecP(1300, -11.5f, 2.4f), RecP(1293, 3.5f, 3.6f), RecP(1300, -11.5f, 2.6f));
    // 10. Alebrijes y papel picado a lo largo de la pista
    T(14, RecP(240, 0, 4.5f),   RecP(300, -11.5f, 2.5f), RecP(300, 0, 4.5f), RecP(335, -11.5f, 2.5f));
    // 11. Grada de la recta principal vista desde atras (fondo de la grada mirando a la pista)
    T(14, RecP(-30, -26.2f, 4.7f), RecP(-12, 15, 1.0f), RecP(40, -26.2f, 4.7f), RecP(22, 15, 1.0f));
    // 12. Atardecer
    T(10, RecP(520, 0, 3.0f),   RecP(560, 0, 14.0f), RecP(560, 0, 3.0f),  RecP(620, 0, 12.0f), 1);
    // 13. Noche (postes prendidos)
    T(14, RecP(L - 60, 0, 5),   RecP(L - 20, 0, 3),  RecP(30, 0, 5),      RecP(80, 0, 3), 2);
    // 14. Dia otra vez
    T(8,  RecP(-30, 8, 6),      RecP(20, 0, 1),      RecP(10, 8, 7),      RecP(60, 0, 1), 3);
    // 15 y 16. Camaras de TV junto a la pista: siguen al carro (que arranca solo). orbita = -1 marca "sigue al carro"
    tomasRec.push_back({ 6.0f, glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec3(0), -1.0f, 0.0f, 0.0f, 3 });
    tomasRec.push_back({ 6.0f, glm::vec3(0), glm::vec3(0), glm::vec3(0), glm::vec3(0), -1.0f, 0.0f, 0.0f, 0 });
}

// El carro arranca solo durante las tomas de seguimiento (main.cpp: if (RecorridoAcelerar()) controles.acelerar = true;)
inline bool RecorridoAcelerar()
{
    return recActivo && recToma < (int)tomasRec.size() && tomasRec[recToma].orbita < 0.0f;
}

static bool recSigueInicio = true;    // true = falta colocar la camara de la toma de seguimiento actual
static glm::vec3 recCamFija(0.0f);

inline void RecorridoAlternar()
{
    if (recActivo) { recActivo = false; RecAmbiente(3); return; }
    RecConstruir();
    recBucle = false;
    recToma = 0; recT = 0.0f; recActivo = true; recSigueInicio = true;
    RecAmbiente(tomasRec[0].ambiente > 0 ? tomasRec[0].ambiente : 3);
}

// Devuelve la matriz de vista del recorrido y la posicion de la camara
inline glm::mat4 RecorridoVista(float dt, glm::vec3& camPos)
{
    recT += dt;
    while (recToma < (int)tomasRec.size() && recT >= tomasRec[recToma].dur)
    {
        recT -= tomasRec[recToma].dur;
        recToma++;
        if (recBucle && recToma >= (int)tomasRec.size()) recToma = 0;
        if (recToma < (int)tomasRec.size() && tomasRec[recToma].ambiente > 0) RecAmbiente(tomasRec[recToma].ambiente);
        recSigueInicio = true;
    }
    if (recToma >= (int)tomasRec.size())
    {
        recActivo = false; RecAmbiente(3);
        camPos = glm::vec3(0, 5, 0);
        return glm::lookAt(camPos, camPos + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    }
    const TomaRecorrido& t = tomasRec[recToma];
    if (t.orbita < 0.0f)   // camara de TV: queda fija junto a la pista, 45 m adelante del carro, y lo sigue con la mirada
    {
        if (recSigueInicio)
        {
            if (recToma == 0 || tomasRec[recToma - 1].orbita >= 0.0f) JuegoReiniciar();   // la primera vez: carro en la salida
            recCamFija = SplinePuntoPista(juego.jugador.dist + 45.0f, 15.0f) + glm::vec3(0.0f, 3.0f, 0.0f);
            recSigueInicio = false;
        }
        camPos = recCamFija;
        glm::vec3 car = SplinePuntoPista(juego.jugador.dist, juego.jugador.lateral) + glm::vec3(0.0f, 0.8f, 0.0f);
        return glm::lookAt(camPos, car, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    float u = glm::clamp(recT / t.dur, 0.0f, 1.0f);
    float e = u * u * (3.0f - 2.0f * u);            // entra y sale suave
    glm::vec3 tgt = glm::mix(t.tgtA, t.tgtB, e);
    if (t.orbita > 0.0f)
    {
        float ang = glm::radians(glm::mix(t.angA, t.angB, e));
        camPos = glm::vec3(t.tgtA.x + t.orbita * std::cos(ang), t.posA.y, t.tgtA.z + t.orbita * std::sin(ang));
    }
    else camPos = glm::mix(t.posA, t.posB, e);
    return glm::lookAt(camPos, tgt, glm::vec3(0.0f, 1.0f, 0.0f));
}

// J: camara dedicada a las 6 personas animadas de la grada nueva. Da vueltas despacio alrededor
// de ellas hasta que se vuelve a pulsar J (no cambia el ambiente).
inline void RecorridoGradaAlternar()
{
    if (recActivo) { recActivo = false; recBucle = false; return; }
    tomasRec.clear();
    tomasRec.push_back({ 22, glm::vec3(0, 5.2f, 0), GRADA_6, glm::vec3(0, 5.2f, 0), GRADA_6, 7.5f, -50.0f, 50.0f, 0 });
    tomasRec.push_back({ 22, glm::vec3(0, 5.2f, 0), GRADA_6, glm::vec3(0, 5.2f, 0), GRADA_6, 7.5f, 50.0f, -50.0f, 0 });
    recBucle = true; recToma = 0; recT = 0.0f; recActivo = true;
}
