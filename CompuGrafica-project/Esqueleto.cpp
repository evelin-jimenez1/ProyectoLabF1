// =============================================================
//  Esqueleto.cpp: animacion de huesos (skinning) con Assimp. Ver Esqueleto.h
// =============================================================
#include "Esqueleto.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SOIL2/SOIL2.h"

// ---------------------------------------------------------------
// Utilidades
// ---------------------------------------------------------------
// Assimp guarda las matrices por filas (a1 a2 a3 a4 = primera fila); glm las guarda por columnas.
static glm::mat4 ConvertirMatriz(const aiMatrix4x4& m)
{
    glm::mat4 r;
    r[0][0] = m.a1; r[1][0] = m.a2; r[2][0] = m.a3; r[3][0] = m.a4;
    r[0][1] = m.b1; r[1][1] = m.b2; r[2][1] = m.b3; r[3][1] = m.b4;
    r[0][2] = m.c1; r[1][2] = m.c2; r[2][2] = m.c3; r[3][2] = m.c4;
    r[0][3] = m.d1; r[1][3] = m.d2; r[2][3] = m.d3; r[3][3] = m.d4;
    return r;
}

// Posicion / escala en el tiempo t (en "ticks" de la animacion): interpolacion lineal entre keyframes
static glm::vec3 InterpolarVec(const aiVectorKey* claves, unsigned int n, double t)
{
    if (n == 0) return glm::vec3(0.0f);
    if (n == 1 || t <= claves[0].mTime) return glm::vec3(claves[0].mValue.x, claves[0].mValue.y, claves[0].mValue.z);
    for (unsigned int i = 0; i + 1 < n; i++)
    {
        if (t < claves[i + 1].mTime)
        {
            float f = (float)((t - claves[i].mTime) / (claves[i + 1].mTime - claves[i].mTime));
            glm::vec3 a(claves[i].mValue.x, claves[i].mValue.y, claves[i].mValue.z);
            glm::vec3 b(claves[i + 1].mValue.x, claves[i + 1].mValue.y, claves[i + 1].mValue.z);
            return a + (b - a) * f;
        }
    }
    const aiVector3D& u = claves[n - 1].mValue;
    return glm::vec3(u.x, u.y, u.z);
}

// Rotacion en el tiempo t: interpolacion esferica (slerp) entre keyframes
static glm::quat InterpolarRot(const aiQuatKey* claves, unsigned int n, double t)
{
    if (n == 0) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    auto aGlm = [](const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); };
    if (n == 1 || t <= claves[0].mTime) return glm::normalize(aGlm(claves[0].mValue));
    for (unsigned int i = 0; i + 1 < n; i++)
    {
        if (t < claves[i + 1].mTime)
        {
            float f = (float)((t - claves[i].mTime) / (claves[i + 1].mTime - claves[i].mTime));
            return glm::normalize(glm::slerp(aGlm(claves[i].mValue), aGlm(claves[i + 1].mValue), f));
        }
    }
    return glm::normalize(aGlm(claves[n - 1].mValue));
}

