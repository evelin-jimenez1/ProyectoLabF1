// =============================================================
//  HUD.cpp
//  Cada letra es una matriz de 5 x 7 puntos; cada punto encendido
//  se convierte en un cuadrito (2 triangulos). Todos los cuadritos
//  de un cuadro se juntan en un solo VBO y se dibujan de una vez.
// =============================================================
#include "HUD.h"
#include "Shader.h"
#include "Juego.h"
#include "ZonaNeumaticos.h"
#include "Spline.h"

#include <vector>
#include <map>
#include <cmath>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static Shader* shaderHUD = nullptr;
static GLuint VAO, VBO;
static std::vector<float> vertices;   // x, y, r, g, b, a

// ---------------- Fuente 5x7 ----------------
// Cada letra: 7 filas de 5 caracteres ('#' = punto encendido)
static std::map<char, std::vector<std::string>> fuente = {
    {'0', {" ### ", "#   #", "#  ##", "# # #", "##  #", "#   #", " ### "}},
    {'1', {"  #  ", " ##  ", "  #  ", "  #  ", "  #  ", "  #  ", " ### "}},
    {'2', {" ### ", "#   #", "    #", "   # ", "  #  ", " #   ", "#####"}},
    {'3', {"#####", "   # ", "  #  ", "   # ", "    #", "#   #", " ### "}},
    {'4', {"   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # "}},
    {'5', {"#####", "#    ", "#### ", "    #", "    #", "#   #", " ### "}},
    {'6', {"  ## ", " #   ", "#    ", "#### ", "#   #", "#   #", " ### "}},
    {'7', {"#####", "    #", "   # ", "  #  ", " #   ", " #   ", " #   "}},
    {'8', {" ### ", "#   #", "#   #", " ### ", "#   #", "#   #", " ### "}},
    {'9', {" ### ", "#   #", "#   #", " ####", "    #", "   # ", " ##  "}},
    {'A', {" ### ", "#   #", "#   #", "#####", "#   #", "#   #", "#   #"}},
    {'B', {"#### ", "#   #", "#   #", "#### ", "#   #", "#   #", "#### "}},
    {'C', {" ### ", "#   #", "#    ", "#    ", "#    ", "#   #", " ### "}},
    {'D', {"#### ", "#   #", "#   #", "#   #", "#   #", "#   #", "#### "}},
    {'E', {"#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#####"}},
    {'F', {"#####", "#    ", "#    ", "#### ", "#    ", "#    ", "#    "}},
    {'G', {" ### ", "#   #", "#    ", "# ###", "#   #", "#   #", " ####"}},
    {'H', {"#   #", "#   #", "#   #", "#####", "#   #", "#   #", "#   #"}},
    {'I', {" ### ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", " ### "}},
    {'J', {"  ###", "   # ", "   # ", "   # ", "   # ", "#  # ", " ##  "}},
    {'K', {"#   #", "#  # ", "# #  ", "##   ", "# #  ", "#  # ", "#   #"}},
    {'L', {"#    ", "#    ", "#    ", "#    ", "#    ", "#    ", "#####"}},
    {'M', {"#   #", "## ##", "# # #", "# # #", "#   #", "#   #", "#   #"}},
    {'N', {"#   #", "#   #", "##  #", "# # #", "#  ##", "#   #", "#   #"}},
    {'O', {" ### ", "#   #", "#   #", "#   #", "#   #", "#   #", " ### "}},
    {'P', {"#### ", "#   #", "#   #", "#### ", "#    ", "#    ", "#    "}},
    {'Q', {" ### ", "#   #", "#   #", "#   #", "# # #", "#  # ", " ## #"}},
    {'R', {"#### ", "#   #", "#   #", "#### ", "# #  ", "#  # ", "#   #"}},
    {'S', {" ####", "#    ", "#    ", " ### ", "    #", "    #", "#### "}},
    {'T', {"#####", "  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "  #  "}},
    {'U', {"#   #", "#   #", "#   #", "#   #", "#   #", "#   #", " ### "}},
    {'V', {"#   #", "#   #", "#   #", "#   #", "#   #", " # # ", "  #  "}},
    {'W', {"#   #", "#   #", "#   #", "# # #", "# # #", "# # #", " # # "}},
    {'X', {"#   #", "#   #", " # # ", "  #  ", " # # ", "#   #", "#   #"}},
    {'Y', {"#   #", "#   #", " # # ", "  #  ", "  #  ", "  #  ", "  #  "}},
    {'Z', {"#####", "    #", "   # ", "  #  ", " #   ", "#    ", "#####"}},
    {':', {"     ", " ##  ", " ##  ", "     ", " ##  ", " ##  ", "     "}},
    {'.', {"     ", "     ", "     ", "     ", "     ", " ##  ", " ##  "}},
    {'/', {"    #", "    #", "   # ", "  #  ", " #   ", "#    ", "#    "}},
    {'-', {"     ", "     ", "     ", "#####", "     ", "     ", "     "}},
    {'+', {"     ", "  #  ", "  #  ", "#####", "  #  ", "  #  ", "     "}},
    {'!', {"  #  ", "  #  ", "  #  ", "  #  ", "  #  ", "     ", "  #  "}},
    {'(', {"   # ", "  #  ", " #   ", " #   ", " #   ", "  #  ", "   # "}},
    {')', {" #   ", "  #  ", "   # ", "   # ", "   # ", "  #  ", " #   "}},
    {'=', {"     ", "     ", "#####", "     ", "#####", "     ", "     "}},
    {'>', {" #   ", "  #  ", "   # ", "    #", "   # ", "  #  ", " #   "}},
    {'<', {"   # ", "  #  ", " #   ", "#    ", " #   ", "  #  ", "   # "}},
};


void HUDInicializar()
{
    shaderHUD = new Shader("Shader/hud.vs", "Shader/hud.frag");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}


void HUDRect(float x, float y, float w, float h, glm::vec4 c)
{
    float q[6][2] = { {x, y}, {x + w, y}, {x + w, y + h}, {x, y}, {x + w, y + h}, {x, y + h} };
    for (auto& p : q)
        vertices.insert(vertices.end(), { p[0], p[1], c.r, c.g, c.b, c.a });
}

float HUDAnchoTexto(const std::string& texto, float alto)
{
    float punto = alto / 7.0f;
    return texto.size() * 6.0f * punto - punto;   // 5 puntos + 1 de espacio por letra
}

void HUDTexto(const std::string& texto, float x, float y, float alto, glm::vec4 color)
{
    float punto = alto / 7.0f;
    float tam = punto * 0.86f;           // un poco de separacion entre puntos (efecto LED)
    for (char ch : texto)
    {
        char c = (char)toupper((unsigned char)ch);
        auto it = fuente.find(c);
        if (it != fuente.end())
            for (int fila = 0; fila < 7; fila++)
                for (int col = 0; col < 5; col++)
                    if (it->second[fila][col] == '#')
                        HUDRect(x + col * punto, y + fila * punto, tam, tam, color);
        x += 6.0f * punto;
    }
}

void HUDDibujar(const glm::mat4& mvp, bool enPantalla)
{
    if (vertices.empty()) return;
    shaderHUD->Use();
    glUniformMatrix4fv(glGetUniformLocation(shaderHUD->Program, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    if (enPantalla) glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / 6));
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    if (enPantalla) glEnable(GL_DEPTH_TEST);
    vertices.clear();
}


// =============================================================
//  HUD del juego
// =============================================================
// Paleta de Dia de Muertos (papel picado y cempasuchil)
static const glm::vec4 BLANCO(1.0f, 0.97f, 0.92f, 1.0f);
static const glm::vec4 GRIS(0.95f, 0.78f, 0.88f, 1.0f);        // rosa palido
static const glm::vec4 AMARILLO(1.0f, 0.62f, 0.08f, 1.0f);     // naranja cempasuchil
static const glm::vec4 VERDE(0.2f, 0.95f, 0.75f, 1.0f);        // turquesa
static const glm::vec4 MORADO(1.0f, 0.30f, 0.70f, 1.0f);       // rosa mexicano
static const glm::vec4 FONDO(0.13f, 0.03f, 0.20f, 0.72f);      // morado oscuro

// Franja de colores de papel picado arriba de cada panel
static void FranjaPapelPicado(float x, float y, float w)
{
    const glm::vec4 colores[6] = {
        glm::vec4(0.95f, 0.10f, 0.55f, 1.0f), glm::vec4(1.0f, 0.55f, 0.05f, 1.0f),
        glm::vec4(0.55f, 0.20f, 0.85f, 1.0f), glm::vec4(0.10f, 0.75f, 0.80f, 1.0f),
        glm::vec4(1.0f, 0.85f, 0.10f, 1.0f), glm::vec4(0.20f, 0.70f, 0.25f, 1.0f) };
    float ancho = w / 6.0f;
    for (int i = 0; i < 6; i++)
        HUDRect(x + i * ancho, y, ancho, 5.0f, colores[i]);
}

// ---------------- Figuras para el HUD circular y el minimapa ----------------
// Angulos en grados: 0 = derecha, y crecen en sentido de las manecillas (la pantalla tiene y hacia abajo)
static const float PI_HUD = 3.14159265f;

static void HUDLinea(float x0, float y0, float x1, float y1, float grosor, glm::vec4 c)
{
    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-4f) return;
    float nx = -dy / len * grosor * 0.5f, ny = dx / len * grosor * 0.5f;
    float q[6][2] = { {x0 + nx, y0 + ny}, {x1 + nx, y1 + ny}, {x1 - nx, y1 - ny},
                      {x0 + nx, y0 + ny}, {x1 - nx, y1 - ny}, {x0 - nx, y0 - ny} };
    for (auto& p : q) vertices.insert(vertices.end(), { p[0], p[1], c.r, c.g, c.b, c.a });
}

// Pedazo de anillo entre los radios r0 y r1
static void HUDArco(float cx, float cy, float r0, float r1, float a0, float a1, glm::vec4 c)
{
    int pasos = glm::max(1, (int)std::ceil(std::fabs(a1 - a0) / 4.0f));
    for (int i = 0; i < pasos; i++)
    {
        float t0 = glm::radians(a0 + (a1 - a0) * i / pasos), t1 = glm::radians(a0 + (a1 - a0) * (i + 1) / pasos);
        float q[6][2] = { {cx + r0 * std::cos(t0), cy + r0 * std::sin(t0)}, {cx + r1 * std::cos(t0), cy + r1 * std::sin(t0)},
                          {cx + r1 * std::cos(t1), cy + r1 * std::sin(t1)},
                          {cx + r0 * std::cos(t0), cy + r0 * std::sin(t0)}, {cx + r1 * std::cos(t1), cy + r1 * std::sin(t1)},
                          {cx + r0 * std::cos(t1), cy + r0 * std::sin(t1)} };
        for (auto& p : q) vertices.insert(vertices.end(), { p[0], p[1], c.r, c.g, c.b, c.a });
    }
}

static void HUDDisco(float cx, float cy, float r, glm::vec4 c) { HUDArco(cx, cy, 0.0f, r, 0.0f, 360.0f, c); }

// Color del velocimetro: verde -> amarillo -> rojo segun la posicion en el arco (0..1)
static glm::vec4 ColorVelocimetro(float t)
{
    glm::vec3 verde(0.20f, 0.95f, 0.50f), amarillo(1.0f, 0.85f, 0.10f), rojo(1.0f, 0.22f, 0.25f);
    glm::vec3 c = t < 0.6f ? glm::mix(verde, amarillo, t / 0.6f) : glm::mix(amarillo, rojo, (t - 0.6f) / 0.4f);
    return glm::vec4(c, 1.0f);
}

// ---------------- Velocimetro circular con RPM, marcha y DRS ----------------
static void DibujarVelocimetro(int W, int H, const glm::vec4& FONDO)
{
    const float R = 100.0f;
    float cx = W - R - 40.0f, cy = H - R - 30.0f;
    float kmh = VelocidadKmh();
    const float KMH_MAX = 190.0f;
    float frac = glm::clamp(kmh / KMH_MAX, 0.0f, 1.0f);

    HUDDisco(cx, cy, R + 10.0f, glm::vec4(FONDO.r, FONDO.g, FONDO.b, 0.85f));
    // arco de fondo y arco de velocidad (270 grados, con la abertura abajo)
    const float A0 = 135.0f, SWEEP = 270.0f;
    HUDArco(cx, cy, R - 18.0f, R - 4.0f, A0, A0 + SWEEP, glm::vec4(1, 1, 1, 0.16f));
    int segs = (int)(frac * 60.0f);
    for (int i = 0; i < segs; i++)
    {
        float t = i / 60.0f;
        glm::vec4 col = juego.drsAbierto ? glm::vec4(0.65f, 0.35f, 1.0f, 1.0f) : ColorVelocimetro(t);
        HUDArco(cx, cy, R - 18.0f, R - 4.0f, A0 + SWEEP * i / 60.0f, A0 + SWEEP * (i + 1) / 60.0f + 0.6f, col);
    }

    // marcha y RPM (simulados a partir de la velocidad)
    int marcha = 1 + glm::min(6, (int)(kmh / 24.0f));
    float enMarcha = glm::clamp((kmh - (marcha - 1) * 24.0f) / 24.0f, 0.0f, 1.0f);
    int rpm = kmh < 1.0f ? 4000 : 6000 + (int)(6000.0f * enMarcha);
    // arco delgado interior = RPM
    HUDArco(cx, cy, R - 30.0f, R - 25.0f, A0, A0 + SWEEP, glm::vec4(1, 1, 1, 0.12f));
    HUDArco(cx, cy, R - 30.0f, R - 25.0f, A0, A0 + SWEEP * glm::clamp((rpm - 4000) / 9000.0f, 0.0f, 1.0f), glm::vec4(0.75f, 0.35f, 1.0f, 1.0f));

    std::string vel = std::to_string((int)std::round(kmh));
    HUDTexto(vel, cx - HUDAnchoTexto(vel, 46) * 0.5f, cy - 44.0f, 46, BLANCO);
    HUDTexto("KM/H", cx - HUDAnchoTexto("KM/H", 12) * 0.5f, cy + 10.0f, 12, GRIS);
    // marcha dentro de un circulo
    HUDDisco(cx, cy + 40.0f, 14.0f, glm::vec4(1, 1, 1, 0.22f));
    HUDTexto(std::to_string(marcha), cx - HUDAnchoTexto("0", 16) * 0.5f, cy + 40.0f - 8.0f, 16, BLANCO);
    std::string r = "RPM " + std::to_string(rpm);
    HUDTexto(r, cx - HUDAnchoTexto(r, 10) * 0.5f, cy + 64.0f, 10, GRIS);

    // DRS: gris = no disponible, contorno = disponible, relleno = abierto
    glm::vec4 colDRS = juego.drsAbierto ? VERDE : (juego.drsDisponible ? VERDE : glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
    float bx = cx - R - 78.0f, by = cy + R - 36.0f;
    if (juego.drsAbierto) HUDRect(bx, by, 64, 28, colDRS);
    else
    {
        HUDRect(bx, by, 64, 3, colDRS); HUDRect(bx, by + 25, 64, 3, colDRS);
        HUDRect(bx, by, 3, 28, colDRS); HUDRect(bx + 61, by, 3, 28, colDRS);
    }
    HUDTexto("DRS", bx + 9, by + 7, 14, juego.drsAbierto ? glm::vec4(0, 0, 0, 1) : colDRS);
}

// ---------------- Minimapa de la pista con los carros ----------------
static void DibujarMinimapa(int H, const glm::vec4& FONDO)
{
    static std::vector<glm::vec2> pista;     // puntos de la spline en coordenadas del mundo (x, z)
    static float minX, minZ, escala;
    const float PX = 20.0f, PW = 200.0f, PH = 150.0f;
    float PY = H - 75.0f - PH - 12.0f;

    if (pista.empty())
    {
        float L = SplineLongitud(), maxX, maxZ;
        minX = minZ = 1e9f; maxX = maxZ = -1e9f;
        for (float s = 0.0f; s < L; s += 8.0f)
        {
            glm::vec3 p = SplinePosicion(s);
            pista.push_back(glm::vec2(p.x, p.z));
            minX = glm::min(minX, p.x); maxX = glm::max(maxX, p.x);
            minZ = glm::min(minZ, p.z); maxZ = glm::max(maxZ, p.z);
        }
        escala = glm::min((PW - 24.0f) / (maxX - minX), (PH - 24.0f) / (maxZ - minZ));
        // se centra en el panel
        minX -= ((PW - 24.0f) / escala - (maxX - minX)) * 0.5f;
        minZ -= ((PH - 24.0f) / escala - (maxZ - minZ)) * 0.5f;
    }
    auto aPantalla = [&](float x, float z) { return glm::vec2(PX + 12.0f + (x - minX) * escala, PY + 12.0f + (z - minZ) * escala); };

    HUDRect(PX, PY, PW, PH, FONDO);
    for (size_t i = 0; i < pista.size(); i++)
    {
        glm::vec2 a = aPantalla(pista[i].x, pista[i].y), b = aPantalla(pista[(i + 1) % pista.size()].x, pista[(i + 1) % pista.size()].y);
        HUDLinea(a.x, a.y, b.x, b.y, 4.0f, glm::vec4(0.05f, 0.05f, 0.08f, 0.9f));   // contorno
    }
    for (size_t i = 0; i < pista.size(); i++)
    {
        glm::vec2 a = aPantalla(pista[i].x, pista[i].y), b = aPantalla(pista[(i + 1) % pista.size()].x, pista[(i + 1) % pista.size()].y);
        HUDLinea(a.x, a.y, b.x, b.y, 2.2f, glm::vec4(0.92f, 0.92f, 0.96f, 1.0f));
    }
    // meta
    glm::vec3 m = SplinePosicion(0.0f);
    glm::vec2 pm = aPantalla(m.x, m.z);
    HUDRect(pm.x - 3, pm.y - 6, 6, 12, glm::vec4(1, 1, 1, 1));
    HUDRect(pm.x - 3, pm.y - 6, 3, 6, glm::vec4(0, 0, 0, 1)); HUDRect(pm.x, pm.y, 3, 6, glm::vec4(0, 0, 0, 1));

    // rivales y jugador
    for (const Rival& r : juego.rivales)
        if (r.activo)
        {
            glm::vec3 p = SplinePuntoPista(r.carro.dist, r.carro.lateral);
            glm::vec2 q = aPantalla(p.x, p.z);
            HUDDisco(q.x, q.y, 4.0f, glm::vec4(r.color, 1.0f));
        }
    glm::vec3 p = SplinePuntoPista(juego.jugador.dist, juego.jugador.lateral);
    glm::vec2 q = aPantalla(p.x, p.z);
    HUDDisco(q.x, q.y, 6.5f, glm::vec4(1, 1, 1, 1));
    HUDDisco(q.x, q.y, 4.8f, glm::vec4(1.0f, 0.85f, 0.10f, 1.0f));
}

void DibujarHUDJuego(int W, int H, int modoCamara)
{
    // ----- Cronometro (arriba a la izquierda) -----
    bool variasVueltas = VUELTAS_CARRERA > 1;
    HUDRect(20, 20, 330, variasVueltas ? 178.0f : 150.0f, FONDO);
    FranjaPapelPicado(20, 15, 330);
    HUDTexto(VUELTAS_CARRERA == 1 ? std::string("TRAMO") : "VUELTA " + std::to_string(glm::min(juego.vuelta, VUELTAS_CARRERA)) + "/" + std::to_string(VUELTAS_CARRERA), 35, 35, 21, GRIS);
    HUDTexto(FormatoTiempo(juego.tiempoVuelta), 35, 66, 42, BLANCO);
    HUDTexto("ULTIMA " + (juego.ultimaVuelta > 0 ? FormatoTiempo(juego.ultimaVuelta) : std::string("-:--.--")), 35, 122, 14, GRIS);
    HUDTexto("MEJOR  " + (juego.mejorVuelta > 0 ? FormatoTiempo(juego.mejorVuelta) : std::string("-:--.--")), 35, 145, 14, MORADO);
    if (variasVueltas)   // contrarreloj de varias vueltas: tiempo total acumulado
    {
        float total = juego.tiempoTotal + (juego.estado == CORRIENDO && juego.vuelta <= VUELTAS_CARRERA ? juego.tiempoVuelta : 0.0f);
        HUDTexto("TOTAL  " + FormatoTiempo(total), 35, 168, 14, BLANCO);
        HUDTexto("CONTRARRELOJ", 350.0f - 15.0f - HUDAnchoTexto("CONTRARRELOJ", 11), 40, 11, AMARILLO);
    }

    // ----- Sectores (arriba a la derecha): morado = tu mejor tiempo -----
    const glm::vec4 PURPURA(0.75f, 0.35f, 1.0f, 1.0f);
    HUDRect(W - 290.0f, 20, 270, 165, FONDO);
    FranjaPapelPicado(W - 290.0f, 15, 270);
    HUDTexto("SECTORES", W - 275.0f, 35, 16, BLANCO);
    int sectorActual = glm::min(2, (juego.sigCheckpoint - 1) / 2);
    for (int k = 0; k < 3; k++)
    {
        float fy = 62.0f + k * 26.0f;
        bool hay = juego.sector[k] > 0.0f;
        glm::vec4 col = !hay ? GRIS : (juego.sectorRecord[k] ? PURPURA : BLANCO);
        if (k == sectorActual && juego.estado == CORRIENDO) HUDRect(W - 283.0f, fy + 2, 4, 14, AMARILLO);   // sector en curso
        HUDTexto("S" + std::to_string(k + 1), W - 275.0f, fy, 16, col);
        std::string t = hay ? FormatoTiempo(juego.sector[k]) : "-:--.--";
        HUDTexto(t, W - 35.0f - HUDAnchoTexto(t, 16), fy, 16, col);
    }
    // barra de avance del tramo y rivales
    float avance = glm::clamp(juego.jugador.dist / SplineLongitud(), 0.0f, 1.0f);
    HUDRect(W - 275.0f, 148, 240, 8, glm::vec4(1, 1, 1, 0.25f));
    HUDRect(W - 275.0f, 148, 240 * avance, 8, AMARILLO);
    HUDTexto("RIVALES " + std::to_string(RivalesActivos()) + "/" + std::to_string(NUM_RIVALES), W - 275.0f, 164, 12, GRIS);

    // ----- Velocimetro circular (abajo a la derecha) y minimapa (abajo a la izquierda) -----
    DibujarVelocimetro(W, H, FONDO);
    DibujarMinimapa(H, FONDO);

    // ----- Camara y controles (abajo a la izquierda) -----
    const char* nombres[3] = { "CAMARA LIBRE", "TERCERA PERSONA", "ONBOARD" };
    HUDRect(20, H - 75.0f, 840, 55, FONDO);
    HUDTexto(std::string("C: ") + nombres[modoCamara], 35, H - 65.0f, 14, AMARILLO);
    if (modoCamara == 0)
        HUDTexto("WASD+MOUSE MOVER  SHIFT RAPIDO  C MANEJAR", 35, H - 42.0f, 11, GRIS);
    else
        HUDTexto("W ACELERAR  S FRENAR  A/D LADO  R DRS  T REINICIAR  N AMBIENTE  F LUCES", 35, H - 42.0f, 11, GRIS);

    // ----- Mensaje al centro -----
    if (juego.tiempoMensaje > 0.0f && !juego.mensaje.empty())
    {
        float alto = 30.0f;
        float w = HUDAnchoTexto(juego.mensaje, alto);
        float x = (W - w) * 0.5f, y = H * 0.30f;
        HUDRect(x - 20, y - 15, w + 40, alto + 30, FONDO);
        HUDTexto(juego.mensaje, x, y, alto, AMARILLO);
    }

    glm::mat4 orto = glm::ortho(0.0f, (float)W, (float)H, 0.0f);   // y hacia abajo
    HUDDibujar(orto, true);
}


// =============================================================
//  Tiempo en el marcador del Foro Sol
//  Mismos numeros que DibujarMarcador (CargaModelos.cpp).
//  El letrero del .obj esta en el plano z = -1.40 viendo hacia -Z,
//  de x = -7.46 a 0.42 y de y = 9.45 a 11.45. El tablero del tiempo
//  se pone justo debajo, por las dos caras.
// =============================================================
void DibujarTiempoMarcador(const glm::mat4& projView)
{
    glm::mat4 base = glm::translate(glm::mat4(1.0f), glm::vec3(-190.0f, 0.0f, -98.0f));
    base = glm::rotate(base, glm::radians(95.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    std::string tiempo = FormatoTiempo(juego.tiempoVuelta);
    std::string mejor = "MEJOR " + (juego.mejorVuelta > 0 ? FormatoTiempo(juego.mejorVuelta) : std::string("-:--.--"));

    for (int cara = 0; cara < 2; cara++)
    {
        // Plano del tablero: x a la derecha, y hacia ABAJO (como en pantalla)
        glm::mat4 m = base;
        if (cara == 0)   // frente (viendo hacia -Z, igual que el letrero)
        {
            m = glm::translate(m, glm::vec3(-0.6f, 9.30f, -1.46f));
            m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0, 1, 0));
        }
        else             // reverso (viendo hacia +Z)
            m = glm::translate(m, glm::vec3(-7.0f, 9.30f, -0.33f));
        m = glm::scale(m, glm::vec3(1.0f, -1.0f, 1.0f));   // y hacia abajo

        // Fondo negro y textos LED (las unidades son metros)
        HUDRect(0.0f, 0.0f, 6.4f, 1.55f, glm::vec4(0.02f, 0.02f, 0.02f, 1.0f));
        HUDDibujar(projView * m, false);
        glm::mat4 enfrente = glm::translate(m, glm::vec3(0.0f, 0.0f, 0.02f));
        HUDTexto("TIEMPO", 0.25f, 0.15f, 0.35f, glm::vec4(1.0f, 0.55f, 0.1f, 1.0f));
        HUDTexto(tiempo, 6.2f - HUDAnchoTexto(tiempo, 0.62f), 0.15f, 0.62f, glm::vec4(1.0f, 0.85f, 0.1f, 1.0f));
        HUDTexto(mejor, 0.25f, 1.0f, 0.35f, glm::vec4(0.8f, 0.45f, 1.0f, 1.0f));
        HUDDibujar(projView * enfrente, false);
    }
}


// =============================================================
//  Letrero de la zona de neumaticos (sobre dos postes)
// =============================================================
void DibujarLetreroZona(const glm::mat4& projView)
{
    glm::mat4 zona = MatrizZonaNeumaticos();
    // En la orilla de la zona, mirando hacia la pista (-X local)
    glm::mat4 m = glm::translate(zona, glm::vec3(1.5f, 3.2f, 7.0f));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    m = glm::scale(m, glm::vec3(1.0f, -1.0f, 1.0f));

    const std::string t1 = "ZONA DE", t2 = "NEUMATICOS";
    HUDRect(-0.2f, -0.2f, 6.4f, 2.0f, glm::vec4(0.40f, 0.08f, 0.45f, 1.0f));   // fondo morado
    HUDRect(-0.2f, 1.8f, 0.15f, 1.3f, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));      // postes
    HUDRect(6.05f, 1.8f, 0.15f, 1.3f, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
    HUDDibujar(projView * m, false);
    glm::mat4 enfrente = glm::translate(m, glm::vec3(0.0f, 0.0f, 0.02f));
    HUDTexto(t1, (6.0f - HUDAnchoTexto(t1, 0.6f)) * 0.5f, 0.1f, 0.6f, glm::vec4(1.0f));
    HUDTexto(t2, (6.0f - HUDAnchoTexto(t2, 0.6f)) * 0.5f, 0.9f, 0.6f, glm::vec4(1.0f, 0.85f, 0.1f, 1.0f));
    HUDDibujar(projView * enfrente, false);
}
