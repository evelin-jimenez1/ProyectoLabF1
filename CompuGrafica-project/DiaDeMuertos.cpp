// =============================================================
//  DiaDeMuertos.cpp
// =============================================================
#include "DiaDeMuertos.h"
#include "Primitivas.h"
#include "Spline.h"
#include "Texturas.h"
#include "CargaModelos.h"

#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const float PI = 3.14159265f;

// =============================================================
//  CALAVERAS (maquina de estados, como el perro de la practica)
//  Estan sentadas en el piso. Cuando el carro pasa cerca:
//    estado 1 AGACHARSE -> toma impulso (se aplasta un poco)
//    estado 2 BRINCO    -> salta estirada y da una vuelta completa
//    estado 3 CAER      -> cae aplastandose y queda mirando al carro
//    estado 0 QUIETA    -> solo "respira" muy poquito
// =============================================================
enum { QUIETA = 0, AGACHARSE = 1, BRINCO = 2, CAER = 3 };

struct Calavera {
	glm::vec3 pos;      // donde esta (en el piso)
	float escala;       // el modelo mide 1 m -> escala = metros de alto
	float radio;        // si el carro pasa a menos de esto, brinca
	float giro;         // hacia donde mira (grados)
	float giroIni, giroFin;
	int   estado;       // QUIETA, AGACHARSE, BRINCO o CAER
	float tiempo;       // segundos dentro del estado actual
	bool  cerca;        // para brincar solo una vez cada que el carro llega
};

// Lugares libres alrededor de la pista (ninguna toca el asfalto)
Calavera calaveras[] = {
	// posicion                          escala  radio
	{ glm::vec3(150.0f, 0.0f,  -25.0f), 60.0f, 100.0f, 0, 0, 0, QUIETA, 0, false }, // GIGANTE: centro del circuito
	{ glm::vec3(-40.0f, 0.0f, -185.0f), 30.0f,  55.0f, 0, 0, 0, QUIETA, 0, false }, // junto a la recta principal
	{ glm::vec3(-210.0f, 0.0f, -205.0f), 30.0f,  55.0f, 0, 0, 0, QUIETA, 0, false }, // afuera de la curva 17 (Peraltada)
};
const int   N_CALAVERAS = sizeof(calaveras) / sizeof(calaveras[0]);
const float T_AGACHARSE = 0.25f;     // segundos de cada estado
const float T_BRINCO = 1.05f;
const float T_CAER = 0.30f;
const float CALAV_MIRA_OFF = -90.0f; // el modelo mira a +X; con esto la cara apunta al carro
const float CALAV_BASE = 0.499f;     // mitad de la altura del modelo (para apoyarlo en el piso)

// Angulo para que una calavera en 'pos' mire hacia 'objetivo'
static float MirarHacia(glm::vec3 pos, glm::vec3 objetivo)
{
	return atan2f(objetivo.x - pos.x, objetivo.z - pos.z) * 180.0f / PI + CALAV_MIRA_OFF;
}

void AnimarCalaveras(float deltaTime, glm::vec3 posCarro)
{
	for (int i = 0; i < N_CALAVERAS; i++)
	{
		Calavera& c = calaveras[i];
		float dx = posCarro.x - c.pos.x, dz = posCarro.z - c.pos.z;
		bool cerca = sqrtf(dx * dx + dz * dz) < c.radio;
		c.tiempo += deltaTime;

		switch (c.estado)
		{
		case QUIETA:
			if (cerca && !c.cerca)              // el carro acaba de llegar: brinco!
			{
				float mira = MirarHacia(c.pos, posCarro);
				while (mira - c.giro > 180.0f) mira -= 360.0f;
				while (mira - c.giro < -180.0f) mira += 360.0f;
				c.giroIni = c.giro;
				c.giroFin = mira + 360.0f;      // una vuelta completa y termina viendo al carro
				c.estado = AGACHARSE;
				c.tiempo = 0.0f;
			}
			break;
		case AGACHARSE:
			if (c.tiempo >= T_AGACHARSE) { c.estado = BRINCO; c.tiempo = 0.0f; }
			break;
		case BRINCO:
		{
			float v = c.tiempo / T_BRINCO;      // el giro ocurre en el aire
			if (v > 1.0f) v = 1.0f;
			v = v * v * (3.0f - 2.0f * v);      // arranca y termina suave
			c.giro = c.giroIni + (c.giroFin - c.giroIni) * v;
			if (c.tiempo >= T_BRINCO) { c.estado = CAER; c.tiempo = 0.0f; }
			break;
		}
		case CAER:
			if (c.tiempo >= T_CAER)
			{
				c.estado = QUIETA;
				c.tiempo = 0.0f;
				c.giro = c.giroFin - 360.0f;
			}
			break;
		}
		c.cerca = cerca;
	}
}

