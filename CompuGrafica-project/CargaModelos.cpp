// =============================================================
//  CargaModelos.cpp
//  - Pista: modelo .obj de Blender cargado con Model.h (Assimp)
//  - Pasto: plano hecho a mano, como el cuadrado de la practica
//           de texturizado pero acostado en el suelo
//  - Monoplaza: modelado jerarquico (chasis -> llantas, alerones,
//           DRS y piloto), igual que el cuerpo del perro
// =============================================================
#include "CargaModelos.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// IMPORTANTE: este es el UNICO .cpp que incluye Model.h
// (Model.h tiene la funcion TextureFromFile escrita dentro del .h)
#include "Model.h"

// Para dibujar la pantalla de video
#include "Texturas.h"

// Piloto del monoplaza (hijo del chasis)
#include "Piloto.h"

// ---------------- Modelos ----------------
// Se guardan como punteros porque un Model solo se puede crear
// despues de glewInit(); por eso se crean dentro de CargarModelos().
static Model* gradasForo = nullptr;
static Model* marcadorForo = nullptr;
Model* modeloBandera;
Model* modeloAsta = nullptr;
Model* modeloTela = nullptr;
Model* modeloTecpro_F1 = nullptr;
Model* modeloBarreras = nullptr;
Model* modeloFermin = nullptr;
Model* modeloPoste = nullptr;
Model* modeloAltar = nullptr;
Model* manecillaHoras = nullptr;
Model* manecillaMinutos = nullptr;

Model* modeloPantalla = nullptr;
static GLuint VAOPantalla, VBOPantalla, EBOPantalla;
static GLuint texReversoPantalla;
const float PANTALLA_ANCHO = 13.0f;  // X
const float PANTALLA_ALTO = 7.3f;  // Z
const float PANTALLA_CENTROY = 8.433f;  // Z
const float PANTALLA_FRENTE = 0.42f;  // -(Y) + 0.01

// ---------------- Monoplaza ----------------
// Cada parte es un .obj separado, como las partes del perro
Model* Chasis = nullptr;       // el auto sin llantas ni DRS
Model* Aleron = nullptr;       // aleron trasero fijo
Model* DRS = nullptr;          // aleta del DRS
Model* Llanta_DI = nullptr;    // delantera izquierda
Model* Llanta_DD = nullptr;    // delantera derecha
Model* Llanta_TI = nullptr;    // trasera izquierda
Model* Llanta_TD = nullptr;    // trasera derecha

// Mapas normal y especular del auto (carro_pintura y carbono)
static GLuint texCarroNormal = 0, texCarroEsp = 0, texCarbonoNormal = 0, texCarbonoEsp = 0;

// ---------------- Dia de Muertos ----------------
Model* ModeloCalavera = nullptr;   // calavera de azucar (mide 1 m de alto)

// Posicion de cada llanta respecto al chasis (= el origen de cada llanta).
// Valores de posiciones_cpp.txt, exportados de CARROLab.blend
// Orden: delantera izq, delantera der, trasera izq, trasera der
glm::vec3 posLlanta[4] = {
    glm::vec3(-1.566f, -0.520f, -3.850f),
    glm::vec3(-0.126f, -0.520f, -3.850f),
    glm::vec3(-1.566f, -0.520f, -0.364f),
    glm::vec3(-0.126f, -0.520f, -0.364f)
};

// DRS: drs.obj ya viene en su lugar; gira sobre su orilla de enfrente (la bisagra)
glm::vec3 posDRS = glm::vec3(-0.847f, 0.0f, 0.0f);
const float DRS_SENTIDO = -1.0f;    // -1 = la orilla de atras sube. Si se abre al reves, poner 1

// llanta_TD.obj vino estirada en Z (mide 1.10 en vez de 0.69): se corrige al dibujarla
const float CORRECCION_TD_Z = 0.344f / 0.550f;


// ---------------- Pasto (plano) ----------------
static GLuint VAO, VBO, EBO;


