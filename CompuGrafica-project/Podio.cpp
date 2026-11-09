// =============================================================
//  Podio.cpp - escena del podio
// =============================================================
#include "Podio.h"
#include "Juego.h"
#include "Spline.h"
#include "Primitivas.h"
#include "Comisario.h"
#include "HUD.h"
#include "Texturas.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

// Centro del podio: terreno libre dentro del circuito
static const glm::vec3 CENTRO(45.0f, 0.0f, -75.0f);

struct Piloto3 { std::string nombre; float tiempo; int textura; bool jugador; };
static Piloto3 orden[1 + NUM_RIVALES];     // ordenados del 1o al ultimo
static bool activo = false;
static float t0 = 0.0f;                    // juego.tiempoJuego cuando se abrio el podio (para animar la copa)

static GLuint texOro, texPlata, texBronce, texBlanco, texNegro;
static GLuint texCamisa[1 + NUM_RIVALES];  // 0 = jugador, 1.. = rivales
static GLuint texConfeti[6];
static GLuint texBanderaMexico;            // foto de la bandera con el escudo (images/mexicobandera.jpg)

bool PodioActivo() { return activo; }
void PodioSalir() { activo = false; }

void PodioInicializar()
{
    texOro = CrearTexturaColor(1.0f, 0.80f, 0.10f);
    texPlata = CrearTexturaColor(0.78f, 0.80f, 0.85f);
    texBronce = CrearTexturaColor(0.80f, 0.50f, 0.22f);
    texBlanco = CrearTexturaColor(0.95f, 0.95f, 0.95f);
    texNegro = CrearTexturaColor(0.10f, 0.10f, 0.12f);
    texCamisa[0] = CrearTexturaColor(1.0f, 0.85f, 0.10f);      // jugador: casco/camisa amarilla
    for (int i = 0; i < NUM_RIVALES; i++)
    {
        glm::vec3 c = juego.rivales[i].color;
        texCamisa[i + 1] = CrearTexturaColor(c.r, c.g, c.b);
    }
    const float col[6][3] = { {0.95f,0.10f,0.55f}, {1.0f,0.55f,0.05f}, {0.55f,0.20f,0.85f},
                              {0.10f,0.75f,0.80f}, {1.0f,0.85f,0.10f}, {0.20f,0.70f,0.25f} };
    for (int i = 0; i < 6; i++) texConfeti[i] = CrearTexturaColor(col[i][0], col[i][1], col[i][2]);
    texBanderaMexico = CargarTextura("images/mexicobandera.jpg");
}

void PodioActivar()
{
    float L = SplineLongitud();
    // Tiempo de cada quien: mejor vuelta del jugador (o 50 s si todavia no completa ninguna)
    // Tiempo de cada quien en el contrarreloj completo: el total real del jugador (o 50 s por vuelta
    // si abres el podio con P sin haber corrido) contra el de cada rival (vueltas x tiempo de vuelta).
    float tJugador = juego.tiempoTotal > 0.0f ? juego.tiempoTotal : 50.0f * VUELTAS_CARRERA;
    Piloto3 todos[1 + NUM_RIVALES];
    todos[0] = { "TU", tJugador, 0, true };
    const char* nombres[NUM_RIVALES] = { "ROSA MEXICANO", "TURQUESA", "CEMPASUCHIL" };
    for (int i = 0; i < NUM_RIVALES; i++)
        todos[i + 1] = { nombres[i], VUELTAS_CARRERA * L / juego.rivales[i].velObjetivo, i + 1, false };

    std::sort(todos, todos + 1 + NUM_RIVALES, [](const Piloto3& a, const Piloto3& b) { return a.tiempo < b.tiempo; });
    for (int i = 0; i < 1 + NUM_RIVALES; i++) orden[i] = todos[i];
    t0 = juego.tiempoJuego;
    activo = true;
}

