// =============================================================
//  Estadio.cpp - gradas, porticos, linea de meta y vallas
// =============================================================
#include "Estadio.h"
#include "Spline.h"
#include "Primitivas.h"
#include "Texturas.h"
#include "CargaModelos.h"
#include "Juego.h"

#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------- Texturas ----------
static const int NUM_PATROCINIOS = 6;
static GLuint texPatrocinio[NUM_PATROCINIOS];
static GLuint texDamero = 0;          // linea de meta (8 x 2 cuadros)
static GLuint texNegro, texGris, texBlanco, texLuzApagada, texLuzRoja;
static GLuint texTronco, texCopa[2], texCerro[2];
static GLuint texPiano = 0;                 // franjas rojas y blancas (images/piano.png)
static GLuint vaoPianos = 0;
static int numVerticesPianos = 0;
// Asfalto y lineas generados desde la spline
static GLuint texAsfalto = 0, texAsfaltoNormal = 0, texAsfaltoEsp = 0, texLineaPista = 0;
static GLuint vaoAsfalto = 0, vaoLineas = 0;
static int numVerticesAsfalto = 0, numVerticesLineas = 0;

// Arboles: posiciones (x, z, escala) calculadas una vez, lejos de la pista y de las zonas ocupadas
struct Arbol { float x, z, escala; int tipo; };
static std::vector<Arbol> arboles;

