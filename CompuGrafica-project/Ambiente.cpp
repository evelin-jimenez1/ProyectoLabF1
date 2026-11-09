// =============================================================
//  Ambiente.cpp - dia / atardecer / noche
// =============================================================
#include "Ambiente.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------- Los tres ambientes ----------
static const ParamsAmbiente AMBIENTES[3] =
{
    // DIA: igual que el look original (sol alto y blanco calido)
    { glm::vec3(0.70f, 0.85f, 0.95f), glm::vec3(0.22f, 0.45f, 0.82f),
      glm::normalize(glm::vec3(0.35f, 0.85f, 0.30f)), glm::vec3(1.0f, 0.95f, 0.80f),
      glm::vec3(1.0f, 0.95f, 0.80f), glm::vec3(0.40f, 0.38f, 0.32f),
      0.0020f, 1.0f, 0.0f },

    // ATARDECER: sol bajo y naranja al fondo de la recta, cielo naranja -> morado
    { glm::vec3(1.00f, 0.60f, 0.30f), glm::vec3(0.28f, 0.18f, 0.46f),
      glm::normalize(glm::vec3(0.80f, 0.30f, 0.42f)), glm::vec3(1.0f, 0.55f, 0.20f),
      glm::vec3(1.35f, 0.80f, 0.42f), glm::vec3(0.50f, 0.40f, 0.46f),
      0.0030f, 1.0f, 0.0f },

    // NOCHE: luna azul, estrellas, sin sombras (los reflectores hacen el trabajo)
    { glm::vec3(0.05f, 0.07f, 0.16f), glm::vec3(0.008f, 0.012f, 0.05f),
      glm::normalize(glm::vec3(-0.30f, 0.70f, 0.20f)), glm::vec3(0.45f, 0.55f, 0.90f),
      glm::vec3(0.16f, 0.21f, 0.40f), glm::vec3(0.11f, 0.14f, 0.26f),
      0.0035f, 0.0f, 1.0f },
};

static ModoAmbiente modo = AMB_ATARDECER;      // arranca en atardecer
static ParamsAmbiente actual = AMBIENTES[AMB_ATARDECER];

static Shader* shaderCielo = nullptr;
static GLuint vaoVacio = 0;       // el cielo no usa vertices, pero core profile exige un VAO

void AmbienteInicializar()
{
    shaderCielo = new Shader("Shader/cielo.vs", "Shader/cielo.frag");
    glGenVertexArrays(1, &vaoVacio);
    actual = AMBIENTES[modo];
}

void AmbienteAlternar()
{
    modo = (ModoAmbiente)((modo + 1) % 3);
}

ModoAmbiente AmbienteModo() { return modo; }
const ParamsAmbiente& AmbienteParams() { return actual; }

void AmbienteActualizar(float dt)
{
    // Cada valor se acerca al del modo elegido (transicion de ~1 s)
    const ParamsAmbiente& d = AMBIENTES[modo];
    float k = glm::min(1.0f, 3.0f * dt);
    actual.horizonte += (d.horizonte - actual.horizonte) * k;
    actual.cenit += (d.cenit - actual.cenit) * k;
    actual.solDir = glm::normalize(actual.solDir + (d.solDir - actual.solDir) * k);
    actual.solColor += (d.solColor - actual.solColor) * k;
    actual.luz += (d.luz - actual.luz) * k;
    actual.ambiente += (d.ambiente - actual.ambiente) * k;
    actual.niebla += (d.niebla - actual.niebla) * k;
    actual.sombras += (d.sombras - actual.sombras) * k;
    actual.estrellas += (d.estrellas - actual.estrellas) * k;
}

void AmbienteDibujarCielo(const glm::mat4& proyeccion, const glm::mat4& vista)
{
    shaderCielo->Use();
    GLuint p = shaderCielo->Program;
    glUniformMatrix4fv(glGetUniformLocation(p, "invProjection"), 1, GL_FALSE, glm::value_ptr(glm::inverse(proyeccion)));
    glUniformMatrix4fv(glGetUniformLocation(p, "invView"), 1, GL_FALSE, glm::value_ptr(glm::inverse(vista)));
    glUniform3fv(glGetUniformLocation(p, "horizonte"), 1, glm::value_ptr(actual.horizonte));
    glUniform3fv(glGetUniformLocation(p, "cenit"), 1, glm::value_ptr(actual.cenit));
    glUniform3fv(glGetUniformLocation(p, "solDir"), 1, glm::value_ptr(actual.solDir));
    glUniform3fv(glGetUniformLocation(p, "solColor"), 1, glm::value_ptr(actual.solColor));
    glUniform1f(glGetUniformLocation(p, "estrellas"), actual.estrellas);

    glDisable(GL_DEPTH_TEST);          // el cielo va al fondo de todo
    glBindVertexArray(vaoVacio);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void AmbienteEnviar(Shader& shader)
{
    GLuint p = shader.Program;
    // El sol se coloca muy lejos en su direccion: la luz sale casi paralela (direccional)
    glm::vec3 pos = actual.solDir * 2000.0f;
    glUniform3fv(glGetUniformLocation(p, "lightPos"), 1, glm::value_ptr(pos));
    glUniform3fv(glGetUniformLocation(p, "lightColor"), 1, glm::value_ptr(actual.luz));
    glUniform3fv(glGetUniformLocation(p, "ambientColor"), 1, glm::value_ptr(actual.ambiente));
    glUniform3fv(glGetUniformLocation(p, "fogColor"), 1, glm::value_ptr(actual.horizonte));
    glUniform1f(glGetUniformLocation(p, "fogDensity"), actual.niebla);
}

void AmbienteLiberar()
{
    glDeleteVertexArrays(1, &vaoVacio);
    delete shaderCielo;
    shaderCielo = nullptr;
}
