// =============================================================
//  Proyecto Final - Autodromo Hermanos Rodriguez (F1)
//  main.cpp: ventana, camara y ciclo de render.
//    - Las texturas estan en      Texturas.h / Texturas.cpp
//    - Los modelos estan en       CargaModelos.h / CargaModelos.cpp
//    - La spline de la pista en   Spline.h / Spline.cpp
//
//  Controles: W A S D = mover | Shift = rapido | mouse = mirar | ESC = salir
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

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


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

    // ---------- Texturas (Texturas.cpp) ----------
    GLuint texAsfalto = CargarTextura("images/asfalto.png");
    GLuint texPasto = CargarTextura("images/pasto.png");

    glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 2000.0f);

    // Propiedades del Sol (Luz Direccional/Puntual)
    glm::vec3 lightPos(0.0f, 300.0f, 0.0f); // Sol alto en el centro del mapa
    glm::vec3 lightColor(1.0f, 0.95f, 0.8f); // Luz ligeramente amarilla/calida

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);   // cielo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Enviar variables de Iluminacion
        glUniform3f(glGetUniformLocation(shader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.Program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // ----- Pasto -----
        UsarTextura(shader, texPasto, REPETIR_PASTO, REPETIR_PASTO);
        DibujarPasto(shader);

        // ----- Pista -----
        UsarTextura(shader, texAsfalto, REPETIR_ASFALTO_U, REPETIR_ASFALTO_V);
        DibujarPista(shader);

        glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
        DibujarGradas(shader);

        DibujarMarcador(shader);

		// Dibuja bandera animada
        shaderBandera.Use();
        // Enviar matrices de cámara al nuevo shader
        glUniformMatrix4fv(glGetUniformLocation(shaderBandera.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderBandera.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform1f(glGetUniformLocation(shaderBandera.Program, "time"), currentFrame);

        DibujarBandera(shader, shaderBandera);


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

        glfwSwapBuffers(window);
    }

    LiberarModelos();
    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
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

    camera.ProcessMouseMovement(xOffset, yOffset);
}