// Textura de 1 pixel con un color (para materiales sin imagen)
static GLuint TexturaColor(float r, float g, float b)
{
    unsigned char px[4] = { (unsigned char)(r * 255.0f + 0.5f), (unsigned char)(g * 255.0f + 0.5f),
                            (unsigned char)(b * 255.0f + 0.5f), 255 };
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

// ---------------------------------------------------------------
// Carga
// ---------------------------------------------------------------
ModeloEsqueletico::ModeloEsqueletico() {}
// El destructor no toca OpenGL (el contexto ya no existe al cerrar): llamar Liberar() antes de glfwTerminate()
ModeloEsqueletico::~ModeloEsqueletico() {}

void ModeloEsqueletico::Liberar()
{
    for (Malla& m : mallas)
    {
        if (m.vao) glDeleteVertexArrays(1, &m.vao);
        if (m.vbo) glDeleteBuffers(1, &m.vbo);
        if (m.ebo) glDeleteBuffers(1, &m.ebo);
    }
    for (GLuint id : texturasCreadas) glDeleteTextures(1, &id);
    texturasCreadas.clear();
    cacheTex.clear();
    mallas.clear();
    nodos.clear();
    global.clear();
    importer.FreeScene();
    escena = nullptr;
    animActual = -1;
}

bool ModeloEsqueletico::Cargar(const std::string& ruta)
{
    Liberar();
    // LimitBoneWeights: maximo 4 huesos por vertice (lo que usa AplicarPiel)
    escena = importer.ReadFile(ruta, aiProcess_Triangulate | aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights);
    if (!escena || (escena->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !escena->mRootNode)
    {
        std::cout << "ERROR::ASSIMP (Esqueleto):: " << importer.GetErrorString() << std::endl;
        escena = nullptr;
        return false;
    }

    size_t barra = ruta.find_last_of("/\\");
    directorio = (barra == std::string::npos) ? std::string(".") : ruta.substr(0, barra);

    // Jerarquia de nodos (padre antes que hijo) y la malla que cuelga de cada uno
    ConstruirNodos(escena->mRootNode, -1);
    global.assign(nodos.size(), glm::mat4(1.0f));
    inversaRaiz = glm::inverse(nodos[0].local);

    std::vector<std::pair<const aiMesh*, int>> pendientes;
    // segundo recorrido (en el mismo orden que ConstruirNodos) para ligar mallas con nodos
    struct Rec {
        static void ir(const aiNode* n, int& contador, std::vector<std::pair<const aiMesh*, int>>& out, const aiScene* e)
        {
            int mio = contador++;
            for (unsigned int i = 0; i < n->mNumMeshes; i++) out.push_back({ e->mMeshes[n->mMeshes[i]], mio });
            for (unsigned int i = 0; i < n->mNumChildren; i++) ir(n->mChildren[i], contador, out, e);
        }
    };
    int contador = 0;
    Rec::ir(escena->mRootNode, contador, pendientes, escena);
    for (auto& p : pendientes) CrearMalla(p.first, p.second);

    std::cout << "Esqueleto: " << ruta << " -> " << mallas.size() << " mallas, "
        << nodos.size() << " nodos, " << NumAnimaciones() << " animaciones" << std::endl;

    // Pose inicial
    animActual = -1;
    CalcularPose();
    AplicarPiel();
    return true;
}

void ModeloEsqueletico::ConstruirNodos(const aiNode* n, int padre)
{
    int mio = (int)nodos.size();
    Nodo nodo;
    nodo.nombre = n->mName.C_Str();
    nodo.local = ConvertirMatriz(n->mTransformation);
    nodo.padre = padre;
    nodos.push_back(nodo);
    for (unsigned int i = 0; i < n->mNumChildren; i++) ConstruirNodos(n->mChildren[i], mio);
}

int ModeloEsqueletico::BuscarNodo(const std::string& nombre) const
{
    for (size_t i = 0; i < nodos.size(); i++)
        if (nodos[i].nombre == nombre) return (int)i;
    return -1;
}

// Sube una imagen RGBA (ya decodificada) a la GPU
static GLuint SubirTextura(const unsigned char* img, int ancho, int alto)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ancho, alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

GLuint ModeloEsqueletico::TexturaDelMaterial(const aiMaterial* mat)
{
    // 1. Si el material tiene imagen (difusa), se carga
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString ruta;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &ruta);
        std::string archivo = ruta.C_Str();

        std::map<std::string, GLuint>::iterator ya = cacheTex.find(archivo);
        if (ya != cacheTex.end()) return ya->second;          // ya se subio (la usan varias mallas)

        GLuint id = 0;
        if (!archivo.empty() && archivo[0] == '*')
        {
            // 1a. Textura INCRUSTADA en el FBX: Assimp la nombra "*0", "*1"... (indice en escena->mTextures)
            unsigned int idx = (unsigned int)atoi(archivo.c_str() + 1);
            if (idx < escena->mNumTextures)
            {
                const aiTexture* tex = escena->mTextures[idx];
                int ancho = 0, alto = 0;
                unsigned char* img = nullptr;
                if (tex->mHeight == 0)                         // comprimida (PNG/JPG): mWidth = bytes del archivo
                    img = SOIL_load_image_from_memory((const unsigned char*)tex->pcData, (int)tex->mWidth, &ancho, &alto, 0, SOIL_LOAD_RGBA);
                if (img)
                {
                    id = SubirTextura(img, ancho, alto);
                    SOIL_free_image_data(img);
                }
                else if (tex->mHeight > 0)                     // sin comprimir: texeles BGRA
                {
                    std::vector<unsigned char> rgba((size_t)tex->mWidth * tex->mHeight * 4);
                    for (size_t i = 0; i < (size_t)tex->mWidth * tex->mHeight; i++)
                    {
                        rgba[i * 4 + 0] = tex->pcData[i].r; rgba[i * 4 + 1] = tex->pcData[i].g;
                        rgba[i * 4 + 2] = tex->pcData[i].b; rgba[i * 4 + 3] = tex->pcData[i].a;
                    }
                    id = SubirTextura(rgba.data(), (int)tex->mWidth, (int)tex->mHeight);
                }
            }
        }
        else
        {
            // 1b. Textura en archivo: el FBX a veces guarda rutas absolutas o con '\\', solo se usa el nombre
            std::string nombre = archivo;
            size_t b = nombre.find_last_of("/\\");
            if (b != std::string::npos) nombre = nombre.substr(b + 1);
            const std::string intentos[2] = { directorio + "/" + nombre, "images/" + nombre };   // junto al FBX, o en images/
            for (int k = 0; k < 2 && id == 0; k++)
            {
                int ancho, alto;
                unsigned char* img = SOIL_load_image(intentos[k].c_str(), &ancho, &alto, 0, SOIL_LOAD_RGBA);
                if (img) { id = SubirTextura(img, ancho, alto); SOIL_free_image_data(img); }
            }
        }

        if (id)
        {
            std::cout << "Esqueleto: textura cargada " << archivo << std::endl;
            texturasCreadas.push_back(id);
            cacheTex[archivo] = id;
            return id;
        }
        std::cout << "Esqueleto: no se pudo cargar la textura " << archivo << std::endl;
    }
    // 2. Si no, se usa el color difuso (Kd) del material
    std::cout << "Esqueleto: material SIN textura (" << directorio << "), se usa color liso" << std::endl;
    aiColor3D kd(0.8f, 0.8f, 0.8f);
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
    GLuint color = TexturaColor(kd.r, kd.g, kd.b);
    texturasCreadas.push_back(color);
    return color;
}

