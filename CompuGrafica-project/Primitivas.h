#pragma once
// =============================================================
//  Primitivas: figuras basicas hechas por codigo (sin .obj)
//  Cubo, cilindro, esfera y cono con normales y coordenadas de
//  textura, para dibujarse con el mismo shader de la pista.
//  Se usan para el piloto, la zona de neumaticos y la utileria.
// =============================================================
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"

struct Malla
{
    GLuint VAO = 0;
    GLsizei numIndices = 0;
};

// Todas miden 1 (de -0.5 a 0.5) y estan centradas en el origen,
// asi se acomodan solo con translate / rotate / scale.
//  - Cubo:     1 x 1 x 1
//  - Cilindro: eje en Y, radio 0.5, alto 1 (con tapas)
//  - Esfera:   radio 0.5
//  - Cono:     eje en Y, radio de la base 0.5, alto 1 (punta arriba)
void PrimitivasInicializar();     // se llama UNA vez despues de glewInit()

extern Malla mallaCubo;
extern Malla mallaCilindro;
extern Malla mallaEsfera;
extern Malla mallaCono;

// Dibuja una malla con la matriz 'model' y la textura indicada
void DibujarMalla(Shader& shader, const Malla& malla, const glm::mat4& model, GLuint textura);

// Texturas generadas por codigo (no necesitan archivo)
GLuint CrearTexturaColor(float r, float g, float b);
// Franjas horizontales (en V) alternando dos colores
GLuint CrearTexturaFranjas(glm::vec3 color1, glm::vec3 color2, int numFranjas);