// Textura de tablero de ajedrez (blanco y negro) con filtro NEAREST
GLuint CrearTexturaDamero(int columnas, int filas)
{
    std::vector<unsigned char> pix;
    for (int y = 0; y < filas; y++)
        for (int x = 0; x < columnas; x++)
        {
            unsigned char v = ((x + y) % 2 == 0) ? 245 : 20;
            pix.push_back(v); pix.push_back(v); pix.push_back(v);
        }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, columnas, filas, 0, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

// ===================== PIANOS (CURBS) =====================
// Franja roja y blanca pegada a cada orilla del asfalto, generada a lo largo de la spline:
// una tira plana arriba y un borde que baja hasta el pasto por el lado de afuera.
static const float PIANO_DENTRO = 4.0f;     // orilla del asfalto (ANCHO_PISTA / 2)
static const float PIANO_FUERA = 4.55f;     // donde empiezan los petales de Dia de Muertos
static const float PIANO_LARGO_TEXTURA = 3.0f;   // metros que cubre una repeticion de la textura

static void Vert(std::vector<float>& v, glm::vec3 p, glm::vec3 n, float u, float t)
{
    v.insert(v.end(), { p.x, p.y, p.z, n.x, n.y, n.z, u, t });
}

static void CrearPianos()
{
    std::vector<float> v;
    float L = SplineLongitud();
    const float Y = ALTURA_PISTA + 0.015f;
    for (int lado = -1; lado <= 1; lado += 2)
        for (float s0 = 0.0f; s0 < L; s0 += 1.0f)
        {
            // el lado derecho se interrumpe donde va el carril de boxes
            if (lado > 0 && s0 > 10.0f && s0 < 140.0f) continue;
            float s1 = glm::min(s0 + 1.0f, L);
            float t0 = s0 / PIANO_LARGO_TEXTURA, t1 = s1 / PIANO_LARGO_TEXTURA;
            glm::vec3 a = SplinePuntoPista(s0, lado * PIANO_DENTRO), b = SplinePuntoPista(s0, lado * PIANO_FUERA);
            glm::vec3 c = SplinePuntoPista(s1, lado * PIANO_FUERA), d = SplinePuntoPista(s1, lado * PIANO_DENTRO);
            a.y = b.y = c.y = d.y = Y;
            glm::vec3 arriba(0, 1, 0);
            // tira de arriba (dos triangulos)
            Vert(v, a, arriba, 0, t0); Vert(v, b, arriba, 1, t0); Vert(v, c, arriba, 1, t1);
            Vert(v, a, arriba, 0, t0); Vert(v, c, arriba, 1, t1); Vert(v, d, arriba, 0, t1);
            // borde de afuera: baja hasta el pasto
            glm::vec3 n = glm::normalize(glm::vec3(lado * glm::normalize(SplineDerecha(s0)).x, 0.0f, lado * glm::normalize(SplineDerecha(s0)).z));
            glm::vec3 b0(b.x, 0.0f, b.z), c0(c.x, 0.0f, c.z);
            Vert(v, b, n, 0, t0); Vert(v, b0, n, 1, t0); Vert(v, c0, n, 1, t1);
            Vert(v, b, n, 0, t0); Vert(v, c0, n, 1, t1); Vert(v, c, n, 0, t1);
        }
    numVerticesPianos = (int)(v.size() / 8);

    GLuint vbo;
    glGenVertexArrays(1, &vaoPianos);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vaoPianos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void DibujarPianos(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);
    UsarTextura(shader, texPiano, 1.0f, 1.0f);
    glm::mat4 model(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(vaoPianos);
    glDrawArrays(GL_TRIANGLES, 0, numVerticesPianos);
    glBindVertexArray(0);
}

// ===================== ASFALTO PROCEDURAL =====================
// La pista se genera a partir de la curva spline (Spline.cpp): una tira de ANCHO_PISTA metros
// centrada en la curva, con un borde que baja al pasto, mas las lineas blancas
// (orillas y raya central discontinua).
static const float TEXTURA_ASFALTO_M = 6.0f;      // metros que cubre una repeticion de la textura

// Crea un VAO (posicion, normal, uv) con los vertices dados
static GLuint SubirMalla(const std::vector<float>& v)
{
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return vao;
}

// Cuadrito entre dos metros de la pista y dos laterales, a la altura 'y'
static void Tira(std::vector<float>& v, float s0, float s1, float latA, float latB, float y,
                 float u0, float u1, float t0, float t1)
{
    glm::vec3 a = SplinePuntoPista(s0, latA), b = SplinePuntoPista(s0, latB);
    glm::vec3 c = SplinePuntoPista(s1, latB), d = SplinePuntoPista(s1, latA);
    a.y = b.y = c.y = d.y = y;
    glm::vec3 n(0, 1, 0);
    Vert(v, a, n, u0, t0); Vert(v, b, n, u1, t0); Vert(v, c, n, u1, t1);
    Vert(v, a, n, u0, t0); Vert(v, c, n, u1, t1); Vert(v, d, n, u0, t1);
}

static void CrearAsfalto()
{
    float L = SplineLongitud();
    float mitad = ANCHO_PISTA * 0.5f;
    std::vector<float> asfalto, lineas;

    for (float s0 = 0.0f; s0 < L; s0 += 1.0f)
    {
        float s1 = glm::min(s0 + 1.0f, L);
        float t0 = s0 / TEXTURA_ASFALTO_M, t1 = s1 / TEXTURA_ASFALTO_M;
        float u0 = 0.0f, u1 = ANCHO_PISTA / TEXTURA_ASFALTO_M;

        // superficie
        Tira(asfalto, s0, s1, -mitad, mitad, ALTURA_PISTA, u0, u1, t0, t1);

        // bordes que bajan al pasto (izquierdo y derecho)
        for (int lado = -1; lado <= 1; lado += 2)
        {
            glm::vec3 a = SplinePuntoPista(s0, lado * mitad), c = SplinePuntoPista(s1, lado * mitad);
            glm::vec3 R = SplineDerecha(s0);
            glm::vec3 n = glm::normalize(glm::vec3(lado * R.x, 0.0f, lado * R.z));
            glm::vec3 a1 = a, c1 = c, a0 = a, c0 = c;
            a1.y = c1.y = ALTURA_PISTA; a0.y = c0.y = 0.0f;
            Vert(asfalto, a1, n, 0, t0); Vert(asfalto, a0, n, 1, t0); Vert(asfalto, c0, n, 1, t1);
            Vert(asfalto, a1, n, 0, t0); Vert(asfalto, c0, n, 1, t1); Vert(asfalto, c1, n, 0, t1);
        }

        // lineas blancas: orillas continuas (menos donde esta el carril de boxes)
        const float YL = ALTURA_PISTA + 0.01f;
        Tira(lineas, s0, s1, -mitad + 0.20f, -mitad + 0.38f, YL, 0, 1, 0, 1);
        if (!(s0 > 10.0f && s0 < 140.0f))
            Tira(lineas, s0, s1, mitad - 0.38f, mitad - 0.20f, YL, 0, 1, 0, 1);
        // raya central discontinua: 3 m con linea y 6 m sin ella
        if (std::fmod(s0, 9.0f) < 3.0f)
            Tira(lineas, s0, s1, -0.08f, 0.08f, YL, 0, 1, 0, 1);
    }
    numVerticesAsfalto = (int)(asfalto.size() / 8);
    numVerticesLineas = (int)(lineas.size() / 8);
    vaoAsfalto = SubirMalla(asfalto);
    vaoLineas = SubirMalla(lineas);
}

void DibujarAsfalto(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);
    glm::mat4 model(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // asfalto con mapa normal y especular
    UsarTextura(shader, texAsfalto, 1.0f, 1.0f);
    UsarMapas(shader, texAsfaltoNormal, texAsfaltoEsp);
    glBindVertexArray(vaoAsfalto);
    glDrawArrays(GL_TRIANGLES, 0, numVerticesAsfalto);
    QuitarMapas(shader);

    // lineas blancas
    UsarTextura(shader, texLineaPista, 1.0f, 1.0f);
    glBindVertexArray(vaoLineas);
    glDrawArrays(GL_TRIANGLES, 0, numVerticesLineas);
    glBindVertexArray(0);
}

void EstadioInicializar()
{
    for (int i = 0; i < NUM_PATROCINIOS; i++)
    {
        std::string ruta = "images/patrocinios/patrocinio" + std::to_string(i + 1) + ".png";
        texPatrocinio[i] = CargarTextura(ruta.c_str());
    }
    texDamero = CrearTexturaDamero(8, 2);
    texNegro = CrearTexturaColor(0.05f, 0.05f, 0.06f);
    texGris = CrearTexturaColor(0.35f, 0.36f, 0.40f);
    texBlanco = CrearTexturaColor(0.95f, 0.95f, 0.95f);
    texLuzApagada = CrearTexturaColor(0.25f, 0.02f, 0.02f);
    texLuzRoja = CrearTexturaColor(1.0f, 0.08f, 0.05f);

    // ---- Pianos (curbs) rojos y blancos a las orillas de la pista ----
    texPiano = CargarTextura("images/piano.png");
    CrearPianos();

    // ---- Asfalto procedural con mapa normal y especular ----
    texAsfalto = CargarTextura("images/asfalto.png");
    texAsfaltoNormal = CargarTextura("images/asfalto_normal.png");
    texAsfaltoEsp = CargarTextura("images/asfalto_especular.png");
    texLineaPista = CargarTextura("images/linea_blanca.png");
    CrearAsfalto();

    // ---- Arboles y cerros ----
    texTronco = CrearTexturaColor(0.30f, 0.20f, 0.12f);
    texCopa[0] = CrearTexturaColor(0.10f, 0.38f, 0.15f);
    texCopa[1] = CrearTexturaColor(0.20f, 0.45f, 0.12f);
    texCerro[0] = CrearTexturaColor(0.16f, 0.30f, 0.20f);
    texCerro[1] = CrearTexturaColor(0.24f, 0.25f, 0.40f);

    // puntos de la pista para medir la distancia a ella
    std::vector<glm::vec3> pista;
    for (float s = 0.0f; s < SplineLongitud(); s += 6.0f) pista.push_back(SplinePosicion(s));

    unsigned int semilla = 12345u;
    auto azar = [&]() { semilla = semilla * 1664525u + 1013904223u; return (semilla >> 8) / 16777216.0f; };
    for (int intento = 0; intento < 2000 && arboles.size() < 150; intento++)
    {
        float x = (azar() - 0.5f) * 640.0f, z = (azar() - 0.5f) * 520.0f;
        float dmin = 1e9f;
        for (const glm::vec3& q : pista) dmin = glm::min(dmin, std::sqrt((x - q.x) * (x - q.x) + (z - q.z) * (z - q.z)));
        bool ocupado = (x > -230 && x < -190 && z > -130 && z < -40)      // gradas Foro Sol
                    || (z > -190 && z < -145 && x > -80 && x < 90)        // gradas de la recta
                    || (x > 10 && x < 150 && z > -135 && z < -95)         // boxes y zona de neumaticos
                    || (std::sqrt((x - 45) * (x - 45) + (z + 75) * (z + 75)) < 25); // podio
        if (dmin > 32.0f && !ocupado)
            arboles.push_back({ x, z, 0.8f + 0.8f * azar(), (int)(azar() * 2) });
    }
}

// Pone en el shader "esta textura se ve con su propio color" (0 = recibe luz normal)
static void BrilloPropio(Shader& shader, float v)
{
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), v);
}

