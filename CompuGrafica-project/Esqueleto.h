#pragma once
// =============================================================
//  Esqueleto: modelos con ANIMACION DE HUESOS (skinning) cargados con Assimp
//
//  Sirve para cualquier .fbx / .dae con armadura y animaciones
//  (Blender, Mixamo, etc.). Tambien anima la jerarquia de nodos de un
//  modelo sin huesos (animaciones "rigidas" hechas en un software externo).
//
//  Como funciona (en cada cuadro):
//    1. Se elige el tiempo de la animacion y se interpola la posicion,
//       rotacion (slerp) y escala de cada hueso entre sus keyframes.
//    2. Se recorre la jerarquia de huesos (padre -> hijo) acumulando las
//       transformaciones: global = global(padre) * local.
//    3. Matriz final del hueso = inversa(raiz) * global(hueso) * offset(hueso)
//       (el 'offset' lo da Assimp y lleva el vertice del modelo al espacio del hueso).
//    4. Cada vertice se mueve con hasta 4 huesos, segun sus pesos:
//       v' = sum( peso_i * matrizFinal_i * v )
//    5. Los vertices nuevos se suben a la GPU y se dibuja con el shader de la pista.
//
//  Uso:
//    ModeloEsqueletico alebrije;
//    alebrije.Cargar("Models/alebrije_anim.fbx");   // despues de glewInit()
//    alebrije.Reproducir("Reposo");                 // nombre de la animacion (o su indice)
//    alebrije.Actualizar(deltaTime);                // cada cuadro
//    alebrije.Dibujar(shader, matrizModelo);        // con el shader de la pista activo
// =============================================================
#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"

class ModeloEsqueletico
{
public:
    ModeloEsqueletico();
    ~ModeloEsqueletico();

    // Carga el modelo (false si no se pudo). Llamar despues de glewInit().
    bool Cargar(const std::string& ruta);
    void Liberar();

    // ---- Animaciones ----
    int         NumAnimaciones() const;
    std::string NombreAnimacion(int indice) const;       // sin el prefijo "Armature|"
    int         BuscarAnimacion(const std::string& nombre) const;   // -1 si no existe
    bool        Reproducir(int indice, bool bucle = true);
    bool        Reproducir(const std::string& nombre, bool bucle = true);
    int         AnimacionActual() const { return animActual; }
    float       TiempoActual() const { return tiempo; }     // segundos dentro de la animacion

    // Pausa / reanuda la animacion (en pausa conserva la pose actual y no gasta CPU)
    void Pausar(bool p) { pausado = p; if (!p) sucio = true; }
    bool EstaPausado() const { return pausado; }

    // Avanza la animacion y recalcula los vertices (se llama una vez por cuadro)
    void Actualizar(float dt);

    // Dibuja con el shader de la pista (usa 'model', 'texture_diffuse1' y 'repetir')
    void Dibujar(Shader& shader, const glm::mat4& model);

    // Dibujo por mallas (para repetir una misma malla animada en varios lugares, p. ej. el publico)
    int  NumMallas() const;
    bool MallaConHuesos(int i) const;                      // true si la malla se deforma con huesos (no es estructura fija)
    void DibujarMalla(Shader& shader, const glm::mat4& model, int i);
    // Para repetir una malla con distinta fase de animacion: PoseEn(t) -> PielMalla(i) -> DibujarMalla(...)
    // Cambia la textura de una malla por un color liso (para mallas que el FBX trae en negro, sin textura de color)
    void  ColorMalla(int i, float r, float g, float b);
    void  PoseEn(float t);
    void  PielMalla(int i);
    float DuracionAnimacion() const;

private:
    struct Nodo
    {
        std::string nombre;
        glm::mat4   local;      // transformacion original del nodo (sin animar)
        int         padre;      // -1 = raiz
    };

    struct Malla
    {
        GLuint vao = 0, vbo = 0, ebo = 0, textura = 0;
        int    numIndices = 0;
        int    nodo = 0;                            // nodo del que cuelga (para mallas sin huesos)
        std::vector<glm::vec3>  pos0, nor0;         // vertices originales (pose de reposo)
        std::vector<glm::vec2>  uv;
        std::vector<glm::ivec4> hueso;              // 4 huesos por vertice
        std::vector<glm::vec4>  peso;               // sus pesos
        std::vector<int>        nodoDeHueso;        // hueso (de esta malla) -> nodo de la jerarquia
        std::vector<glm::mat4>  offset;             // hueso (de esta malla) -> matriz offset de Assimp
        std::vector<float>      datos;              // pos(3) + normal(3) + uv(2) por vertice, ya animados
    };

    Assimp::Importer importer;          // dueno de la escena: debe seguir vivo mientras se use
    const aiScene* escena = nullptr;

    std::string        directorio;
    std::vector<Nodo>  nodos;           // orden "padre antes que hijo"
    std::vector<Malla> mallas;
    std::vector<GLuint> texturasCreadas;       // todas las texturas de este modelo (para liberarlas)
    std::map<std::string, GLuint> cacheTex;    // una sola textura por imagen, aunque la usen varias mallas
    std::vector<glm::mat4> global;      // transformacion global de cada nodo en el cuadro actual
    glm::mat4          inversaRaiz = glm::mat4(1.0f);

    int   animActual = -1;
    bool  bucle = true;
    float tiempo = 0.0f;                // segundos
    bool  pausado = false;
    bool  sucio = true;                 // hay que recalcular la pose

    void ConstruirNodos(const aiNode* n, int padre);
    int  BuscarNodo(const std::string& nombre) const;
    void CrearMalla(const aiMesh* m, int nodoDeMalla);
    GLuint TexturaDelMaterial(const aiMaterial* mat);
    void CalcularPose();                // paso 1 y 2
    void AplicarPiel();                 // paso 3 y 4 + subir a la GPU
    void AplicarMalla(Malla& m);       // piel de una sola malla
};
