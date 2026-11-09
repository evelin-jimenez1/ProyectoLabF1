// =============================================================
//  Proyecto Final - Autodromo Hermanos Rodriguez (F1)
//  main.cpp: ventana, camara y ciclo de render.
//    - Las texturas estan en      Texturas.h / Texturas.cpp
//    - Los modelos estan en       CargaModelos.h / CargaModelos.cpp
//    - La spline de la pista en   Spline.h / Spline.cpp
//
//    - Mecanicas de juego en       Juego.h / Juego.cpp
//    - HUD y marcador en           HUD.h / HUD.cpp
//    - Piloto en                   Piloto.h / Piloto.cpp
//    - Zona de neumaticos en       ZonaNeumaticos.h / .cpp
//    - Keyframes del DRS en        KeyFramesDRS.h / .cpp
//    - Dia de Muertos en           DiaDeMuertos.h / .cpp
//    - Sonido en                   Audio.h / Audio.cpp
//
//  Controles:
//    C           = cambiar camara (tercera persona -> onboard -> libre)
//    Camara libre: W A S D = mover | Shift = rapido | mouse = mirar
//    Con el carro: W = acelerar | S = frenar | A D = moverse a los lados
//    R = abrir / cerrar DRS | T = reiniciar la vuelta
//    F = encender postes | L = ver spline | ESC = salir
// =============================================================

// Std. Includes
#include <string>
#include <iostream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Archivos propios del proyecto
#include "Texturas.h"
#include "CargaModelos.h"
#include "Spline.h"
#include "Primitivas.h"
#include "Piloto.h"
#include "ZonaNeumaticos.h"
#include "Juego.h"
#include "HUD.h"
#include "Audio.h"
#include "KeyFramesDRS.h"
#include "DiaDeMuertos.h"


// Properties
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// ---------- Repeticion de las texturas ----------
// Pista: 488 x 354 en una sola imagen -> 80 x 60 = una imagen cada ~6 m
const float REPETIR_ASFALTO_U = 80.0f;
const float REPETIR_ASFALTO_V = 60.0f;
// Pasto: plano de 1000 x 1000 -> 100 = una imagen cada 10 m
const float REPETIR_PASTO = 100.0f;

// Mostrar la linea blanca de la spline (tecla L la prende/apaga)
bool verSpline = true;

// Camera (arriba y alejada, porque la pista es grande)
Camera camera(glm::vec3(0.0f, 100.0f, 250.0f));
bool keys[1024];
GLfloat lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// ---------- Camaras ----------
// 0 = libre, 1 = tercera persona (detras del carro), 2 = onboard (cabina)
int modoCamara = 1;