// Posicion en el piso y giro (grados) de un punto de la pista.
// 'lateral' > 0 = derecha de la pista.
static glm::mat4 MatrizEnPista(float s, float lateral, float giroExtra)
{
    glm::vec3 p = SplinePuntoPista(s, lateral);
    p.y = 0.0f;
    glm::mat4 m = glm::translate(glm::mat4(1.0f), p);
    return glm::rotate(m, glm::radians(SplineAnguloY(s) + giroExtra), glm::vec3(0.0f, 1.0f, 0.0f));
}

// ===================== GRADAS CON PUBLICO =====================
// Ancho de un modulo de grada (m)
static const float ANCHO_GRADA = 14.5f;

void DibujarGradasEstadio(Shader& shader)
{
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
    BrilloPropio(shader, 0.0f);
    float L = SplineLongitud();

    // 1) Zona de estadio (Foro Sol): 4 modulos al exterior de la curva del marcador,
    //    mirando hacia la pista (+X).
    for (int i = 0; i < 4; i++)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(-203.0f, 0.0f, -105.0f + i * ANCHO_GRADA));
        m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        DibujarGradasEn(shader, m);
    }

    // 2) Gradas de la recta principal (lado izquierdo, frente a la meta)
    for (int i = 0; i < 8; i++)
    {
        float s = L - 50.0f + i * ANCHO_GRADA;
        DibujarGradasEn(shader, MatrizEnPista(s, -18.0f, 90.0f));   // miran hacia la pista
    }
}

