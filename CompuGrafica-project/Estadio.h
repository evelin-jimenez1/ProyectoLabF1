#pragma once
// =============================================================
//  Estadio: elementos de la zona de estadio y de la recta principal
//    - Gradas con publico (modulo .obj repetido en varios lugares)
//    - Porticos de cronometraje (el de la meta lleva el semaforo)
//    - Linea de meta a cuadros sobre el asfalto
//    - Vallas publicitarias con patrocinadores ficticios
//  Todo se coloca con la spline de la pista (ver Spline.h).
// =============================================================
#include <GL/glew.h>
#include "Shader.h"

void EstadioInicializar();            // despues de CargarModelos() y SplineInicializar()

void DibujarGradasEstadio(Shader& shader);
void DibujarPorticos(Shader& shader);
void DibujarAsfalto(Shader& shader);         // asfalto + lineas, generados desde la spline
void DibujarLineaMeta(Shader& shader);
void DibujarPianos(Shader& shader);          // curbs rojos y blancos a las orillas de toda la pista
void DibujarVallas(Shader& shader);
void DibujarArbolesYCerros(Shader& shader);   // ambientacion: arboles fuera de la pista y cerros al fondo

// Tablero de ajedrez (blanco y negro) de columnas x filas cuadros, filtro NEAREST.
// Sirve para la linea de meta y la bandera a cuadros del comisario.
GLuint CrearTexturaDamero(int columnas, int filas);
