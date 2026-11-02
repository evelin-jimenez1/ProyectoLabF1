#pragma once
// =============================================================
//  Spline de la pista (Catmull-Rom cerrada)
//
//  Los puntos de control se calcularon a partir del centro de
//  Models/pistaf1.obj, asi que la curva pasa justo por la mitad
//  del asfalto. Sirve para:
//    - mover el auto y los rivales sobre la pista
//    - limitar el movimiento lateral (colision con el muro)
//    - checkpoints y linea de meta
//    - la camara de transmision (TV)
//
//  Todo se mide en METROS recorridos sobre la pista ("s"):
//    s = 0                -> mitad de la recta principal (meta)
//    s = SplineLongitud() -> una vuelta completa
//  Si s se pasa de la longitud, se da la vuelta solo.
// =============================================================
#include <GL/glew.h>
#include <glm/glm.hpp>

// Ancho del asfalto en el modelo (m) y altura de su superficie
const float ANCHO_PISTA = 8.0f;
const float ALTURA_PISTA = 0.3f;

// Se llama UNA vez al inicio (antes de usar las demas funciones)
void SplineInicializar();

// Longitud total de una vuelta (m)
float SplineLongitud();

// Punto del centro de la pista a 's' metros de la meta
glm::vec3 SplinePosicion(float s);

// Direccion de avance en ese punto (vector unitario)
glm::vec3 SplineTangente(float s);

// Direccion hacia la derecha del auto en ese punto (vector unitario)
glm::vec3 SplineDerecha(float s);

// Punto sobre la pista: 's' metros recorridos y 'lateral' metros
// a la derecha (+) o izquierda (-) del centro.
// 'lateral' va de -ANCHO_PISTA/2 a +ANCHO_PISTA/2.
glm::vec3 SplinePuntoPista(float s, float lateral);

// Angulo (en grados, eje Y) para orientar un modelo hacia donde
// avanza la pista. Pensado para modelos que miran hacia -Z.
float SplineAnguloY(float s);

// ---------- Para ver la spline en pantalla (depuracion) ----------
// Crea una linea por el centro de la pista. Devuelve el VAO y
// guarda en 'numPuntos' cuantos vertices tiene.
// Se dibuja con: glDrawArrays(GL_LINE_LOOP, 0, numPuntos);
GLuint CrearLineaSpline(int& numPuntos);