static void DibujarCalaveras(Shader& shader)
{
	glm::mat4 model(1);
	for (int i = 0; i < N_CALAVERAS; i++)
	{
		Calavera& c = calaveras[i];
		float alto = 1.0f + 0.015f * sinf((float)glfwGetTime() * 2.0f + i * 1.7f);   // respira
		float ancho = 1.0f;
		float salto = 0.0f;

		switch (c.estado)
		{
		case AGACHARSE:   // se aplasta para tomar impulso
		{
			float k = sinf(PI * 0.5f * c.tiempo / T_AGACHARSE);
			alto = 1.0f - 0.25f * k;  ancho = 1.0f + 0.12f * k;
			break;
		}
		case BRINCO:      // parabola: sube y baja
		{
			float v = c.tiempo / T_BRINCO;
			salto = 0.6f * c.escala * 4.0f * v * (1.0f - v);
			alto = 1.12f;  ancho = 0.94f;
			break;
		}
		case CAER:        // se aplasta al caer y regresa
		{
			float k = sinf(PI * c.tiempo / T_CAER);
			alto = 1.0f - 0.22f * k;  ancho = 1.0f + 0.10f * k;
			break;
		}
		}

		model = glm::translate(glm::mat4(1), c.pos + glm::vec3(0.0f, salto, 0.0f));
		model = glm::rotate(model, glm::radians(c.giro), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(c.escala * ancho, c.escala * alto, c.escala * ancho));
		model = glm::translate(model, glm::vec3(0.0f, CALAV_BASE, 0.0f));  // apoyada en el piso
		DibujarModeloCalavera(shader, model);
	}
}


// =============================================================
//  CEMPASUCHIL Y CAMINO DE PETALOS
//  En Dia de Muertos el camino de petalos guia a las almas:
//  aqui marca las orillas de la recta principal.
// =============================================================
static GLuint texPetalos, texFlor, texCentroFlor, texHojas, texMaceta;
static GLuint VAOPetalos = 0;
static int numVerticesPetalos = 0;

// Tramos de la recta principal (metros sobre la spline)
static const float RECTA_INICIO = 1235.0f;
static const float RECTA_FIN = 205.0f;       // ya en la siguiente vuelta
const float LATERAL_PETALOS = 4.55f;         // orilla del asfalto (ancho 8 m)
const float ANCHO_PETALOS = 0.9f;
const float LATERAL_FLORES = 9.5f;           // arreglos de flores, por fuera de las barreras

