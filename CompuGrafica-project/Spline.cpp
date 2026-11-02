// =============================================================
//  Spline.cpp
//  Catmull-Rom cerrada + tabla de distancias para poder pedir
//  posiciones por metros recorridos (velocidad constante).
// =============================================================
#include "Spline.h"
#include <vector>
#include <cmath>

// ---------------- Puntos de control (x, z) ----------------
// Uno cada ~8 m por el centro del asfalto de pistaf1.obj.
// El primero es la mitad de la recta principal (la meta).
static const float PUNTOS_CONTROL[][2] =
{
    {    5.63f,  -144.23f}, {   13.57f,  -143.12f}, {   21.52f,  -142.02f}, {   29.46f,  -140.92f},
    {   37.41f,  -139.81f}, {   45.35f,  -138.71f}, {   53.30f,  -137.60f}, {   61.24f,  -136.50f},
    {   69.19f,  -135.40f}, {   77.13f,  -134.29f}, {   85.08f,  -133.19f}, {   93.02f,  -132.09f},
    {  100.97f,  -130.99f}, {  108.91f,  -129.89f}, {  116.86f,  -128.79f}, {  124.80f,  -127.69f},
    {  132.75f,  -126.58f}, {  140.69f,  -125.48f}, {  148.64f,  -124.38f}, {  156.58f,  -123.27f},
    {  164.53f,  -122.17f}, {  172.47f,  -121.06f}, {  180.42f,  -119.96f}, {  188.37f,  -118.86f},
    {  196.31f,  -117.76f}, {  204.26f,  -116.67f}, {  212.20f,  -115.54f}, {  220.12f,  -114.29f},
    {  226.85f,  -110.60f}, {  227.27f,  -102.68f}, {  226.49f,   -94.70f}, {  225.96f,   -86.71f},
    {  229.60f,   -79.92f}, {  236.58f,   -76.00f}, {  239.79f,   -69.11f}, {  238.80f,   -61.16f},
    {  237.44f,   -53.25f}, {  235.31f,   -45.52f}, {  232.76f,   -37.92f}, {  229.83f,   -30.45f},
    {  226.20f,   -23.31f}, {  222.18f,   -16.36f}, {  218.07f,    -9.48f}, {  213.94f,    -2.60f},
    {  209.80f,     4.27f}, {  205.65f,    11.14f}, {  201.50f,    18.00f}, {  197.34f,    24.86f},
    {  193.18f,    31.71f}, {  189.02f,    38.57f}, {  184.85f,    45.43f}, {  180.69f,    52.29f},
    {  176.54f,    59.14f}, {  172.38f,    66.01f}, {  168.21f,    72.86f}, {  164.03f,    79.70f},
    {  159.80f,    86.52f}, {  155.52f,    93.31f}, {  151.18f,   100.05f}, {  146.72f,   106.72f},
    {  142.15f,   113.31f}, {  137.69f,   119.98f}, {  133.72f,   126.93f}, {  135.82f,   134.02f},
    {  142.46f,   138.51f}, {  148.58f,   143.64f}, {  147.30f,   150.76f}, {  141.18f,   155.93f},
    {  134.76f,   160.74f}, {  128.33f,   165.54f}, {  121.86f,   170.28f}, {  114.39f,   172.48f},
    {  108.68f,   167.37f}, {  108.28f,   159.42f}, {  109.27f,   151.46f}, {  110.34f,   143.52f},
    {  111.40f,   135.56f}, {  112.44f,   127.61f}, {  113.47f,   119.66f}, {  114.51f,   111.70f},
    {  115.55f,   103.75f}, {  116.59f,    95.80f}, {  117.64f,    87.84f}, {  118.67f,    79.89f},
    {  119.67f,    71.93f}, {  119.69f,    63.95f}, {  115.37f,    57.40f}, {  108.73f,    52.91f},
    {  101.95f,    48.63f}, {   95.59f,    43.75f}, {   90.55f,    37.55f}, {   87.01f,    30.36f},
    {   83.52f,    23.15f}, {   78.35f,    17.06f}, {   71.90f,    12.32f}, {   64.33f,     9.77f},
    {   56.40f,     8.64f}, {   48.42f,     7.77f}, {   40.44f,     7.00f}, {   32.45f,     6.28f},
    {   24.53f,     5.06f}, {   17.37f,     1.60f}, {   12.70f,    -4.82f}, {   10.13f,   -12.41f},
    {    7.73f,   -20.06f}, {    3.73f,   -26.97f}, {   -2.09f,   -32.46f}, {   -9.05f,   -36.42f},
    {  -16.28f,   -39.90f}, {  -23.35f,   -43.67f}, {  -30.27f,   -47.74f}, {  -37.28f,   -51.63f},
    {  -44.59f,   -54.93f}, {  -52.26f,   -57.24f}, {  -60.17f,   -58.51f}, {  -68.12f,   -59.62f},
    {  -76.07f,   -60.70f}, {  -84.01f,   -61.79f}, {  -91.96f,   -62.90f}, {  -99.90f,   -64.01f},
    { -107.85f,   -65.11f}, { -115.79f,   -66.21f}, { -123.74f,   -67.30f}, { -131.69f,   -68.40f},
    { -139.63f,   -69.50f}, { -147.58f,   -70.61f}, { -155.52f,   -71.71f}, { -163.47f,   -72.81f},
    { -171.26f,   -74.58f}, { -176.06f,   -80.59f}, { -176.96f,   -88.55f}, { -177.47f,   -96.56f},
    { -178.13f,  -104.55f}, { -179.03f,  -112.52f}, { -180.07f,  -120.47f}, { -181.50f,  -128.36f},
    { -186.99f,  -133.18f}, { -193.27f,  -128.60f}, { -198.93f,  -123.03f}, { -206.62f,  -123.79f},
    { -214.15f,  -126.45f}, { -222.12f,  -127.39f}, { -230.09f,  -128.28f}, { -237.38f,  -131.34f},
    { -239.76f,  -138.63f}, { -237.60f,  -146.33f}, { -233.82f,  -153.38f}, { -228.88f,  -159.70f},
    { -223.12f,  -165.26f}, { -216.36f,  -169.54f}, { -208.75f,  -172.03f}, { -200.81f,  -173.05f},
    { -192.79f,  -173.18f}, { -184.83f,  -172.23f}, { -176.89f,  -171.06f}, { -168.96f,  -169.91f},
    { -161.02f,  -168.72f}, { -153.10f,  -167.49f}, { -145.18f,  -166.23f}, { -137.26f,  -164.94f},
    { -129.34f,  -163.65f}, { -121.42f,  -162.37f}, { -113.50f,  -161.10f}, { -105.58f,  -159.87f},
    {  -97.64f,  -158.68f}, {  -89.71f,  -157.51f}, {  -81.77f,  -156.37f}, {  -73.83f,  -155.24f},
    {  -65.88f,  -154.13f}, {  -57.94f,  -153.03f}, {  -49.99f,  -151.92f}, {  -42.05f,  -150.82f},
    {  -34.10f,  -149.73f}, {  -26.15f,  -148.63f}, {  -18.21f,  -147.53f}, {  -10.26f,  -146.43f},
    {   -2.32f,  -145.33f},
};
static const int NUM_CONTROL = sizeof(PUNTOS_CONTROL) / sizeof(PUNTOS_CONTROL[0]);