void CargarModelos()
{
    // ----- Pista -----
    gradasForo = new Model((char*)"Models/Gradas_Estadio_Texturizadas.obj");
    marcadorForo = new Model((char*)"Models/Marcador_F1_animado.obj");
    modeloBandera = new Model((char*)"Models/Bandera_Mexico.obj");
    modeloAsta = new Model((char*)"Models/Asta.obj");
    modeloTela = new Model((char*)"Models/Tela.obj");
    modeloTecpro_F1 = new Model((char*)"Models/Tecpro_F1.obj");
    modeloBarreras = new Model((char*)"Models/Barreras.obj");
    modeloFermin = new Model((char*)"Models/Fermin.obj");

    // Mapas normal y especular del monoplaza (se cargan igual que las texturas de los modelos)
    texCarroNormal = TextureFromFile("carro_normal.png", "images");
    texCarroEsp = TextureFromFile("carro_especular.png", "images");
    texCarbonoNormal = TextureFromFile("carbono_normal.png", "images");
    texCarbonoEsp = TextureFromFile("carbono_especular.png", "images");
    modeloPoste = new Model((char*)"Models/Postes_Ilum.obj");
    modeloAltar = new Model((char*)"Models/Altar.obj");
    modeloPantalla = new Model((char*)"Models/Pantalla.obj");
    texReversoPantalla = CargarTextura("images/fibra.jpg");
    manecillaHoras = new Model((char*)"Models/Manecilla_Horas.obj");
    manecillaMinutos = new Model((char*)"Models/Manecilla_Minutos.obj");

    // ----- Monoplaza -----
    Chasis = new Model((char*)"Models/chasis.obj");
    Aleron = new Model((char*)"Models/aleron.obj");
    DRS = new Model((char*)"Models/drs.obj");
    Llanta_DI = new Model((char*)"Models/llanta_DI.obj");
    Llanta_DD = new Model((char*)"Models/llanta_DD.obj");
    Llanta_TI = new Model((char*)"Models/llanta_TI.obj");
    Llanta_TD = new Model((char*)"Models/llanta_TD.obj");

    // ----- Dia de Muertos -----
    ModeloCalavera = new Model((char*)"Models/calaverita.obj");


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

    // ----- Rectangulo de la pantalla (1 x 1) -----
    GLfloat verticesPantalla[] =
    {
        // Positions            // Normal            // Texture Coords
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
    };
    GLuint indicesPantalla[] = { 0, 1, 2,  0, 2, 3 };

    glGenVertexArrays(1, &VAOPantalla);
    glGenBuffers(1, &VBOPantalla);
    glGenBuffers(1, &EBOPantalla);
    glBindVertexArray(VAOPantalla);
    glBindBuffer(GL_ARRAY_BUFFER, VBOPantalla);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPantalla), verticesPantalla, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOPantalla);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPantalla), indicesPantalla, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
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



void LiberarModelos()
{
    delete gradasForo;
    gradasForo = nullptr;

	delete marcadorForo;
	marcadorForo = nullptr;

    delete modeloPoste;
	modeloPoste = nullptr;

    delete modeloAltar;
    modeloAltar = nullptr;

    delete modeloPantalla;
    modeloPantalla = nullptr;

    delete manecillaHoras;
    manecillaHoras = nullptr;

    delete manecillaMinutos;
    manecillaMinutos = nullptr;

    delete Chasis;     Chasis = nullptr;
    delete Aleron;     Aleron = nullptr;
    delete DRS;        DRS = nullptr;
    delete Llanta_DI;  Llanta_DI = nullptr;
    delete Llanta_DD;  Llanta_DD = nullptr;
    delete Llanta_TI;  Llanta_TI = nullptr;
    delete Llanta_TD;  Llanta_TD = nullptr;
    delete ModeloCalavera;  ModeloCalavera = nullptr;

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void DibujarGradas(Shader& shader)
{
    glm::mat4 model = glm::mat4(1.0f);
    // Posicion inicial para probar (luego modificaras estos numeros para acomodarlas en la curva 12)
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    gradasForo->Draw(shader);
}

void DibujarGradasEn(Shader& shader, const glm::mat4& model)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    gradasForo->Draw(shader);
}

