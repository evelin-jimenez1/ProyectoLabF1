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
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static GLuint texTraje, texGuantes, texVisera, texVolante, texRayaCasco;

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
	texRayaCasco = CrearTexturaColor(0.95f, 0.95f, 0.95f);
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

void DibujarPiloto(Shader& shader, const glm::mat4& chasis, float direccion, bool dibujarCabeza, glm::vec3 colorCasco,
                   const ReaccionPiloto& r)
{
	glm::mat4 model(1);
	glm::mat4 modelTemp(1);     // torso (padre de la cabeza y los brazos)
	glm::mat4 modelTemp2(1);    // brazo (padre del antebrazo)
	float giroVolante = direccion * RELACION_VOLANTE;

	// ---- Reaccion del piloto a los movimientos del carro (grados) ----
	//  empuje > 0 (acelera): el torso y la cabeza se van hacia atras; empuje < 0 (frena): hacia adelante
	//  direccion (curva): la fuerza lateral empuja el torso hacia afuera y la cabeza se inclina hacia adentro
	//  velocidad: vibracion fina del carro; impacto: sacudida fuerte despues de un choque
	float vibra = r.velocidad * 0.45f * std::sin(r.tiempo * 41.0f);
	float sacudida = r.impacto * 10.0f * std::sin(r.tiempo * 34.0f);
	float inclinacion = PILOTO_INCLINACION + 10.0f * r.empuje;                     // torso hacia atras / adelante
	float rolloTorso = -glm::clamp(direccion * 0.35f, -9.0f, 9.0f) + 0.5f * sacudida;   // lo empuja hacia afuera de la curva
	float cabeceo = (r.empuje > 0.0f ? 14.0f : 24.0f) * r.empuje + 0.8f * vibra + 0.7f * sacudida;   // cabeza atras / adelante
	float rolloCabeza = -rolloTorso + glm::clamp(direccion * 0.45f, -11.0f, 11.0f) + 0.4f * vibra;     // se inclina hacia adentro

	// 1. VOLANTE (hijo del chasis): gira sobre su columna
	model = glm::translate(chasis, VOLANTE_CENTRO);
	model = glm::rotate(model, glm::radians(VOLANTE_INCLINACION), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(giroVolante + 1.5f * vibra + 0.9f * sacudida), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.28f, 0.04f, 0.18f));
	DibujarMalla(shader, mallaCilindro, model, texVolante);

	// 2. TORSO (hijo del chasis): pivote en la cadera, inclinado hacia atras
	model = glm::translate(chasis, PILOTO_CADERA);
	model = glm::rotate(model, glm::radians(inclinacion), glm::vec3(1.0f, 0.0f, 0.0f));
	modelTemp = model = glm::rotate(model, glm::radians(rolloTorso), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(0.0f, LARGO_TORSO * 0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.36f, LARGO_TORSO, 0.22f));
	DibujarMalla(shader, mallaCubo, model, texTraje);

	// 3. CABEZA Y CASCO (hija del torso): voltea hacia la curva
	if (dibujarCabeza)   // en la camara onboard la camara va dentro del casco
	{
		model = glm::translate(modelTemp, glm::vec3(0.0f, LARGO_TORSO, 0.0f));
		model = glm::rotate(model, glm::radians(-rolloTorso), glm::vec3(0.0f, 0.0f, 1.0f));            // deshace el giro del torso
		model = glm::rotate(model, glm::radians(-inclinacion + cabeceo), glm::vec3(1.0f, 0.0f, 0.0f));  // la cabeza queda derecha (+ reaccion)
		model = glm::rotate(model, glm::radians(rolloCabeza + rolloTorso), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(direccion * 0.8f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelTemp2 = model;
		// Casco: esfera del color del piloto, visera oscura, raya blanca arriba y toma de aire
		model = glm::translate(model, glm::vec3(0.0f, 0.14f, 0.0f));
		model = glm::scale(model, glm::vec3(0.34f, 0.33f, 0.35f));
		DibujarMalla(shader, mallaEsfera, model, TexturaCasco(colorCasco));
		model = glm::translate(modelTemp2, glm::vec3(0.0f, 0.16f, -0.15f));      // visera
		model = glm::scale(model, glm::vec3(0.27f, 0.09f, 0.08f));
		DibujarMalla(shader, mallaCubo, model, texVisera);
		model = glm::translate(modelTemp2, glm::vec3(0.0f, 0.30f, -0.01f));      // raya blanca por el centro del casco
		model = glm::scale(model, glm::vec3(0.05f, 0.025f, 0.30f));
		DibujarMalla(shader, mallaCubo, model, texRayaCasco);
		model = glm::translate(modelTemp2, glm::vec3(0.0f, 0.32f, 0.10f));       // toma de aire
		model = glm::scale(model, glm::vec3(0.07f, 0.05f, 0.08f));
		DibujarMalla(shader, mallaCubo, model, TexturaCasco(colorCasco));
	}

	// 4. BRAZOS (hijos del torso): brazo -> antebrazo -> guante
	//    lado = -1 izquierdo, +1 derecho. Con la direccion un brazo sube y el otro baja.
	for (int i = 0; i < 2; i++)
	{
		float lado = (i == 0) ? -1.0f : 1.0f;

		// Brazo: pivote en el hombro
		model = glm::translate(modelTemp, glm::vec3(0.19f * lado, LARGO_TORSO - 0.06f, 0.0f));
		model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelTemp2 = model = glm::rotate(model, glm::radians(-lado * 12.0f + giroVolante * 0.25f + 0.5f * sacudida), glm::vec3(0.0f, 0.0f, 1.0f));
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