void ModeloEsqueletico::CrearMalla(const aiMesh* m, int nodoDeMalla)
{
    mallas.push_back(Malla());
    Malla& mesh = mallas.back();
    mesh.nodo = nodoDeMalla;

    unsigned int nv = m->mNumVertices;
    mesh.pos0.resize(nv);
    mesh.nor0.resize(nv);
    mesh.uv.assign(nv, glm::vec2(0.0f));
    mesh.hueso.assign(nv, glm::ivec4(0));
    mesh.peso.assign(nv, glm::vec4(0.0f));
    for (unsigned int i = 0; i < nv; i++)
    {
        mesh.pos0[i] = glm::vec3(m->mVertices[i].x, m->mVertices[i].y, m->mVertices[i].z);
        mesh.nor0[i] = m->HasNormals() ? glm::vec3(m->mNormals[i].x, m->mNormals[i].y, m->mNormals[i].z)
            : glm::vec3(0.0f, 1.0f, 0.0f);
        if (m->mTextureCoords[0]) mesh.uv[i] = glm::vec2(m->mTextureCoords[0][i].x, m->mTextureCoords[0][i].y);
    }

    // Huesos: nodo de la jerarquia, matriz offset y pesos de cada vertice
    for (unsigned int b = 0; b < m->mNumBones; b++)
    {
        const aiBone* hueso = m->mBones[b];
        mesh.nodoDeHueso.push_back(BuscarNodo(hueso->mName.C_Str()));
        mesh.offset.push_back(ConvertirMatriz(hueso->mOffsetMatrix));
        for (unsigned int j = 0; j < hueso->mNumWeights; j++)
        {
            unsigned int v = hueso->mWeights[j].mVertexId;
            float w = hueso->mWeights[j].mWeight;
            int hueco = -1;                                   // primer espacio libre (peso 0)
            for (int k = 0; k < 4; k++) if (mesh.peso[v][k] == 0.0f) { hueco = k; break; }
            if (hueco < 0)                                    // lleno: reemplaza el peso mas chico si este es mayor
            {
                int menor = 0;
                for (int k = 1; k < 4; k++) if (mesh.peso[v][k] < mesh.peso[v][menor]) menor = k;
                if (w > mesh.peso[v][menor]) hueco = menor;
            }
            if (hueco >= 0) { mesh.hueso[v][hueco] = (int)b; mesh.peso[v][hueco] = w; }
        }
    }
    for (unsigned int i = 0; i < nv; i++)                     // los 4 pesos suman 1
    {
        float s = mesh.peso[i].x + mesh.peso[i].y + mesh.peso[i].z + mesh.peso[i].w;
        if (s > 1e-6f) mesh.peso[i] /= s;
    }

    std::vector<GLuint> indices;
    for (unsigned int f = 0; f < m->mNumFaces; f++)
        for (unsigned int k = 0; k < m->mFaces[f].mNumIndices; k++)
            indices.push_back(m->mFaces[f].mIndices[k]);
    mesh.numIndices = (int)indices.size();

    mesh.textura = TexturaDelMaterial(escena->mMaterials[m->mMaterialIndex]);

    // Buffers: los vertices se actualizan cada cuadro (GL_DYNAMIC_DRAW)
    mesh.datos.assign((size_t)nv * 8, 0.0f);
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.datos.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);   // posicion
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(1);   // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);   // coordenadas de textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

