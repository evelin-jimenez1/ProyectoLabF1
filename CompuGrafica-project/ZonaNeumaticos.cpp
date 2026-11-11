// =============================================================
//  ZonaNeumaticos.cpp
//  La zona se acomoda sola junto a la pista usando la spline:
//  se coloca a S_ZONA metros de la meta y LATERAL_ZONA metros a
//  la derecha del centro (por dentro de la recta principal).
// =============================================================
#include "ZonaNeumaticos.h"
#include "Primitivas.h"
#include "Spline.h"
#include "Texturas.h"
#include <glm/gtc/matrix_transform.hpp>

const float S_ZONA = 75.0f;          // metros desde la meta
const float LATERAL_ZONA = 10.0f;    // metros a la derecha del centro de la pista
const float LARGO_ZONA = 40.0f;      // a lo largo de la pista
const float ANCHO_ZONA = 12.0f;      // alejandose de la pista

// Neumatico de F1 (aprox.)
const float RADIO_NEUMATICO = 0.33f;
const float ANCHO_NEUMATICO = 0.36f;

static glm::mat4 origenZona;
static GLuint texLlanta, texPiso, texLinea, texMantaRoja, texMantaAzul;
static GLuint texCono, texNegro, texExtintor, texPlata;


glm::mat4 MatrizZonaNeumaticos() { return origenZona; }


void ZonaNeumaticosInicializar()
{
    glm::vec3 p = SplinePuntoPista(S_ZONA, LATERAL_ZONA);
    p.y = 0.0f;   // sobre el pasto
    origenZona = glm::translate(glm::mat4(1.0f), p);
    origenZona = glm::rotate(origenZona, glm::radians(SplineAnguloY(S_ZONA)), glm::vec3(0, 1, 0));

    texLlanta = CargarTextura("images/llanta.png");
    texPiso = CrearTexturaColor(0.55f, 0.55f, 0.53f);       // concreto
    texLinea = CrearTexturaColor(0.95f, 0.95f, 0.95f);
    texMantaRoja = CrearTexturaColor(0.80f, 0.10f, 0.10f);  // mantas termicas
    texMantaAzul = CrearTexturaColor(0.10f, 0.25f, 0.75f);
    texCono = CrearTexturaFranjas(glm::vec3(1.0f, 0.45f, 0.0f), glm::vec3(1.0f), 5);   // naranja / blanco
    texNegro = CrearTexturaColor(0.08f, 0.08f, 0.08f);
    texExtintor = CrearTexturaColor(0.85f, 0.05f, 0.05f);
    texPlata = CrearTexturaColor(0.75f, 0.75f, 0.78f);
}


// ---------- Piezas ----------
// Neumatico acostado (eje vertical), apoyado en 'base'
static void Neumatico(Shader& sh, glm::vec3 base, GLuint tex)
{
    glm::mat4 model = glm::translate(origenZona, base + glm::vec3(0.0f, ANCHO_NEUMATICO * 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(RADIO_NEUMATICO * 2.0f, ANCHO_NEUMATICO, RADIO_NEUMATICO * 2.0f));
    DibujarMalla(sh, mallaCilindro, model, tex);
}

// Pila de neumaticos; con manta = cubiertos con su manta termica
static void Pila(Shader& sh, glm::vec3 base, int cuantos, GLuint manta)
{
    if (manta)
    {
        float alto = cuantos * ANCHO_NEUMATICO;
        glm::mat4 model = glm::translate(origenZona, base + glm::vec3(0.0f, alto * 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(RADIO_NEUMATICO * 2.1f, alto + 0.02f, RADIO_NEUMATICO * 2.1f));
        DibujarMalla(sh, mallaCilindro, model, manta);
        return;
    }
    for (int i = 0; i < cuantos; i++)
        Neumatico(sh, base + glm::vec3(0.0f, i * ANCHO_NEUMATICO, 0.0f), texLlanta);
}

static void Cono(Shader& sh, glm::vec3 base)
{
    glm::mat4 model = glm::translate(origenZona, base + glm::vec3(0.0f, 0.02f, 0.0f));
    DibujarMalla(sh, mallaCubo, glm::scale(model, glm::vec3(0.40f, 0.04f, 0.40f)), texNegro);   // base cuadrada
    model = glm::translate(origenZona, base + glm::vec3(0.0f, 0.04f + 0.30f, 0.0f));
    DibujarMalla(sh, mallaCono, glm::scale(model, glm::vec3(0.34f, 0.60f, 0.34f)), texCono);
}

static void Extintor(Shader& sh, glm::vec3 base)
{
    // Cuerpo, tapa redonda, valvula y manguera
    glm::mat4 model = glm::translate(origenZona, base + glm::vec3(0.0f, 0.30f, 0.0f));
    DibujarMalla(sh, mallaCilindro, glm::scale(model, glm::vec3(0.20f, 0.55f, 0.20f)), texExtintor);
    model = glm::translate(origenZona, base + glm::vec3(0.0f, 0.575f, 0.0f));
    DibujarMalla(sh, mallaEsfera, glm::scale(model, glm::vec3(0.20f, 0.12f, 0.20f)), texExtintor);
    model = glm::translate(origenZona, base + glm::vec3(0.0f, 0.66f, 0.0f));
    DibujarMalla(sh, mallaCilindro, glm::scale(model, glm::vec3(0.06f, 0.08f, 0.06f)), texPlata);
    model = glm::translate(origenZona, base + glm::vec3(0.07f, 0.52f, 0.0f));
    model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0, 0, 1));
    DibujarMalla(sh, mallaCilindro, glm::scale(model, glm::vec3(0.03f, 0.30f, 0.03f)), texNegro);
    // Base / soporte
    model = glm::translate(origenZona, base + glm::vec3(0.0f, 0.015f, 0.0f));
    DibujarMalla(sh, mallaCubo, glm::scale(model, glm::vec3(0.32f, 0.03f, 0.32f)), texNegro);
}


