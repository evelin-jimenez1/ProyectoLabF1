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

// Activa un mapa normal (unidad 6) y un mapa especular (unidad 7) para lo que se dibuje
// a continuacion. El shader de la escena los usa con el modelo Blinn-Phong.
void UsarMapas(Shader& shader, GLuint normal, GLuint especular);
// Regresa a la iluminacion normal (sin mapas)
void QuitarMapas(Shader& shader);