// ---------------------------------------------------------------
// Animaciones
// ---------------------------------------------------------------
int ModeloEsqueletico::NumAnimaciones() const
{
    return escena ? (int)escena->mNumAnimations : 0;
}

std::string ModeloEsqueletico::NombreAnimacion(int indice) const
{
    if (indice < 0 || indice >= NumAnimaciones()) return "";
    std::string n = escena->mAnimations[indice]->mName.C_Str();
    size_t barra = n.find_last_of('|');                       // Blender: "Armadura|Aleteo"
    return (barra == std::string::npos) ? n : n.substr(barra + 1);
}

int ModeloEsqueletico::BuscarAnimacion(const std::string& nombre) const
{
    for (int i = 0; i < NumAnimaciones(); i++)
        if (NombreAnimacion(i) == nombre) return i;
    return -1;
}

bool ModeloEsqueletico::Reproducir(int indice, bool enBucle)
{
    if (indice < 0 || indice >= NumAnimaciones()) return false;
    animActual = indice;
    bucle = enBucle;
    tiempo = 0.0f;
    sucio = true;
    return true;
}

bool ModeloEsqueletico::Reproducir(const std::string& nombre, bool enBucle)
{
    return Reproducir(BuscarAnimacion(nombre), enBucle);
}

void ModeloEsqueletico::Actualizar(float dt)
{
    if (!escena) return;
    if (pausado && !sucio) return;          // en pausa: la pose ya esta calculada
    if (animActual >= 0 && !pausado)
    {
        const aiAnimation* a = escena->mAnimations[animActual];
        double tps = (a->mTicksPerSecond > 0.0) ? a->mTicksPerSecond : 25.0;
        float duracion = (float)(a->mDuration / tps);
        tiempo += dt;
        if (duracion > 0.0f)
        {
            if (bucle) tiempo = std::fmod(tiempo, duracion);
            else if (tiempo > duracion) tiempo = duracion;
        }
    }
    CalcularPose();
    AplicarPiel();
    sucio = false;
}