void DibujarMarcador(Shader& shader, float tiempo)
{
    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, glm::vec3(-190.0f, 0.0f, -98.0f));
    base = glm::rotate(base, glm::radians(95.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Letrero y poste
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(base));
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
    marcadorForo->Draw(shader);

    //  Manecillas del reloj (animacion) 
    glm::vec3 centroReloj(-3.52f, 10.198f, -1.428f);
    float anguloMin = tiempo * 72.0f;      // minutero: 2 vueltas en 10 seg
    float anguloHora = anguloMin /36.0f;    // horario: 1 vuelta en 10 seg

    glm::mat4 m = glm::translate(base, centroReloj);
    m = glm::rotate(m, glm::radians(anguloMin), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::translate(m, -centroReloj);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
    manecillaMinutos->Draw(shader);

    m = glm::translate(base, centroReloj);
    m = glm::rotate(m, glm::radians(anguloHora), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::translate(m, -centroReloj);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
    manecillaHoras->Draw(shader);
}
void DibujarBandera(Shader shaderEstatico, Shader shaderViento) {
    glm::mat4 model = glm::mat4(1.0f);
    // Coordenadas (X, Y, Z) para acomodarla. Cámbialas a tu gusto.
    model = glm::translate(model, glm::vec3(-166.9f, 0.0f, -80.021f));
    model = glm::rotate(model, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    shaderEstatico.Use();
    glUniformMatrix4fv(glGetUniformLocation(shaderEstatico.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloAsta->Draw(shaderEstatico);
    
    shaderViento.Use();
    glUniformMatrix4fv(glGetUniformLocation(shaderViento.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloTela->Draw(shaderViento);

}

void DibujarFermin(Shader shader) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(120.0f, 0.0f, -70.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloFermin->Draw(shader);
}

void DibujarBarreras(Shader shader, float x, float y, float z, float rotY) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f)); // Gira el modelo
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // Ajusta la escala si es necesario
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloBarreras->Draw(shader);
}

void DibujarFilaBarreras(Shader shader, float xIni, float zIni, float xFin, float zFin, bool voltear)
{
    float dx = xFin - xIni;
    float dz = zFin - zIni;
    float largo = sqrt(dx * dx + dz * dz);
    int num = (int)(largo / 6.0f) + 1;          // 6 = largo de una barrera
    float rot = glm::degrees(atan2(dx, dz)) + (voltear ? 180.0f : 0.0f);

    for (int i = 0; i < num; i++) {
        float t = (num > 1) ? (float)i / (num - 1) : 0.0f;
        DibujarBarreras(shader, xIni + t * dx, 0.0f, zIni + t * dz, rot);
    }
}

void DibujarTecpro_F1(Shader shader, float x, float y, float z, float rotY) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloTecpro_F1->Draw(shader);
}

void DibujarPoste(Shader shader, float x, float y, float z, float rotY) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f)); 
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloPoste->Draw(shader);
}

void DibujarAltar(Shader shader, float x, float y, float z, float rotY, float escala) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(escala, escala, escala));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    modeloAltar->Draw(shader);
}

void DibujarPantalla(Shader& shader, GLuint textura, float x, float y, float z, float rotY)
{
    // Marco
    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, glm::vec3(x, y, z));
    base = glm::rotate(base, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(base));
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
    modeloPantalla->Draw(shader);

    // Imagen (el cuadro actual del video)
    glm::mat4 model = glm::translate(base, glm::vec3(0.0f, PANTALLA_CENTROY, PANTALLA_FRENTE));
    model = glm::scale(model, glm::vec3(PANTALLA_ANCHO, PANTALLA_ALTO, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 1.0f);
    UsarTextura(shader, textura, 1.0f, 1.0f);
    glBindVertexArray(VAOPantalla);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // Tapa de atras (para que el video no se vea por detras)
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);
    glm::mat4 atras = glm::translate(base, glm::vec3(0.0f, PANTALLA_CENTROY, -PANTALLA_FRENTE));
    atras = glm::rotate(atras, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // mira hacia atras
    atras = glm::scale(atras, glm::vec3(PANTALLA_ANCHO, PANTALLA_ALTO, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(atras));
    UsarTextura(shader, texReversoPantalla, 1.0f, 1.0f);
    glBindVertexArray(VAOPantalla);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);
}