// ===================== LINEA DE META =====================
void DibujarLineaMeta(Shader& shader)
{
    BrilloPropio(shader, 0.0f);
    glm::mat4 m = MatrizEnPista(0.0f, 0.0f, 0.0f);
    m = glm::translate(m, glm::vec3(0.0f, ALTURA_PISTA + 0.02f, 0.0f));
    m = glm::scale(m, glm::vec3(ANCHO_PISTA, 0.02f, 1.6f));
    DibujarMalla(shader, mallaCubo, m, texDamero);
}

// ===================== PORTICOS DE CRONOMETRAJE =====================
// Un porticos cruza la pista a 's' metros. Mira hacia +Z (hacia los autos que llegan).
// 'semaforo' = true: lleva las 5 luces de salida.
static void DibujarPortico(Shader& shader, float s, int patrocinio, bool semaforo, int lucesEncendidas)
{
    glm::mat4 base = MatrizEnPista(s, 0.0f, 0.0f);
    const float X = 9.0f;             // columnas fuera de las barreras (a +-7 m)
    const float ALTO = 9.0f;

    // Columnas
    for (int lado = -1; lado <= 1; lado += 2)
    {
        glm::mat4 m = glm::translate(base, glm::vec3(lado * X, ALTO / 2.0f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(0.9f, ALTO, 0.9f)), texGris);
    }
    // Viga con patrocinador
    {
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, ALTO + 0.5f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(2.0f * X + 0.9f, 1.8f, 1.0f)), texPatrocinio[patrocinio]);
    }
    // Caja del cronometraje (colgando al centro)
    {
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, ALTO - 1.3f, 0.0f));
        DibujarMalla(shader, mallaCubo, glm::scale(m, glm::vec3(7.2f, 1.8f, 0.8f)), texNegro);
    }
    if (!semaforo) return;

    // Semaforo de salida: 5 luces rojas (encendidas = con brillo propio)
    for (int i = 0; i < 5; i++)
    {
        bool on = i < lucesEncendidas;
        glm::mat4 m = glm::translate(base, glm::vec3((i - 2) * 1.25f, ALTO - 1.3f, 0.5f));
        BrilloPropio(shader, on ? 1.0f : 0.0f);
        DibujarMalla(shader, mallaEsfera, glm::scale(m, glm::vec3(1.0f)), on ? texLuzRoja : texLuzApagada);
    }
    BrilloPropio(shader, 0.0f);
}

