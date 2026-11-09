// =============================================================
//  Piloto.cpp - piloto simplificado con modelado jerarquico
//  Mismo metodo que la practica de modelado jerarquico (brazo,
//  antebrazo, palma, falanges):
//    - se trasladan / rotan las articulaciones sobre 'model'
//    - modelTemp guarda la matriz del padre para colgar a los hijos
//    - el scale va al final y solo afecta a la figura que se dibuja
// =============================================================
#include "Piloto.h"
#include "Primitivas.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

static GLuint texTraje, texGuantes, texVisera, texVolante;

// Puntos en coordenadas del chasis (.obj de Blender)
const glm::vec3 PILOTO_CADERA(-0.847f, -0.52f, -2.42f);
const float     PILOTO_INCLINACION = 23.0f;     // grados hacia atras (van casi acostados)
const float     LARGO_TORSO = 0.38f;
const glm::vec3 VOLANTE_CENTRO(-0.847f, -0.20f, -2.66f);
const float     VOLANTE_INCLINACION = -60.0f;    // casi vertical, viendo al piloto
const float     RELACION_VOLANTE = 3.0f;         // grados de volante por grado de llanta

void PilotoInicializar()
{
	texTraje = CrearTexturaColor(0.10f, 0.25f, 0.55f);    // azul oscuro
	texGuantes = CrearTexturaColor(0.10f, 0.10f, 0.10f);
	texVisera = CrearTexturaColor(0.05f, 0.05f, 0.08f);
	texVolante = CrearTexturaColor(0.15f, 0.15f, 0.15f);
}

// Una textura por color de casco (se crea la primera vez que se pide)
static GLuint TexturaCasco(glm::vec3 color)
{
	static std::vector<std::pair<glm::vec3, GLuint>> creadas;
	for (auto& c : creadas)
		if (c.first == color) return c.second;
	GLuint tex = CrearTexturaColor(color.r, color.g, color.b);
	creadas.push_back({ color, tex });
	return tex;
}

void DibujarPiloto(Shader& shader, const glm::mat4& chasis, float direccion, bool dibujarCabeza, glm::vec3 colorCasco)
{
	glm::mat4 model(1);
	glm::mat4 modelTemp(1);     // torso (padre de la cabeza y los brazos)
	glm::mat4 modelTemp2(1);    // brazo (padre del antebrazo)
	float giroVolante = direccion * RELACION_VOLANTE;

	// 1. VOLANTE (hijo del chasis): gira sobre su columna
	model = glm::translate(chasis, VOLANTE_CENTRO);
	model = glm::rotate(model, glm::radians(VOLANTE_INCLINACION), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(giroVolante), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.28f, 0.04f, 0.18f));
	DibujarMalla(shader, mallaCilindro, model, texVolante);

	// 2. TORSO (hijo del chasis): pivote en la cadera, inclinado hacia atras
	model = glm::translate(chasis, PILOTO_CADERA);
	modelTemp = model = glm::rotate(model, glm::radians(PILOTO_INCLINACION), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, LARGO_TORSO * 0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.36f, LARGO_TORSO, 0.22f));
	DibujarMalla(shader, mallaCubo, model, texTraje);

	// 3. CABEZA Y CASCO (hija del torso): voltea hacia la curva
	if (dibujarCabeza)   // en la camara onboard la camara va dentro del casco
	{
		model = glm::translate(modelTemp, glm::vec3(0.0f, LARGO_TORSO, 0.0f));
		model = glm::rotate(model, glm::radians(-PILOTO_INCLINACION), glm::vec3(1.0f, 0.0f, 0.0f));  // la cabeza queda derecha
		model = glm::rotate(model, glm::radians(direccion * 0.8f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelTemp2 = model;
		model = glm::translate(model, glm::vec3(0.0f, 0.13f, 0.0f));
		model = glm::scale(model, glm::vec3(0.30f, 0.29f, 0.31f));
		DibujarMalla(shader, mallaEsfera, model, TexturaCasco(colorCasco));
		// Visera: franja oscura al frente del casco
		model = glm::translate(modelTemp2, glm::vec3(0.0f, 0.15f, -0.13f));
		model = glm::scale(model, glm::vec3(0.22f, 0.06f, 0.06f));
		DibujarMalla(shader, mallaCubo, model, texVisera);
	}

	// 4. BRAZOS (hijos del torso): brazo -> antebrazo -> guante
	//    lado = -1 izquierdo, +1 derecho. Con la direccion un brazo sube y el otro baja.
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? -1.0f : 1.0f;

		// Brazo: pivote en el hombro
		model = glm::translate(modelTemp, glm::vec3(0.19f * lado, LARGO_TORSO - 0.06f, 0.0f));
		model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelTemp2 = model = glm::rotate(model, glm::radians(-lado * 12.0f + giroVolante * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
		model = glm::scale(model, glm::vec3(0.09f, 0.26f, 0.09f));
		DibujarMalla(shader, mallaCilindro, model, texTraje);

		// Antebrazo: pivote en el codo
		model = glm::translate(modelTemp2, glm::vec3(0.0f, -0.26f, 0.0f));
		modelTemp2 = model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, -0.12f, 0.0f));
		model = glm::scale(model, glm::vec3(0.08f, 0.24f, 0.08f));
		DibujarMalla(shader, mallaCilindro, model, texTraje);

		// Guante
		model = glm::translate(modelTemp2, glm::vec3(0.0f, -0.25f, 0.0f));
		model = glm::scale(model, glm::vec3(0.09f));
		DibujarMalla(shader, mallaEsfera, model, texGuantes);
	}
}