// Video de la pantalla (animacion por cuadros)
const int NUM_CUADROS = 50;
const float CUADROS_POR_SEG = 12.0f;
GLuint texPantalla[NUM_CUADROS];
int cuadroActual = 0;
float tiempoCuadro = 0.0f;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Tecpro: x, z, rotacion en Y (por fuera de cada curva)
float tecproCurva1[11][3] = {
    { 215.86f, -125.18f,  -6.5f }, { 219.99f, -124.55f, -10.9f }, { 223.99f, -123.50f, -18.4f },
    { 227.94f, -121.93f, -24.9f }, { 231.61f, -119.73f, -37.3f }, { 234.55f, -116.75f, -53.4f },
    { 236.53f, -112.93f, -71.3f }, { 237.39f, -108.71f, -86.2f }, { 237.40f, -104.50f, -93.5f },
    { 237.11f, -100.28f, -94.3f }, { 236.71f,  -96.10f, -96.6f },
};
float tecproHorquilla[14][3] = {
    { 138.90f, 170.13f, -143.2f }, { 135.54f, 172.64f, -143.3f }, { 132.21f, 175.22f, -141.3f },
    { 128.72f, 177.74f, -146.9f }, { 125.04f, 179.83f, -154.3f }, { 121.20f, 181.40f, -161.1f },
    { 117.10f, 182.33f, -173.5f }, { 112.73f, 182.15f,  169.6f }, { 108.63f, 180.80f,  153.1f },
    { 105.10f, 178.45f,  140.0f }, { 102.09f, 175.36f,  128.4f }, {  99.87f, 171.83f,  115.8f },
    {  98.56f, 167.87f,  101.2f }, {  98.14f, 163.63f,   90.0f },
};
float tecproForoSol[10][3] = {
    { -167.41f, -63.28f, 171.9f }, { -171.60f, -64.13f, 165.1f }, { -175.65f, -65.74f, 151.6f },
    { -179.25f, -68.28f, 138.0f }, { -182.14f, -71.41f, 127.1f }, { -184.37f, -74.99f, 117.1f },
    { -185.93f, -79.06f, 104.8f }, { -186.66f, -83.28f,  94.5f }, { -186.94f, -87.42f,  93.3f },
    { -187.18f, -91.53f,  93.3f },
};
float tecproS[10][3] = {
    {  9.11f,   7.38f, 133.4f }, {  6.37f,   3.99f, 124.2f }, {  4.18f,   0.35f, 117.8f },
    {  2.49f,  -3.46f, 110.1f }, {  1.15f,  -7.51f, 106.5f }, { -0.04f, -11.57f, 106.0f },
    { -1.33f, -15.49f, 110.8f }, { -3.14f, -19.16f, 121.7f }, { -5.71f, -22.34f, 136.0f },
    { -8.94f, -24.89f, 147.4f },
};

// ---------- Spotlights de los postes ----------
// 2 focos por poste: abajo y arriba. Mismo orden que los DibujarPoste.
const int NUM_POSTES = 9;

glm::vec3 posFocoAbajo[NUM_POSTES] = {
    glm::vec3(-142.47f, 13.96f, -151.26f), glm::vec3(211.59f, 13.96f, -109.38f),
    glm::vec3(213.71f, 13.96f,  -88.64f), glm::vec3(224.96f, 13.96f,  -62.99f),
    glm::vec3(185.66f, 13.96f,    6.11f), glm::vec3(104.07f, 13.96f,  141.85f),
    glm::vec3(23.35f, 13.96f,   -8.64f), glm::vec3(-116.71f, 13.96f,  -77.44f),
    glm::vec3(-171.26f, 13.96f, -127.53f),
};
glm::vec3 dirFoco[NUM_POSTES] = {
    glm::vec3(0.000f, -0.819f, -0.574f), glm::vec3(0.000f, -0.819f, -0.574f),
    glm::vec3(0.539f, -0.819f,  0.196f), glm::vec3(0.440f, -0.819f,  0.369f),
    glm::vec3(0.287f, -0.819f,  0.497f), glm::vec3(0.574f, -0.819f,  0.000f),
    glm::vec3(-0.196f, -0.819f,  0.539f), glm::vec3(-0.100f, -0.819f,  0.565f),
    glm::vec3(-0.574f, -0.819f,  0.000f),
};
const float ALTURA_FOCO_ARRIBA = 1.34f;   // cuanto mas arriba esta el foco de arriba

// ---------- Animacion de encendido  ----------
// Keyframes del parpadeo donde 1 es prendido y 0 apagado
float patronParpadeo[10] = { 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 };
bool  lucesActivas = false;   // se activa con F
float tiempoLuces = 0.0f;

// Regresa la intensidad (0 o 1) de un grupo de focos segun el tiempo.
// inicio = segundo en que ese grupo empieza a parpadear.
float IntensidadGrupo(float t, float inicio)
{
    if (t < inicio) return 0.0f;                          // todavia apagado
    float tt = t - inicio;
    if (tt < 1.5f) return patronParpadeo[(int)(tt * 10.0f) % 10];  // 1.5 s parpadeando
    return 1.0f;                                          // encendido fijo
}