// =============================================================
//  Monoplaza con modelado jerarquico (igual que el perro):
//
//      base (posicion en la pista)
//       └─ chasis                      (nodo padre)
//           ├─ aleron trasero          (fijo)
//           ├─ DRS                     (gira en su bisagra, keyframes)
//           ├─ llantas delanteras      (giran con el volante y ruedan)
//           ├─ llantas traseras        (ruedan)
//           └─ piloto y volante        (ver Piloto.cpp)
// =============================================================
void DibujarCarro(Shader& shader, glm::mat4 base, float giroRuedas,
                  float anguloDireccion, float aperturaDRS,
                  glm::vec3 pintura, bool dibujarCabeza, const ReaccionPiloto& reaccion)
{
    glm::mat4 model(1);
    glm::mat4 modelCarro(1);
    GLint modelLoc = glGetUniformLocation(shader.Program, "model");
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);

    // Los rivales usan el mismo modelo con otra pintura
    bool esRival = (pintura != glm::vec3(1.0f));
    glUniform1f(glGetUniformLocation(shader.Program, "usarTinte"), esRival ? 1.0f : 0.0f);
    glUniform3f(glGetUniformLocation(shader.Program, "tinte"), pintura.r, pintura.g, pintura.b);

    // ---------- Chasis (padre) ----------
    // El origen del chasis no esta al centro del carro: usamos el punto
    // medio entre las 4 llantas, a ras del piso
    glm::vec3 centro = (posLlanta[0] + posLlanta[1] + posLlanta[2] + posLlanta[3]) / 4.0f;
    centro.y -= RADIO_LLANTA;
    modelCarro = glm::translate(base, -centro);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCarro));
    UsarMapas(shader, texCarroNormal, texCarroEsp);      // pintura: mapa normal + especular
    Chasis->Draw(shader);
    Aleron->Draw(shader);   // fijo: usa la misma matriz del chasis
    QuitarMapas(shader);
    glUniform1f(glGetUniformLocation(shader.Program, "usarTinte"), 0.0f);   // lo demas no se pinta

    // ---------- DRS (hijo del chasis, gira en su bisagra) ----------
    model = glm::translate(modelCarro, posDRS);
    model = glm::rotate(model, glm::radians(DRS_SENTIDO * aperturaDRS), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, -posDRS);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    UsarMapas(shader, texCarbonoNormal, texCarbonoEsp);  // fibra de carbono
    DRS->Draw(shader);
    QuitarMapas(shader);

    // ---------- Llantas (como las patas del perro) ----------
    Model* llantas[4] = { Llanta_DI, Llanta_DD, Llanta_TI, Llanta_TD };
    for (int i = 0; i < 4; i++)
    {
        model = glm::translate(modelCarro, posLlanta[i]);
        if (i < 2)   // las delanteras giran con el volante
            model = glm::rotate(model, glm::radians(anguloDireccion), glm::vec3(0.0f, 1.0f, 0.0f));
        // el carro avanza hacia -Z: para rodar hacia adelante se gira en -X
        model = glm::rotate(model, glm::radians(-giroRuedas), glm::vec3(1.0f, 0.0f, 0.0f));
        if (i == 3)  // arregla el .obj estirado
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, CORRECCION_TD_Z));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        llantas[i]->Draw(shader);
    }

    // ---------- Piloto y volante (hijos del chasis) ----------
    // Casco amarillo para el jugador; del color de su carro para los rivales
    glm::vec3 casco = esRival ? pintura : glm::vec3(1.0f, 0.85f, 0.1f);
    DibujarPiloto(shader, modelCarro, anguloDireccion, dibujarCabeza, casco, reaccion);
}


// Calavera de azucar (la animacion esta en DiaDeMuertos.cpp)
void DibujarModeloCalavera(Shader& shader, glm::mat4 model)
{
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    ModeloCalavera->Draw(shader);
}