// Textura de petalos: puntos naranjas, amarillos y rojizos
static GLuint CrearTexturaPetalos()
{
	const int N = 64;
	std::vector<unsigned char> pix(N * N * 3);
	unsigned int semilla = 12345;
	for (int i = 0; i < N * N; i++)
	{
		semilla = semilla * 1103515245u + 12345u;
		int r = (semilla >> 16) % 100;
		glm::vec3 c = (r < 55) ? glm::vec3(1.0f, 0.55f, 0.05f)      // naranja cempasuchil
			: (r < 85) ? glm::vec3(1.0f, 0.75f, 0.10f)                // amarillo
			: glm::vec3(0.85f, 0.30f, 0.05f);                         // naranja oscuro
		pix[i * 3 + 0] = (unsigned char)(c.r * 255);
		pix[i * 3 + 1] = (unsigned char)(c.g * 255);
		pix[i * 3 + 2] = (unsigned char)(c.b * 255);
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, N, N, 0, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

// Dos tiras de petalos (una por orilla) siguiendo la spline, un punto cada metro
static void CrearCaminoPetalos()
{
	std::vector<float> v;   // posicion, normal, textura (como el plano del pasto)
	float L = SplineLongitud();
	float largo = (L - RECTA_INICIO) + RECTA_FIN;
	for (int lado = -1; lado <= 1; lado += 2)
	{
		for (float d = 0.0f; d < largo; d += 1.0f)
		{
			float s0 = RECTA_INICIO + d, s1 = s0 + 1.0f;
			float dentro = lado * LATERAL_PETALOS, fuera = lado * (LATERAL_PETALOS + ANCHO_PETALOS);
			glm::vec3 a = SplinePuntoPista(s0, dentro), b = SplinePuntoPista(s0, fuera);
			glm::vec3 c = SplinePuntoPista(s1, fuera), e = SplinePuntoPista(s1, dentro);
			float y = 0.03f;   // apenas arriba del pasto
			glm::vec3 q[6] = { a, b, c, a, c, e };
			float uv[6][2] = { {0, d}, {1, d}, {1, d + 1}, {0, d}, {1, d + 1}, {0, d + 1} };
			for (int k = 0; k < 6; k++)
				v.insert(v.end(), { q[k].x, y, q[k].z, 0.0f, 1.0f, 0.0f, uv[k][0], uv[k][1] * 0.5f });
		}
	}
	numVerticesPetalos = (int)(v.size() / 8);

	GLuint VBO;
	glGenVertexArrays(1, &VAOPetalos);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAOPetalos);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

// Un arreglo: maceta de barro con 5 flores de cempasuchil.
// La flor es una esfera naranja aplastada con un centro mas oscuro.
static void ArregloCempasuchil(Shader& shader, glm::vec3 base, float rotY)
{
	glm::mat4 model(1);
	glm::mat4 modelTemp(1);
	modelTemp = model = glm::rotate(glm::translate(glm::mat4(1), base), glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));

	// Maceta (cilindro de barro) y follaje
	model = glm::translate(modelTemp, glm::vec3(0.0f, 0.25f, 0.0f));
	model = glm::scale(model, glm::vec3(1.1f, 0.5f, 1.1f));
	DibujarMalla(shader, mallaCilindro, model, texMaceta);
	model = glm::translate(modelTemp, glm::vec3(0.0f, 0.62f, 0.0f));
	model = glm::scale(model, glm::vec3(1.05f, 0.35f, 1.05f));
	DibujarMalla(shader, mallaEsfera, model, texHojas);

	// Flores
	const glm::vec3 posFlor[5] = {
		glm::vec3(0.0f, 0.95f, 0.0f), glm::vec3(0.30f, 0.85f, 0.15f), glm::vec3(-0.28f, 0.85f, 0.18f),
		glm::vec3(0.12f, 0.85f, -0.30f), glm::vec3(-0.18f, 0.83f, -0.25f)
	};
	for (int i = 0; i < 5; i++)
	{
		model = glm::translate(modelTemp, posFlor[i]);
		model = glm::scale(model, glm::vec3(0.36f, 0.26f, 0.36f));
		DibujarMalla(shader, mallaEsfera, model, texFlor);
		model = glm::translate(modelTemp, posFlor[i] + glm::vec3(0.0f, 0.11f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f, 0.06f, 0.12f));
		DibujarMalla(shader, mallaEsfera, model, texCentroFlor);
	}
}

static void DibujarCempasuchil(Shader& shader)
{
	// Camino de petalos
	glUniform1f(glGetUniformLocation(shader.Program, "brilloPropio"), 0.0f);
	UsarTextura(shader, texPetalos, 1.0f, 1.0f);
	glm::mat4 model(1);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(VAOPetalos);
	glDrawArrays(GL_TRIANGLES, 0, numVerticesPetalos);
	glBindVertexArray(0);

	// Arreglos de flores cada 12 m, de los dos lados de la recta principal
	float L = SplineLongitud();
	float largo = (L - RECTA_INICIO) + RECTA_FIN;
	for (float d = 6.0f; d < largo; d += 12.0f)
	{
		float s = RECTA_INICIO + d;
		float sv = std::fmod(s, L);
		for (int lado = -1; lado <= 1; lado += 2)
		{
			// a la derecha entre los 45 y 105 m esta la zona de neumaticos
			if (lado > 0 && sv > 45.0f && sv < 105.0f) continue;
			glm::vec3 p = SplinePuntoPista(s, lado * LATERAL_FLORES);
			p.y = 0.0f;
			ArregloCempasuchil(shader, p, SplineAnguloY(s));
		}
	}
}


// =============================================================
//  PAPEL PICADO
//  Tiras de banderitas colgadas de un cable sobre la pista.
//  Cada banderita cuelga del cable y se balancea con el viento
//  (su propio shader descarta lo recortado del papel).
// =============================================================
static Shader* shaderPapel = nullptr;
static GLuint texPapel;
static GLuint VAOPapel = 0;
static GLuint texCable;

struct TiraPapel { float s; float alto; };   // metros sobre la spline y altura del cable
static const TiraPapel tiras[] = {
	{ 1270.0f, 7.0f }, { 1310.0f, 7.0f }, { 1350.0f, 7.0f }, { 1390.0f, 7.0f },   // recta principal
	{ 20.0f, 7.0f }, { 60.0f, 7.0f }, { 110.0f, 7.0f }, { 160.0f, 7.0f },
	{ 1040.0f, 6.0f }, { 1110.0f, 6.0f }, { 1180.0f, 6.0f },                    // Foro Sol
};
const int N_TIRAS = sizeof(tiras) / sizeof(tiras[0]);
const float MEDIO_ANCHO_TIRA = 7.0f;     // el cable va de -7 a 7 m del centro de la pista
const float ANCHO_PAPEL = 0.75f, ALTO_PAPEL = 0.95f;

// Colores del papel picado
static const glm::vec3 coloresPapel[6] = {
	glm::vec3(0.95f, 0.10f, 0.55f),   // rosa mexicano
	glm::vec3(1.00f, 0.55f, 0.05f),   // naranja cempasuchil
	glm::vec3(0.55f, 0.20f, 0.85f),   // morado
	glm::vec3(0.10f, 0.75f, 0.80f),   // turquesa
	glm::vec3(1.00f, 0.85f, 0.10f),   // amarillo
	glm::vec3(0.20f, 0.70f, 0.25f),   // verde
};

static void CrearPapelPicado()
{
	shaderPapel = new Shader("Shader/papel.vs", "Shader/papel.frag");
	texPapel = CargarTextura("images/papelPicado_mascara.png");   // blanco = papel, transparente = recorte
	texCable = CrearTexturaColor(0.1f, 0.1f, 0.1f);

	// Rectangulo de 1 x 1 que cuelga hacia abajo desde su orilla de arriba (y = 0)
	GLfloat vertices[] = {
		// Positions          // Normal            // Texture Coords
		-0.5f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
		 0.5f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
		 0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
		-0.5f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
		 0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
		-0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
	};
	GLuint VBO;
	glGenVertexArrays(1, &VAOPapel);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAOPapel);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

// Punto del cable: va de un lado a otro de la pista y se cuelga un poco en medio
static glm::vec3 PuntoCable(const TiraPapel& t, float u)   // u de 0 a 1
{
	float lateral = -MEDIO_ANCHO_TIRA + 2.0f * MEDIO_ANCHO_TIRA * u;
	glm::vec3 p = SplinePuntoPista(t.s, lateral);
	float panza = 0.8f * (1.0f - (2.0f * u - 1.0f) * (2.0f * u - 1.0f));
	p.y = t.alto - panza;
	return p;
}

void DibujarPapelPicado(const glm::mat4& projection, const glm::mat4& view, float tiempo)
{
	shaderPapel->Use();
	glUniformMatrix4fv(glGetUniformLocation(shaderPapel->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shaderPapel->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(glGetUniformLocation(shaderPapel->Program, "time"), tiempo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texPapel);
	glUniform1i(glGetUniformLocation(shaderPapel->Program, "texture_diffuse1"), 0);
	GLint modelLoc = glGetUniformLocation(shaderPapel->Program, "model");
	GLint colorLoc = glGetUniformLocation(shaderPapel->Program, "colorPapel");

	glm::mat4 model(1);
	glBindVertexArray(VAOPapel);
	for (int t = 0; t < N_TIRAS; t++)
	{
		float rotY = SplineAnguloY(tiras[t].s);   // las banderitas ven hacia los carros
		int n = (int)(2.0f * MEDIO_ANCHO_TIRA / ANCHO_PAPEL * 0.8f);
		for (int i = 0; i < n; i++)
		{
			float u = (i + 0.5f) / n;
			glm::vec3 p = PuntoCable(tiras[t], u);
			// se balancea colgada del cable (rota sobre su orilla de arriba)
			float viento = 18.0f * sinf(tiempo * 2.5f + i * 0.7f + t * 1.3f);

			model = glm::translate(glm::mat4(1), p);
			model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(viento), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(ANCHO_PAPEL, ALTO_PAPEL, 1.0f));
			glm::vec3 c = coloresPapel[(i + t) % 6];
			glUniform3f(colorLoc, c.r, c.g, c.b);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	glBindVertexArray(0);
}

// Cables y postes de las tiras (con el shader de la pista)
static void DibujarCables(Shader& shader)
{
	glm::mat4 model(1);
	for (int t = 0; t < N_TIRAS; t++)
	{
		// cable: tramos cortos entre puntos de la curva
		const int TRAMOS = 12;
		for (int k = 0; k < TRAMOS; k++)
		{
			glm::vec3 a = PuntoCable(tiras[t], (float)k / TRAMOS);
			glm::vec3 b = PuntoCable(tiras[t], (float)(k + 1) / TRAMOS);
			glm::vec3 d = b - a;
			float largo = glm::length(d);
			model = glm::translate(glm::mat4(1), (a + b) * 0.5f);
			model = glm::rotate(model, atan2f(d.x, d.z), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, -asinf(d.y / largo), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.04f, 0.04f, largo));
			DibujarMalla(shader, mallaCubo, model, texCable);
		}
		// postes en los extremos
		for (int lado = 0; lado < 2; lado++)
		{
			glm::vec3 p = PuntoCable(tiras[t], (float)lado);
			model = glm::translate(glm::mat4(1), glm::vec3(p.x, p.y * 0.5f, p.z));
			model = glm::scale(model, glm::vec3(0.12f, p.y, 0.12f));
			DibujarMalla(shader, mallaCilindro, model, texCable);
		}
	}
}


// =============================================================
void DiaDeMuertosInicializar()
{
	texPetalos = CrearTexturaPetalos();
	texFlor = CrearTexturaColor(1.0f, 0.55f, 0.05f);
	texCentroFlor = CrearTexturaColor(0.85f, 0.35f, 0.02f);
	texHojas = CrearTexturaColor(0.15f, 0.45f, 0.12f);
	texMaceta = CrearTexturaColor(0.70f, 0.35f, 0.18f);   // barro
	CrearCaminoPetalos();
	CrearPapelPicado();

	// al empezar, cada calavera mira hacia la parte mas cercana de la pista
	for (int i = 0; i < N_CALAVERAS; i++)
	{
		float mejor = 1e30f;
		glm::vec3 masCerca(0.0f);
		for (float s = 0.0f; s < SplineLongitud(); s += 5.0f)
		{
			glm::vec3 p = SplinePosicion(s);
			float d = glm::length(glm::vec2(p.x - calaveras[i].pos.x, p.z - calaveras[i].pos.z));
			if (d < mejor) { mejor = d; masCerca = p; }
		}
		calaveras[i].giro = MirarHacia(calaveras[i].pos, masCerca);
	}
}

void DibujarDiaDeMuertos(Shader& shader)
{
	glUniform2f(glGetUniformLocation(shader.Program, "repetir"), 1.0f, 1.0f);
	DibujarCalaveras(shader);
	DibujarCempasuchil(shader);
	DibujarCables(shader);
}
