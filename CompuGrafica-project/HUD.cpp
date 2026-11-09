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

void DibujarHUDJuego(int W, int H, int modoCamara)
{
    // ----- Cronometro (arriba a la izquierda) -----
    HUDRect(20, 20, 330, 150, FONDO);
    FranjaPapelPicado(20, 15, 330);
    HUDTexto("VUELTA " + std::to_string(juego.vuelta), 35, 35, 21, GRIS);
    HUDTexto(FormatoTiempo(juego.tiempoVuelta), 35, 66, 42, BLANCO);
    HUDTexto("ULTIMA " + (juego.ultimaVuelta > 0 ? FormatoTiempo(juego.ultimaVuelta) : std::string("-:--.--")), 35, 122, 14, GRIS);
    HUDTexto("MEJOR  " + (juego.mejorVuelta > 0 ? FormatoTiempo(juego.mejorVuelta) : std::string("-:--.--")), 35, 145, 14, MORADO);

    // ----- Checkpoints y rivales (arriba a la derecha) -----
    HUDRect(W - 290.0f, 20, 270, 100, FONDO);
    FranjaPapelPicado(W - 290.0f, 15, 270);
    int cp = glm::min(juego.sigCheckpoint - 1, NUM_CHECKPOINTS - 1);
    HUDTexto("CHECKPOINT " + std::to_string(cp) + "/" + std::to_string(NUM_CHECKPOINTS - 1), W - 275.0f, 35, 16, BLANCO);
    // barra de avance de la vuelta
    float avance = glm::clamp(juego.jugador.dist / SplineLongitud(), 0.0f, 1.0f);
    HUDRect(W - 275.0f, 62, 240, 8, glm::vec4(1, 1, 1, 0.25f));
    HUDRect(W - 275.0f, 62, 240 * avance, 8, AMARILLO);
    HUDTexto("RIVALES " + std::to_string(RivalesActivos()) + "/" + std::to_string(NUM_RIVALES), W - 275.0f, 85, 16, GRIS);

    // ----- Velocidad y DRS (abajo a la derecha) -----
    HUDRect(W - 250.0f, H - 140.0f, 230, 120, FONDO);
    FranjaPapelPicado(W - 250.0f, H - 145.0f, 230);
    std::string vel = std::to_string((int)std::round(VelocidadKmh()));
    HUDTexto(vel, W - 40.0f - HUDAnchoTexto(vel, 56) - 70, H - 125.0f, 56, BLANCO);
    HUDTexto("KM/H", W - 95.0f, H - 85.0f, 14, GRIS);
    // DRS: gris = no disponible, contorno verde = disponible, relleno verde = abierto
    glm::vec4 colDRS = juego.drsAbierto ? VERDE : (juego.drsDisponible ? VERDE : glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
    float bx = W - 235.0f, by = H - 55.0f;
    if (juego.drsAbierto) HUDRect(bx, by, 80, 28, colDRS);
    else
    {
        HUDRect(bx, by, 80, 3, colDRS); HUDRect(bx, by + 25, 80, 3, colDRS);
        HUDRect(bx, by, 3, 28, colDRS); HUDRect(bx + 77, by, 3, 28, colDRS);
    }
    HUDTexto("DRS", bx + 17, by + 7, 14, juego.drsAbierto ? glm::vec4(0, 0, 0, 1) : colDRS);

    // ----- Camara y controles (abajo a la izquierda) -----
    const char* nombres[3] = { "CAMARA LIBRE", "TERCERA PERSONA", "ONBOARD" };
    HUDRect(20, H - 75.0f, 640, 55, FONDO);
    HUDTexto(std::string("C: ") + nombres[modoCamara], 35, H - 65.0f, 14, AMARILLO);
    if (modoCamara == 0)
        HUDTexto("WASD+MOUSE MOVER  SHIFT RAPIDO  C MANEJAR", 35, H - 42.0f, 11, GRIS);
    else
        HUDTexto("W ACELERAR  S FRENAR  A/D LADO  R DRS  T REINICIAR", 35, H - 42.0f, 11, GRIS);

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
