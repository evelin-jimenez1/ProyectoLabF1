#pragma once
// =============================================================
//  Texturas: cargar imagenes y activarlas antes de dibujar
// =============================================================
#include <GL/glew.h>
#include "Shader.h"

// Carga una imagen (.png o .jpg) como textura con GL_REPEAT.
// Devuelve el ID de la textura.
GLuint CargarTextura(const char* ruta);

// Activa una textura en la unidad 0 y le dice al shader
// cuantas veces repetirla en U y en V.
// Se llama justo antes de dibujar cada objeto.
void UsarTextura(Shader& shader, GLuint textura, float repetirU, float repetirV);
