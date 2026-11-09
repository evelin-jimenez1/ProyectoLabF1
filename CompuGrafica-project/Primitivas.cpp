// =============================================================
//  Primitivas.cpp
//  Cada figura se arma con un arreglo de vertices
//  (posicion, normal, coordenada de textura) igual que el plano
//  del pasto, pero calculados con ciclos (seno y coseno).
// =============================================================
#include "Primitivas.h"
#include <vector>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Malla mallaCubo;
Malla mallaCilindro;
Malla mallaEsfera;
Malla mallaCono;

static const float PI = 3.14159265f;

// Sube los vertices (8 floats cada uno) y los indices a la GPU
static Malla CrearMalla(const std::vector<float>& v, const std::vector<GLuint>& idx)
{
    Malla m;
    GLuint VBO, EBO;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
    // Posicion
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Coordenadas de textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    m.numIndices = (GLsizei)idx.size();
    return m;
}

static void Vertice(std::vector<float>& v, glm::vec3 p, glm::vec3 n, float u, float t)
{
    v.insert(v.end(), { p.x, p.y, p.z, n.x, n.y, n.z, u, t });
}

// ---------------- Cubo: 6 caras, 4 vertices cada una ----------------
static Malla CrearCubo()
{
    std::vector<float> v;
    std::vector<GLuint> idx;
    // normal, eje "u" y eje "v" de cada cara
    glm::vec3 caras[6][3] = {
        { { 0, 0, 1}, { 1, 0, 0}, {0, 1, 0} },   // frente
        { { 0, 0,-1}, {-1, 0, 0}, {0, 1, 0} },   // atras
        { { 1, 0, 0}, { 0, 0,-1}, {0, 1, 0} },   // derecha
        { {-1, 0, 0}, { 0, 0, 1}, {0, 1, 0} },   // izquierda
        { { 0, 1, 0}, { 1, 0, 0}, {0, 0,-1} },   // arriba
        { { 0,-1, 0}, { 1, 0, 0}, {0, 0, 1} },   // abajo
    };
    for (int c = 0; c < 6; c++)
    {
        glm::vec3 n = caras[c][0], a = caras[c][1], b = caras[c][2];
        GLuint base = (GLuint)(v.size() / 8);
        Vertice(v, 0.5f * (n - a - b), n, 0, 0);
        Vertice(v, 0.5f * (n + a - b), n, 1, 0);
        Vertice(v, 0.5f * (n + a + b), n, 1, 1);
        Vertice(v, 0.5f * (n - a + b), n, 0, 1);
        idx.insert(idx.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
    }
    return CrearMalla(v, idx);
}

// ---------------- Cilindro (eje Y) o cono ----------------
// radioArriba = 0.5 -> cilindro, radioArriba = 0 -> cono
static Malla CrearCilindro(int lados, float radioArriba)
{
    std::vector<float> v;
    std::vector<GLuint> idx;
    float radioAbajo = 0.5f;
    float pendiente = (radioAbajo - radioArriba);   // para inclinar la normal del cono

    // Costado
    for (int i = 0; i <= lados; i++)
    {
        float a = 2.0f * PI * i / lados;
        float c = cos(a), s = sin(a);
        glm::vec3 n = glm::normalize(glm::vec3(c, pendiente, s));
        Vertice(v, glm::vec3(c * radioAbajo, -0.5f, s * radioAbajo), n, (float)i / lados, 0.0f);
        Vertice(v, glm::vec3(c * radioArriba, 0.5f, s * radioArriba), n, (float)i / lados, 1.0f);
    }
    for (int i = 0; i < lados; i++)
    {
        GLuint a = i * 2;
        idx.insert(idx.end(), { a, a + 1, a + 3, a, a + 3, a + 2 });
    }

    // Tapas (abajo siempre; arriba solo si no es cono)
    for (int tapa = 0; tapa < 2; tapa++)
    {
        float y = tapa == 0 ? -0.5f : 0.5f;
        float r = tapa == 0 ? radioAbajo : radioArriba;
        if (r <= 0.0f) continue;
        glm::vec3 n(0.0f, tapa == 0 ? -1.0f : 1.0f, 0.0f);
        GLuint centro = (GLuint)(v.size() / 8);
        Vertice(v, glm::vec3(0, y, 0), n, 0.5f, 0.5f);
        for (int i = 0; i <= lados; i++)
        {
            float a = 2.0f * PI * i / lados;
            Vertice(v, glm::vec3(cos(a) * r, y, sin(a) * r), n, 0.5f + 0.5f * cos(a), 0.5f + 0.5f * sin(a));
        }
        for (int i = 0; i < lados; i++)
            idx.insert(idx.end(), { centro, centro + 1 + i, centro + 2 + i });
    }
    return CrearMalla(v, idx);
}

// ---------------- Esfera (paralelos y meridianos) ----------------
static Malla CrearEsfera(int paralelos, int meridianos)
{
    std::vector<float> v;
    std::vector<GLuint> idx;
    for (int i = 0; i <= paralelos; i++)
    {
        float lat = PI * i / paralelos;             // 0 (arriba) .. PI (abajo)
        for (int j = 0; j <= meridianos; j++)
        {
            float lon = 2.0f * PI * j / meridianos;
            glm::vec3 n(sin(lat) * cos(lon), cos(lat), sin(lat) * sin(lon));
            Vertice(v, 0.5f * n, n, (float)j / meridianos, 1.0f - (float)i / paralelos);
        }
    }
    for (int i = 0; i < paralelos; i++)
        for (int j = 0; j < meridianos; j++)
        {
            GLuint a = i * (meridianos + 1) + j;
            GLuint b = a + meridianos + 1;
            idx.insert(idx.end(), { a, b, a + 1, a + 1, b, b + 1 });
        }
    return CrearMalla(v, idx);
}


void PrimitivasInicializar()
{
    mallaCubo = CrearCubo();
    mallaCilindro = CrearCilindro(24, 0.5f);
    mallaCono = CrearCilindro(24, 0.0f);
    mallaEsfera = CrearEsfera(16, 24);
}


void DibujarMalla(Shader& shader, const Malla& malla, const glm::mat4& model, GLuint textura)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textura);
    glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse1"), 0);
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(malla.VAO);
    glDrawElements(GL_TRIANGLES, malla.numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


// ---------------- Texturas por codigo ----------------
static GLuint SubirTextura(const std::vector<unsigned char>& pix, int w, int h)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

GLuint CrearTexturaColor(float r, float g, float b)
{
    std::vector<unsigned char> pix = { (unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255) };
    return SubirTextura(pix, 1, 1);
}

GLuint CrearTexturaFranjas(glm::vec3 c1, glm::vec3 c2, int numFranjas)
{
    std::vector<unsigned char> pix;
    for (int i = 0; i < numFranjas; i++)
    {
        glm::vec3 c = (i % 2 == 0) ? c1 : c2;
        pix.push_back((unsigned char)(c.r * 255));
        pix.push_back((unsigned char)(c.g * 255));
        pix.push_back((unsigned char)(c.b * 255));
    }
    return SubirTextura(pix, 1, numFranjas);   // 1 x N: las franjas van en V
}