// Pasos 1 y 2: interpolar los keyframes y acumular la jerarquia
void ModeloEsqueletico::CalcularPose()
{
    const aiAnimation* anim = (animActual >= 0) ? escena->mAnimations[animActual] : nullptr;

    // canal de animacion de cada nodo (si tiene)
    std::vector<const aiNodeAnim*> canal(nodos.size(), nullptr);
    double ticks = 0.0;
    if (anim)
    {
        double tps = (anim->mTicksPerSecond > 0.0) ? anim->mTicksPerSecond : 25.0;
        ticks = (double)tiempo * tps;
        for (unsigned int c = 0; c < anim->mNumChannels; c++)
        {
            int idx = BuscarNodo(anim->mChannels[c]->mNodeName.C_Str());
            if (idx >= 0) canal[idx] = anim->mChannels[c];
        }
    }

    for (size_t i = 0; i < nodos.size(); i++)
    {
        glm::mat4 local = nodos[i].local;
        if (canal[i])
        {
            const aiNodeAnim* c = canal[i];
            glm::vec3 pos = InterpolarVec(c->mPositionKeys, c->mNumPositionKeys, ticks);
            glm::quat rot = InterpolarRot(c->mRotationKeys, c->mNumRotationKeys, ticks);
            glm::vec3 esc = (c->mNumScalingKeys > 0) ? InterpolarVec(c->mScalingKeys, c->mNumScalingKeys, ticks)
                : glm::vec3(1.0f);
            local = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), esc);
        }
        global[i] = (nodos[i].padre < 0) ? local : global[nodos[i].padre] * local;
    }
}

// Pasos 3 y 4: matriz final de cada hueso y mover los vertices
void ModeloEsqueletico::AplicarPiel()
{
    for (Malla& m : mallas)
    {
        size_t nv = m.pos0.size();
        if (nv == 0) continue;

        std::vector<glm::mat4> finales(m.nodoDeHueso.size(), glm::mat4(1.0f));
        for (size_t b = 0; b < finales.size(); b++)
            if (m.nodoDeHueso[b] >= 0)
                finales[b] = inversaRaiz * global[m.nodoDeHueso[b]] * m.offset[b];

        glm::mat4 sinHuesos = inversaRaiz * global[m.nodo];   // mallas sin huesos siguen a su nodo

        for (size_t i = 0; i < nv; i++)
        {
            glm::vec3 p, n;
            if (finales.empty())
            {
                p = glm::vec3(sinHuesos * glm::vec4(m.pos0[i], 1.0f));
                n = glm::vec3(sinHuesos * glm::vec4(m.nor0[i], 0.0f));
            }
            else
            {
                glm::vec4 pa(0.0f), na(0.0f);
                float total = 0.0f;
                for (int k = 0; k < 4; k++)
                {
                    float w = m.peso[i][k];
                    if (w <= 0.0f) continue;
                    const glm::mat4& M = finales[m.hueso[i][k]];
                    pa += w * (M * glm::vec4(m.pos0[i], 1.0f));
                    na += w * (M * glm::vec4(m.nor0[i], 0.0f));
                    total += w;
                }
                if (total > 0.0f) { p = glm::vec3(pa); n = glm::vec3(na); }
                else { p = m.pos0[i];     n = m.nor0[i]; }
            }
            float len = glm::length(n);
            if (len > 1e-6f) n /= len;
            float* d = &m.datos[i * 8];
            d[0] = p.x; d[1] = p.y; d[2] = p.z;
            d[3] = n.x; d[4] = n.y; d[5] = n.z;
            d[6] = m.uv[i].x; d[7] = m.uv[i].y;
        }
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m.datos.size() * sizeof(float), m.datos.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

// ---------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------
void ModeloEsqueletico::Dibujar(Shader& shader, const glm::mat4& model)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    for (const Malla& m : mallas)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m.textura);
        glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse1"), 0);
        glBindVertexArray(m.vao);
        glDrawElements(GL_TRIANGLES, m.numIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}