void DibujarPorticos(Shader& shader)
{
    float L = SplineLongitud();
    // Semaforo: las 5 luces estan prendidas mientras esperas el arranque
    int luces = (juego.estado == ESPERANDO_ARRANQUE) ? 5 : 0;

    DibujarPortico(shader, 24.0f, 0, true, luces);              // salida / meta (24 m adelante de la linea)
    DibujarPortico(shader, L * 1.0f / 3.0f, 1, false, 0);       // sector 1
    DibujarPortico(shader, L * 2.0f / 3.0f, 2, false, 0);       // sector 2
}

// ===================== VALLAS PUBLICITARIAS =====================
// Rectas donde se colocan (metros sobre la spline): recta principal y recta larga
struct Tramo { float ini, fin; };

void DibujarVallas(Shader& shader)
{
    static const float LARGO = 10.0f, ALTO = 1.3f, GROSOR = 0.15f;
    static const float PASO = 11.0f;                 // una valla cada 11 m
    static const float LATERAL = 9.6f;               // detras de las barreras (a +-7 m)
    float L = SplineLongitud();
    Tramo tramos[3] = { { L - 170.0f, L }, { 0.0f, 200.0f }, { 335.0f, 480.0f } };

    BrilloPropio(shader, 0.0f);
    int n = 0;
    for (const Tramo& t : tramos)
        for (float s = t.ini; s < t.fin; s += PASO, n++)
            for (int lado = -1; lado <= 1; lado += 2)
            {
                if (lado > 0 && s > 5.0f && s < 150.0f) continue;   // ahi va el carril de boxes
                glm::mat4 m = MatrizEnPista(s, lado * LATERAL, 0.0f);
                m = glm::translate(m, glm::vec3(0.0f, ALTO / 2.0f + 0.1f, 0.0f));
                m = glm::scale(m, glm::vec3(GROSOR, ALTO, LARGO));   // el largo va a lo largo de la pista (eje Z local)
                DibujarMalla(shader, mallaCubo, m, texPatrocinio[(n + (lado > 0 ? 3 : 0)) % NUM_PATROCINIOS]);
            }
}


// ===================== ARBOLES Y CERROS =====================
void DibujarArbolesYCerros(Shader& shader)
{
    BrilloPropio(shader, 0.0f);

    // Cerros: elipsoides grandes en un anillo alrededor del terreno (la niebla los difumina)
    for (int i = 0; i < 16; i++)
    {
        float a = glm::radians(22.5f * i + 7.0f * std::sin(i * 3.1f));
        float r = 430.0f + 25.0f * std::sin(i * 1.7f);
        float alto = 70.0f + 40.0f * (0.5f + 0.5f * std::sin(i * 2.3f));
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(r * std::cos(a), 0.0f, r * std::sin(a)));
        DibujarMalla(shader, mallaEsfera, glm::scale(m, glm::vec3(240.0f, alto * 2.0f, 170.0f)), texCerro[i % 2]);
    }

    // Arboles: tronco + dos conos de copa
    for (const Arbol& t : arboles)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(t.x, 0.0f, t.z));
        b = glm::scale(b, glm::vec3(t.escala));
        DibujarMalla(shader, mallaCilindro, glm::scale(glm::translate(b, glm::vec3(0.0f, 1.5f, 0.0f)), glm::vec3(0.5f, 3.0f, 0.5f)), texTronco);
        DibujarMalla(shader, mallaCono, glm::scale(glm::translate(b, glm::vec3(0.0f, 5.0f, 0.0f)), glm::vec3(4.4f, 5.0f, 4.4f)), texCopa[t.tipo]);
        DibujarMalla(shader, mallaCono, glm::scale(glm::translate(b, glm::vec3(0.0f, 7.6f, 0.0f)), glm::vec3(3.2f, 4.2f, 3.2f)), texCopa[t.tipo]);
    }
}