void DibujarZonaNeumaticos(Shader& sh)
{
    // ----- Piso de concreto con orilla blanca -----
    glm::mat4 model = glm::translate(origenZona, glm::vec3(ANCHO_ZONA * 0.5f, 0.05f, 0.0f));
    DibujarMalla(sh, mallaCubo, glm::scale(model, glm::vec3(ANCHO_ZONA, 0.10f, LARGO_ZONA)), texPiso);
    model = glm::translate(origenZona, glm::vec3(ANCHO_ZONA * 0.5f, 0.105f, LARGO_ZONA * 0.5f - 0.15f));
    DibujarMalla(sh, mallaCubo, glm::scale(model, glm::vec3(ANCHO_ZONA, 0.01f, 0.15f)), texLinea);
    model = glm::translate(origenZona, glm::vec3(ANCHO_ZONA * 0.5f, 0.105f, -LARGO_ZONA * 0.5f + 0.15f));
    DibujarMalla(sh, mallaCubo, glm::scale(model, glm::vec3(ANCHO_ZONA, 0.01f, 0.15f)), texLinea);

    const float PISO = 0.10f;

    // ----- Barrera de llantas del lado de la pista (3 de alto) -----
    for (float z = -LARGO_ZONA * 0.5f + 0.5f; z <= LARGO_ZONA * 0.5f - 0.5f; z += 0.68f)
        Pila(sh, glm::vec3(0.45f, PISO, z), 3, 0);

    // ----- Pilas de neumaticos (3 filas x 6) -----
    for (int fila = 0; fila < 3; fila++)
        for (int k = 0; k < 6; k++)
        {
            glm::vec3 base(3.5f + fila * 2.2f, PISO, -13.0f + k * 5.2f);
            int cuantos = 3 + (k + fila) % 3;                    // de 3 a 5 de alto
            GLuint manta = 0;
            if (fila == 1) manta = (k % 2 == 0) ? texMantaRoja : texMantaAzul;   // con mantas termicas
            Pila(sh, base, cuantos, manta);
            Pila(sh, base + glm::vec3(0.75f, 0.0f, 0.0f), cuantos - 1, 0);      // pila de al lado
        }

    // ----- Unos neumaticos sueltos, parados -----
    for (int k = 0; k < 4; k++)
    {
        model = glm::translate(origenZona, glm::vec3(10.8f, PISO + RADIO_NEUMATICO, -9.0f + k * 0.45f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(8.0f), glm::vec3(0, 0, 1));
        DibujarMalla(sh, mallaCilindro, glm::scale(model, glm::vec3(RADIO_NEUMATICO * 2.0f, ANCHO_NEUMATICO, RADIO_NEUMATICO * 2.0f)), texLlanta);
    }

    // ----- Conos: fila junto a la pista y a la entrada de la zona -----
    // La fila va en el borde exterior del carril de boxes (x = -0.4), pegada al muro de llantas.
    // Antes estaba en x = -2.5, que es el centro del carril (LATERAL_PIT = 7.5 en Boxes.cpp) y el auto
    // pasaba y se detenia encima de los conos. Se apoyan sobre la losa del carril (ALTURA_PISTA).
    for (float z = -18.0f; z <= 18.0f; z += 4.0f)
        Cono(sh, glm::vec3(-0.4f, ALTURA_PISTA + 0.005f, z));
    for (float x = 2.0f; x <= 10.0f; x += 2.0f)
        Cono(sh, glm::vec3(x, PISO, -LARGO_ZONA * 0.5f + 0.8f));

    // ----- Extintores en el fondo de la zona -----
    for (float z = -15.0f; z <= 15.0f; z += 10.0f)
        Extintor(sh, glm::vec3(ANCHO_ZONA - 0.6f, PISO, z));
}