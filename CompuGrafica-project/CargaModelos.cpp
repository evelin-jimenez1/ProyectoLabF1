// =============================================================
//  CargaModelos.cpp
//  - Pista: modelo .obj de Blender cargado con Model.h (Assimp)
//  - Pasto: plano hecho a mano, como el cuadrado de la practica
//           de texturizado pero acostado en el suelo
// =============================================================
#include "CargaModelos.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// IMPORTANTE: este es el UNICO .cpp que incluye Model.h
// (Model.h tiene la funcion TextureFromFile escrita dentro del .h)
#include "Model.h"

// ---------------- Modelos ----------------
// Se guardan como punteros porque un Model solo se puede crear
// despues de glewInit(); por eso se crean dentro de CargarModelos().
static Model* pista = nullptr;

// ---------------- Pasto (plano) ----------------
static GLuint VAO, VBO, EBO;


void CargarModelos()
{
    // ----- Pista -----
    pista = new Model((char*)"Models/pistaf1.obj");

    // ----- Pasto: plano de 1000 x 1000 -----
    GLfloat vertices[] =
    {
        // Positions               // Normal             // Texture Coords
        -500.0f, 0.0f, -500.0f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         500.0f, 0.0f, -500.0f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         500.0f, 0.0f,  500.0f,    0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
        -500.0f, 0.0f,  500.0f,    0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
    };

    GLuint indices[] =
    {  // Note that we start from 0!
        0,1,3,
        1,2,3
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute (en la practica era color; aqui es la normal para la luz)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Texture Coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}


void DibujarPasto(Shader& shader)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.05f, 0.0f));   // un poco abajo de la pista
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void DibujarPista(Shader& shader)
{
    glm::mat4 model = glm::mat4(1.0f);   // en el origen, tamano real
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    pista->Draw(shader);
}


void LiberarModelos()
{
    delete pista;
    pista = nullptr;

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