// Manda un foco al shader (focos[i])
void EnviarFoco(Shader& shader, int i, glm::vec3 pos, glm::vec3 dir, float intensidad)
{
    std::string n = "focos[" + std::to_string(i) + "].";
    glUniform3f(glGetUniformLocation(shader.Program, (n + "position").c_str()), pos.x, pos.y, pos.z);
    glUniform3f(glGetUniformLocation(shader.Program, (n + "direction").c_str()), dir.x, dir.y, dir.z);
    glUniform1f(glGetUniformLocation(shader.Program, (n + "cutOff").c_str()), glm::cos(glm::radians(21.0f)));
    glUniform1f(glGetUniformLocation(shader.Program, (n + "outerCutOff").c_str()), glm::cos(glm::radians(30.0f)));
    glUniform1f(glGetUniformLocation(shader.Program, (n + "constant").c_str()), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, (n + "linear").c_str()), 0.022f);
    glUniform1f(glGetUniformLocation(shader.Program, (n + "quadratic").c_str()), 0.0019f);
    glUniform3f(glGetUniformLocation(shader.Program, (n + "color").c_str()), 1.0f, 0.91f, 0.80f); // color de Blender
    glUniform1f(glGetUniformLocation(shader.Program, (n + "intensidad").c_str()), intensidad * 2.0f); // "5000 W"
}

