// =============================================================
//  Sombras.cpp - shadow mapping direccional
// =============================================================
#include "Sombras.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const int RESOLUCION = 2048;       // tamano del mapa de sombras
static const float MEDIO_LADO = 45.0f;    // m a cada lado del auto que reciben sombra
static const float DIST_SOL = 120.0f;     // a que distancia del auto se coloca el sol
static const float CERCA = 60.0f;         // plano cercano / lejano de la camara del sol
static const float LEJOS = 180.0f;

static GLuint fbo = 0;
static GLuint texProfundidad = 0;
static Shader* shaderSombra = nullptr;

void SombrasInicializar()
{
    shaderSombra = new Shader("Shader/sombra.vs", "Shader/sombra.frag");

    // Textura de profundidad
    glGenTextures(1, &texProfundidad);
    glBindTexture(GL_TEXTURE_2D, texProfundidad);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, RESOLUCION, RESOLUCION, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Fuera del mapa = profundidad 1 (no hay sombra)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borde[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borde);

    // Framebuffer que solo guarda profundidad
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texProfundidad, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::mat4 SombrasRender(glm::vec3 centro, glm::vec3 direccionSol,
                        const std::function<void(Shader&)>& dibujar)
{
    glm::vec3 sol = glm::normalize(direccionSol);
    glm::vec3 arriba = (std::fabs(sol.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

    glm::mat4 vista = glm::lookAt(centro + sol * DIST_SOL, centro, arriba);
    glm::mat4 proy = glm::ortho(-MEDIO_LADO, MEDIO_LADO, -MEDIO_LADO, MEDIO_LADO, CERCA, LEJOS);
    glm::mat4 lightSpace = proy * vista;

    // Guardar el viewport actual para regresarlo
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, RESOLUCION, RESOLUCION);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    shaderSombra->Use();
    glUniformMatrix4fv(glGetUniformLocation(shaderSombra->Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpace));
    dibujar(*shaderSombra);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    return lightSpace;
}

void SombrasEnviar(Shader& shader, const glm::mat4& lightSpace, float intensidad)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpace));
    glUniform1f(glGetUniformLocation(shader.Program, "sombrasActivas"), intensidad);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texProfundidad);
    glUniform1i(glGetUniformLocation(shader.Program, "shadowMap"), 5);
    glActiveTexture(GL_TEXTURE0);
}

void SombrasLiberar()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texProfundidad);
    delete shaderSombra;
    shaderSombra = nullptr;
}