// Cuantos puntos se calculan entre cada par de puntos de control
static const int MUESTRAS_POR_TRAMO = 10;

// Tabla: puntos de la curva y metros acumulados hasta cada uno
static std::vector<glm::vec3> curva;
static std::vector<float> distancia;
static float longitudTotal = 0.0f;


static glm::vec3 Control(int i)
{
    i = ((i % NUM_CONTROL) + NUM_CONTROL) % NUM_CONTROL;   // da la vuelta (pista cerrada)
    return glm::vec3(PUNTOS_CONTROL[i][0], ALTURA_PISTA, PUNTOS_CONTROL[i][1]);
}

// Formula de Catmull-Rom: curva suave que pasa por p1 (t=0) y p2 (t=1)
static glm::vec3 CatmullRom(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}


void SplineInicializar()
{
    curva.clear();
    distancia.clear();

    for (int i = 0; i < NUM_CONTROL; i++)
    {
        for (int k = 0; k < MUESTRAS_POR_TRAMO; k++)
        {
            float t = (float)k / MUESTRAS_POR_TRAMO;
            curva.push_back(CatmullRom(Control(i - 1), Control(i), Control(i + 1), Control(i + 2), t));
        }
    }

    // Metros acumulados (se agrega el primer punto al final para cerrar la vuelta)
    distancia.push_back(0.0f);
    for (size_t i = 1; i <= curva.size(); i++)
    {
        glm::vec3 a = curva[i - 1];
        glm::vec3 b = curva[i % curva.size()];
        distancia.push_back(distancia.back() + glm::length(b - a));
    }
    longitudTotal = distancia.back();
}


float SplineLongitud()
{
    return longitudTotal;
}


// Convierte 's' a [0, longitud) y busca entre que puntos de la tabla cae
static void Buscar(float s, int& i, float& f)
{
    s = std::fmod(s, longitudTotal);
    if (s < 0.0f) s += longitudTotal;

    // busqueda binaria en la tabla de distancias
    int lo = 0, hi = (int)distancia.size() - 1;
    while (hi - lo > 1)
    {
        int mid = (lo + hi) / 2;
        if (distancia[mid] <= s) lo = mid;
        else hi = mid;
    }
    i = lo;
    float tramo = distancia[lo + 1] - distancia[lo];
    f = (tramo > 0.0f) ? (s - distancia[lo]) / tramo : 0.0f;
}


glm::vec3 SplinePosicion(float s)
{
    int i; float f;
    Buscar(s, i, f);
    glm::vec3 a = curva[i];
    glm::vec3 b = curva[(i + 1) % curva.size()];
    return a + (b - a) * f;
}


glm::vec3 SplineTangente(float s)
{
    // diferencia entre un punto un poco adelante y uno un poco atras
    glm::vec3 d = SplinePosicion(s + 1.0f) - SplinePosicion(s - 1.0f);
    d.y = 0.0f;
    return glm::normalize(d);
}


glm::vec3 SplineDerecha(float s)
{
    glm::vec3 t = SplineTangente(s);
    return glm::vec3(-t.z, 0.0f, t.x);    // perpendicular en el suelo
}


glm::vec3 SplinePuntoPista(float s, float lateral)
{
    return SplinePosicion(s) + SplineDerecha(s) * lateral;
}


float SplineAnguloY(float s)
{
    glm::vec3 t = SplineTangente(s);
    // un modelo que mira hacia -Z queda viendo hacia 't'
    return glm::degrees(std::atan2(-t.x, -t.z));
}


GLuint CrearLineaSpline(int& numPuntos)
{
    // puntos un poco arriba del asfalto para que se vean
    std::vector<glm::vec3> linea;
    for (size_t i = 0; i < curva.size(); i++)
        linea.push_back(curva[i] + glm::vec3(0.0f, 0.05f, 0.0f));
    numPuntos = (int)linea.size();

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, linea.size() * sizeof(glm::vec3), &linea[0], GL_STATIC_DRAW);
    // Position attribute (lamp.vs solo usa la posicion)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return VAO;
}
