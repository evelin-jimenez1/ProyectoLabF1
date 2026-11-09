// =============================================================
//  Recap.cpp - repeticion de la carrera en las pantallas
// =============================================================
#include "Recap.h"
#include <vector>

static const int RW = 192, RH = 108;          // tamano de cada cuadro grabado
static const float FPS = 10.0f;               // cuadros por segundo
static const size_t MAX_CUADROS = 900;        // 90 s por vuelta como maximo

typedef std::vector<unsigned char> Cuadro;    // RW * RH * 3 bytes

static std::vector<Cuadro> grabando;          // la vuelta que se esta corriendo
static std::vector<Cuadro> repeticion;        // la vuelta que se muestra en las pantallas
static float acumGrabar = 0.0f, acumVideo = 0.0f;
static size_t cuadroActual = 0;
static bool subir = false;

static GLuint fbo = 0, texFbo = 0;            // framebuffer chico donde se reduce la imagen
static GLuint texRecap = 0;                   // textura que ven las pantallas

void RecapInicializar()
{
    // framebuffer chico
    glGenTextures(1, &texFbo);
    glBindTexture(GL_TEXTURE_2D, texFbo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, RW, RH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texFbo, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // textura de las pantallas
    glGenTextures(1, &texRecap);
    glBindTexture(GL_TEXTURE_2D, texRecap);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RW, RH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RecapGrabar(int ancho, int alto, float dt, bool activo)
{
    if (!activo) return;
    acumGrabar += dt;
    if (acumGrabar < 1.0f / FPS) return;
    acumGrabar -= 1.0f / FPS;
    if (acumGrabar > 1.0f / FPS) acumGrabar = 0.0f;    // por si hubo un tiron
    if (grabando.size() >= MAX_CUADROS) return;

    // reduce la pantalla completa al framebuffer chico y la lee
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, ancho, alto, 0, 0, RW, RH, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    Cuadro c(RW * RH * 3);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, RW, RH, GL_RGB, GL_UNSIGNED_BYTE, c.data());
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    grabando.push_back(std::move(c));
}

void RecapVueltaTerminada()
{
    if (grabando.size() >= 20)           // al menos 2 s para que valga la pena
    {
        repeticion.swap(grabando);
        cuadroActual = 0;
        acumVideo = 0.0f;
        subir = true;
    }
    grabando.clear();
}

void RecapReiniciar()
{
    grabando.clear();
    repeticion.clear();
    acumGrabar = acumVideo = 0.0f;
    cuadroActual = 0;
    subir = false;
}

void RecapActualizar(float dt)
{
    if (repeticion.empty()) return;
    acumVideo += dt;
    while (acumVideo >= 1.0f / FPS)
    {
        acumVideo -= 1.0f / FPS;
        cuadroActual = (cuadroActual + 1) % repeticion.size();
        subir = true;
    }
    if (subir)
    {
        glBindTexture(GL_TEXTURE_2D, texRecap);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, RW, RH, GL_RGB, GL_UNSIGNED_BYTE, repeticion[cuadroActual].data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        subir = false;
    }
}

GLuint RecapTextura()
{
    return repeticion.empty() ? 0 : texRecap;
}