int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto Final - Autodromo F1", nullptr, nullptr);
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Shader (textura que se repite + sombreado sencillo)
    Shader shader("Shader/pista.vs", "Shader/pista.frag");
    Shader shaderBandera("Shader/bandera.vs", "Shader/bandera.frag");

    // ---------- Modelos (CargaModelos.cpp) ----------
    CargarModelos();

    // ---------- Spline (Spline.cpp) ----------
    SplineInicializar();
    std::cout << "Longitud de la pista: " << SplineLongitud() << " m" << std::endl;
    int numPuntosSpline;
    GLuint vaoSpline = CrearLineaSpline(numPuntosSpline);
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");   // color blanco, de las practicas

    // ---------- Juego, piloto, utileria, Dia de Muertos, HUD y sonido ----------
    PrimitivasInicializar();
    PilotoInicializar();
    ZonaNeumaticosInicializar();
    DiaDeMuertosInicializar();
    JuegoInicializar();
    HUDInicializar();
    AudioInicializar();

    // ---------- Texturas (Texturas.cpp) ----------
    GLuint texAsfalto = CargarTextura("images/asfalto.png");
    GLuint texPasto = CargarTextura("images/pasto.png");

    //  Cuadros del video de la pantalla 
		// Carga las 50 imagenes (0001 a 0050) como texturas y los guard en arreglo
    for (int i = 0; i < NUM_CUADROS; i++) {
        std::string num = std::to_string(i + 1);
        while (num.length() < 4) num = "0" + num;      // 1 - "0001"
        std::string ruta = "images/pantalla/" + num + ".png";
        texPantalla[i] = CargarTextura(ruta.c_str());
    }

    glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 2000.0f);

    // Propiedades del Sol Luz Direccionaly puntual
    glm::vec3 lightPos(0.0f, 300.0f, 0.0f); // Sol alto en el centro del mapa
    glm::vec3 lightColor(1.0f, 0.95f, 0.8f); // Luz ligeramente amarilla/calida

    

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //  Animacion del video de la pantalla 
        // Va pasando al siguiente cuadro y al llegar al ultimo, reinicia.
        tiempoCuadro += deltaTime;
        if (tiempoCuadro >= 1.0f / CUADROS_POR_SEG) {
            tiempoCuadro = 0.0f;
            cuadroActual = (cuadroActual + 1) % NUM_CUADROS;   // al llegar al 50 reinicia
        }

        if (lucesActivas) tiempoLuces += deltaTime;
        float intAbajo = IntensidadGrupo(tiempoLuces, 0.0f);  // abajo: empieza en 0 s
        float intArriba = IntensidadGrupo(tiempoLuces, 3.0f);  // arriba: empieza a los 3 s

        glfwPollEvents();
        DoMovement();

        // ----- Juego: el carro solo se maneja en las camaras del carro -----
        Controles controles;
        if (modoCamara != 0)
        {
            controles.acelerar = keys[GLFW_KEY_W] || keys[GLFW_KEY_UP];
            controles.frenar = keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN];
            controles.izquierda = keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT];
            controles.derecha = keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT];
        }
        JuegoActualizar(deltaTime, controles);

        // ----- Calaveras: brincan cuando pasa el carro (maquina de estados) -----
        glm::mat4 baseCarro = MatrizCarroMundo(juego.jugador);
        AnimarCalaveras(deltaTime, glm::vec3(baseCarro[3]));

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);   // cielo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // ----- Camara segun el modo -----
        glm::mat4 view;
        glm::vec3 camPos;
        if (modoCamara == 1)
        {
            // Tercera persona / TV: detras y arriba del carro
            camPos = glm::vec3(baseCarro * glm::vec4(0.0f, 3.2f, 9.0f, 1.0f));
            glm::vec3 objetivo = glm::vec3(baseCarro * glm::vec4(0.0f, 1.0f, -4.0f, 1.0f));
            view = glm::lookAt(camPos, objetivo, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (modoCamara == 2)
        {
            // Onboard: en los ojos del piloto (por eso no se dibuja su cabeza)
            camPos = glm::vec3(baseCarro * glm::vec4(0.0f, 0.90f, -0.24f, 1.0f));
            glm::vec3 objetivo = glm::vec3(baseCarro * glm::vec4(0.0f, 0.75f, -20.0f, 1.0f));
            view = glm::lookAt(camPos, objetivo, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else
        {
            camPos = camera.GetPosition();
            view = camera.GetViewMatrix();
        }
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Enviar variables de Iluminacion
        glUniform3f(glGetUniformLocation(shader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.Program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camPos.x, camPos.y, camPos.z);

        // ----- Focos de los postes (se mandan cada cuadro por la animacion) -----
        for (int i = 0; i < NUM_POSTES; i++) {
            EnviarFoco(shader, i * 2, posFocoAbajo[i], dirFoco[i], intAbajo);
            EnviarFoco(shader, i * 2 + 1, posFocoAbajo[i] + glm::vec3(0.0f, ALTURA_FOCO_ARRIBA, 0.0f), dirFoco[i], intArriba);
        }
        // ----- Foco del letrero -----
        // Siempre encendido. Sigue al marcador (mismos numeros que DibujarMarcador)
        // y su atenuacion fuerte hace que solo alumbre el letrero, no el piso.
        glm::mat4 baseMarcador = glm::translate(glm::mat4(1.0f), glm::vec3(-190.0f, 0.0f, -98.0f));
        baseMarcador = glm::rotate(baseMarcador, glm::radians(95.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec3 posFocoLetrero = glm::vec3(baseMarcador * glm::vec4(-3.5195f, 13.2f, -3.604f, 1.0f)); // punto (w = 1)
        glm::vec3 dirFocoLetrero = glm::vec3(baseMarcador * glm::vec4(0.0f, -0.7966f, 0.6045f, 0.0f));  // direccion (w = 0)

        EnviarFoco(shader, 18, posFocoLetrero, dirFocoLetrero, 2.0f);

        glUniform1f(glGetUniformLocation(shader.Program, "focos[18].cutOff"), glm::cos(glm::radians(40.0f)));
        glUniform1f(glGetUniformLocation(shader.Program, "focos[18].outerCutOff"), glm::cos(glm::radians(50.0f)));
        glUniform1f(glGetUniformLocation(shader.Program, "focos[18].linear"), 0.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "focos[18].quadratic"), 0.2f);

        // ----- Pasto -----
        UsarTextura(shader, texPasto, REPETIR_PASTO, REPETIR_PASTO);
        DibujarPasto(shader);

        // ----- Pista -----
        UsarTextura(shader, texAsfalto, REPETIR_ASFALTO_U, REPETIR_ASFALTO_V);
        DibujarPista(shader);

        glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
        //DibujarGradas(shader);

        DibujarMarcador(shader, currentFrame);


        DibujarFermin(shader);
       
        for (int i = 0; i < 11; i++)
            DibujarTecpro_F1(shader, tecproCurva1[i][0], 0.0f, tecproCurva1[i][1], tecproCurva1[i][2]);
        for (int i = 0; i < 14; i++)
            DibujarTecpro_F1(shader, tecproHorquilla[i][0], 0.0f, tecproHorquilla[i][1], tecproHorquilla[i][2]);
        for (int i = 0; i < 10; i++)
            DibujarTecpro_F1(shader, tecproForoSol[i][0], 0.0f, tecproForoSol[i][1], tecproForoSol[i][2]);
        for (int i = 0; i < 10; i++)
            DibujarTecpro_F1(shader, tecproS[i][0], 0.0f, tecproS[i][1], tecproS[i][2]);
        
        // BARRERAS DE CONCRETO 
        int numBarreras = 12; // 66.5 de largo / 6 por barrera = quedan pegadas

        float dx1 = 65.85f, dz1 = 9.67f;   // Fila izquierda
        float dx2 = 65.94f, dz2 = 10.38f;  // Fila derecha
        float rotacionIzquierda = glm::degrees(atan2(dx1, dz1));
        float rotacionDerecha = glm::degrees(atan2(dx2, dz2)) + 180.0f;
		//Inicio recta principal 
        DibujarFilaBarreras(shader, -175.65f, -163.64f, -109.80f, -153.97f, false);
        DibujarFilaBarreras(shader, -174.51f, -177.76f, -108.57f, -167.38f, true);
        // Final de la recta principal 
        DibujarFilaBarreras(shader, 147.68f, -117.45f, 195.35f, -110.83f, false);
        DibujarFilaBarreras(shader, 149.60f, -131.31f, 197.27f, -124.69f, true);

        // inicio de la recta larga, saliendo de las eses
        DibujarFilaBarreras(shader, 207.95f, -6.22f, 178.86f, 41.81f, false);
        DibujarFilaBarreras(shader, 219.93f, 1.02f, 190.84f, 49.05f, true);
        // final de la recta larga, frenado antes de la horquilla
        DibujarFilaBarreras(shader, 153.95f, 82.67f, 136.30f, 109.46f, false);
        DibujarFilaBarreras(shader, 165.65f, 90.37f, 148.00f, 117.16f, true);

        // Recta de abajo a la izquierda (despues de la bandera)
        DibujarFilaBarreras(shader, -156.48f, -64.78f, -100.86f, -57.08f, false);
        DibujarFilaBarreras(shader, -154.56f, -78.64f, -98.94f, -70.94f, true);

        DibujarPoste(shader, -150.0f, 0.0f, -150.0f, -90.0f);
        DibujarPoste(shader, 204.06f, 0.0f, -108.12f, -90.0f);
        DibujarPoste(shader, 215.1f, 0.0f, -96.148f, 160.0f);
        DibujarPoste(shader, 228.83f, 0.0f, -69.564f, 140.0f);
        DibujarPoste(shader, 191.55f, 0.0f, 1.256f, 120.0f);
        DibujarPoste(shader, 102.811f, 0.0f, 134.32f, 180.0f);
        DibujarPoste(shader, 30.858f, 0.0f, -7.2571f, 70.0f);
        DibujarPoste(shader, -109.08f, 0.0f, -77.373f, 80.0f);
        DibujarPoste(shader, -170.0f, 0.0f, -120.0f, 0.0f);


        // ----- Monoplaza del jugador y rivales (modelado jerarquico) -----
        JuegoDibujarCarros(shader, modoCamara != 2);

        // ----- Zona de neumaticos (zona de apoyo) -----
        DibujarZonaNeumaticos(shader);

        // ----- Dia de Muertos: calaveras, cempasuchil y petalos -----
        DibujarDiaDeMuertos(shader);

        // ---------- Pantallas alrededor del circuito ----------
        // (x, y, z, rotacion en Y).
        DibujarPantalla(shader, texPantalla[cuadroActual], -57.1f, 0.0f, -134.7f, -98.0f); // Recta principal
        DibujarPantalla(shader, texPantalla[cuadroActual], -180.5f, 0.0f, -143.3f, -33.0f); // Curva izquierda arriba
        DibujarPantalla(shader, texPantalla[cuadroActual], -91.2f, 0.0f, -44.6f, 82.0f); // Recta interior
        DibujarPantalla(shader, texPantalla[cuadroActual], 42.7f, 0.0f, -12.8f, 84.0f); // Despues de la S
        DibujarPantalla(shader, texPantalla[cuadroActual], 216.3f, 0.0f, -79.4f, 118.0f); // Derecha (eses)
        DibujarPantalla(shader, texPantalla[cuadroActual], 153.5f, 0.0f, 124.5f, -124.0f); // Antes de la horquilla


		// Dibuja bandera animada
        shaderBandera.Use();
        // Enviar matrices de cámara al nuevo shader
        glUniformMatrix4fv(glGetUniformLocation(shaderBandera.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderBandera.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform1f(glGetUniformLocation(shaderBandera.Program, "time"), currentFrame);

        DibujarBandera(shader, shaderBandera);

        // ZONA DEL ESTADIO (FORO SOL) - ALTAR DE MUERTOS
        shader.Use(); // la bandera deja activo otro shader
        DibujarAltar(shader, -160.0f, 0.0f, -105.0f, -90.0f, 1.0f);

        // ----- Spline (linea blanca por el centro de la pista) -----
        if (verSpline)
        {
            lampShader.Use();
            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(vaoSpline);
            glDrawArrays(GL_LINE_LOOP, 0, numPuntosSpline);
            glBindVertexArray(0);
        }

        // ----- Papel picado, tiempo en el marcador, letrero de la zona y HUD -----
        DibujarPapelPicado(projection, view, currentFrame);
        DibujarTiempoMarcador(projection * view);
        DibujarLetreroZona(projection * view);
        DibujarHUDJuego(SCREEN_WIDTH, SCREEN_HEIGHT, modoCamara);

        glfwSwapBuffers(window);
    }

    AudioCerrar();
    LiberarModelos();
    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
    if (modoCamara != 0) return;   // con el carro, WASD lo controla el juego (Juego.cpp)

    // Con Shift se mueve 15 veces mas rapido (la pista mide ~500)
    int pasos = (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT]) ? 15 : 1;

    for (int i = 0; i < pasos; i++)
    {
        if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)        keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    // L: mostrar / ocultar la spline
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        verSpline = !verSpline;

	// L: activar / desactivar luces de los postes
    if (key == GLFW_KEY_F && action == GLFW_PRESS) { lucesActivas = true; tiempoLuces = 0.0f; }

    // C: cambiar de camara (tercera persona -> onboard -> libre)
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
        modoCamara = (modoCamara + 1) % 3;

    // R: abrir / cerrar DRS (solo en las rectas)
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        JuegoBotonDRS();

    // T: reiniciar la vuelta
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
        JuegoReiniciar();
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = (GLfloat)xPos;
        lastY = (GLfloat)yPos;
        firstMouse = false;
    }

    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;  // Reversed since y-coordinates go from bottom to left

    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;

    if (modoCamara == 0)   // el mouse solo mueve la camara libre
        camera.ProcessMouseMovement(xOffset, yOffset);
}