glm::mat4 PodioVista(float t, glm::vec3& pos)
{
    float a = 0.8f * std::sin(t * 0.35f);                  // vaiven alrededor del frente del podio
    pos = CENTRO + glm::vec3(11.0f * std::sin(a), 2.8f + 0.6f * std::sin(t * 0.5f), 11.0f * std::cos(a));
    return glm::lookAt(pos, CENTRO + glm::vec3(0.0f, 2.3f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

static float Hash(float n) { float s = std::sin(n * 127.1f) * 43758.5453f; return s - std::floor(s); }

void PodioDibujar(Shader& shader, float t)
{
    if (!activo) return;
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), CENTRO);

    // ---------- Podio: 1o al centro, 2o a la izquierda, 3o a la derecha ----------
    const float x[3] = { 0.0f, -3.1f, 3.1f };
    const float h[3] = { 1.6f, 1.1f, 0.7f };
    GLuint texEscalon[3] = { texOro, texPlata, texBronce };
    for (int i = 0; i < 3; i++)
    {
        glm::mat4 m = glm::translate(base, glm::vec3(x[i], h[i] * 0.5f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(3.0f, h[i], 3.0f)), texEscalon[i]);
        // tapa blanca arriba del escalon
        m = glm::translate(base, glm::vec3(x[i], h[i] + 0.03f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(3.05f, 0.06f, 3.05f)), texBlanco);
    }

    // ---------- La copa: el 1o la levanta con las dos manos ----------
    // k va de 0 (la copa esta frente al 1o) a 1 (la tiene arriba de la cabeza)
    float tp = juego.tiempoJuego - t0;
    float k = glm::clamp((tp - 0.6f) / 1.6f, 0.0f, 1.0f);
    k = k * k * (3.0f - 2.0f * k);

    // ---------- Pilotos (festejan con los brazos arriba; el 1o levanta la copa) ----------
    for (int i = 0; i < 1 + NUM_RIVALES; i++)
    {
        glm::mat4 p;
        float brazo = 140.0f, brazoIzq = 140.0f;
        if (i < 3)
        {
            p = glm::translate(base, glm::vec3(x[i], h[i] + 0.06f, 0.0f));
            brazo = 140.0f + 20.0f * std::sin(t * 6.0f + i);
            brazoIzq = 140.0f + 20.0f * std::sin(t * 6.0f + i + 2.0f);
            if (i == 0)   // sube los brazos hasta la vertical para sostener la copa
            {
                brazo = brazoIzq = 25.0f + 147.0f * k + 3.0f * k * std::sin(t * 6.0f);
            }
        }
        else   // el 4o lugar mira desde un lado
        {
            p = glm::translate(base, glm::vec3(-7.0f, 0.0f, 1.5f));
            brazo = brazoIzq = 0.0f;
        }
        p = glm::rotate(p, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));   // mira hacia +Z (la camara)
        DibujarPersona(shader, p, brazo, 0.0f, 0.0f, texCamisa[orden[i].textura], 0, brazoIzq,
                       texCamisa[orden[i].textura]);   // gorra del color de cada piloto
    }

    // ---------- Trofeo: copa dorada que sube de la tarima a las manos del 1o ----------
    {
        float bob = 0.07f * k * std::sin(t * 6.0f);                          // se mueve al festejar
        float bowlY = h[0] + 0.06f + 0.80f + 1.98f * k + bob;               // centro de la copa
        float z = 1.0f * (1.0f - k);
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, bowlY - 0.55f, z));            // base
        DibujarMalla(shader, mallaCilindro, glm::scale(m, glm::vec3(0.34f, 0.08f, 0.34f)), texOro);
        m = glm::translate(base, glm::vec3(0.0f, bowlY - 0.32f, z));                       // pie
        DibujarMalla(shader, mallaCilindro, glm::scale(m, glm::vec3(0.12f, 0.40f, 0.12f)), texOro);
        m = glm::translate(base, glm::vec3(0.0f, bowlY, z));                               // copa
        DibujarMalla(shader, mallaEsfera, glm::scale(m, glm::vec3(0.52f, 0.46f, 0.52f)), texOro);
        for (int lado = -1; lado <= 1; lado += 2)                                          // asas
        {
            m = glm::translate(base, glm::vec3(lado * 0.30f, bowlY, z));
            DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.07f, 0.20f, 0.07f)), texOro);
        }
    }

    // ---------- Publico festejando: grupos a los lados del podio, mirando hacia el 1o ----------
    for (int lado = -1; lado <= 1; lado += 2)
        for (int fila = 0; fila < 4; fila++)
            for (int col = 0; col < 5; col++)
            {
                int id = (lado > 0 ? 100 : 0) + fila * 5 + col;
                float r1 = Hash(id * 1.0f + 7.0f), r2 = Hash(id * 1.0f + 57.0f), r3 = Hash(id * 1.0f + 307.0f);
                float px = lado * (6.9f + col * 1.1f + 0.4f * r1);
                float pz = -4.2f + fila * 1.2f + 0.4f * r2;
                float salto = 0.18f * std::fabs(std::sin(t * (3.5f + 2.0f * r3) + id));      // saltan de gusto
                glm::mat4 q = glm::translate(base, glm::vec3(px, salto, pz));
                q = glm::rotate(q, std::atan2(px, pz), glm::vec3(0.0f, 1.0f, 0.0f));         // mira hacia el podio
                float b1 = 120.0f + 40.0f * std::sin(t * 6.0f + id);
                float b2 = 120.0f + 40.0f * std::sin(t * 6.0f + id + 1.7f);
                DibujarPersona(shader, q, b1, 0.0f, 0.0f, texConfeti[id % 6], 0, b2, texConfeti[(id + 2) % 6]);
            }

    // ---------- Bandera de Mexico (con el escudo) detras del podio, entre dos astas ----------
    {
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 4.0f, -2.6f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(7.2f, 4.5f, 0.12f)), texBanderaMexico);   // 1200 x 750 = 1.6 : 1
    }
    for (int lado = -1; lado <= 1; lado += 2)
    {
        glm::mat4 m = glm::translate(base, glm::vec3(lado * 3.9f, 3.3f, -2.6f));
        DibujarMalla(shader, mallaCilindro, glm::scale(m, glm::vec3(0.12f, 6.6f, 0.12f)), texPlata);
    }

    // ---------- Confeti (cae y gira; la posicion sale de una formula, sin guardar estado) ----------
    const float ALTURA = 14.0f;
    for (int i = 0; i < 260; i++)
    {
        float r1 = Hash(i * 1.0f + 1.0f), r2 = Hash(i * 1.0f + 101.0f), r3 = Hash(i * 1.0f + 201.0f);
        float vel = 1.2f + 1.8f * r3;
        float y = ALTURA - std::fmod(t * vel + r1 * ALTURA, ALTURA);
        float px = (r1 - 0.5f) * 16.0f + 0.8f * std::sin(t * 2.0f + i);
        float pz = (r2 - 0.5f) * 12.0f;
        glm::mat4 m = glm::translate(base, glm::vec3(px, y, pz));
        m = glm::rotate(m, t * (2.0f + 3.0f * r2) + i, glm::vec3(r1, 1.0f, r3));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.20f, 0.02f, 0.13f)), texConfeti[i % 6]);
    }
}

void PodioHUD(int W, int H)
{
    if (!activo) return;
    const glm::vec4 FONDO(0.13f, 0.03f, 0.20f, 0.78f), BLANCO(1, 1, 1, 1), AMARILLO(1.0f, 0.62f, 0.08f, 1.0f);
    const glm::vec4 GRIS(0.8f, 0.8f, 0.85f, 1.0f);

    float w = 460.0f, x = 30.0f, y = 30.0f;
    HUDRect(x, y, w, 70 + 34.0f * (1 + NUM_RIVALES) + 40, FONDO);
    HUDTexto("PODIO", x + 20, y + 18, 34, AMARILLO);
    HUDTexto("TIEMPO TOTAL", x + w - 20 - HUDAnchoTexto("TIEMPO TOTAL", 12), y + 30, 12, GRIS);
    for (int i = 0; i < 1 + NUM_RIVALES; i++)
    {
        float fy = y + 75 + i * 34.0f;
        glm::vec4 col = orden[i].jugador ? AMARILLO : BLANCO;
        HUDTexto(std::to_string(i + 1), x + 20, fy, 20, col);
        HUDTexto(orden[i].nombre, x + 60, fy, 20, col);
        HUDTexto(FormatoTiempo(orden[i].tiempo), x + w - 20 - HUDAnchoTexto(FormatoTiempo(orden[i].tiempo), 20), fy, 20, col);
    }
    HUDTexto("T VOLVER A LA PISTA   P CERRAR", x + 20, y + 75 + (1 + NUM_RIVALES) * 34.0f + 8, 12, GRIS);

    glm::mat4 orto = glm::ortho(0.0f, (float)W, (float)H, 0.0f);
    HUDDibujar(orto, true);
}
