// =============================================================
//  Texturas.cpp
//  Carga de texturas igual que en la practica de texturizado
//  (glGenTextures, glTexParameteri, stbi_load, glTexImage2D),
//  con dos correcciones:
//   - filtro de magnificacion GL_LINEAR (GL_NEAREST_MIPMAP_NEAREST no es valido)
//   - RGB o RGBA segun la imagen (jpg = 3 canales, png = 4)
// =============================================================
#include "Texturas.h"
#include <iostream>
#include <string>

// IMPORTANTE: este es el UNICO .cpp que incluye stb_image.h
// (su stb_image.h ya trae STB_IMAGE_IMPLEMENTATION adentro)
#include "stb_image.h"

GLuint CargarTextura(const char* ruta)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int textureWidth, textureHeight, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(ruta, &textureWidth, &textureHeight, &nrChannels, 0);
    if (image)
    {
        GLenum formato = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, formato, textureWidth, textureHeight, 0, formato, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << ruta << std::endl;
    }
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void UsarTextura(Shader& shader, GLuint textura, float repetirU, float repetirV)
{
    // Bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textura);
    glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse1"), 0);
    glUniform2f(glGetUniformLocation(shader.Program, "repetir"), repetirU, repetirV);
}

void UsarMapas(Shader& shader, GLuint normal, GLuint especular)
{
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, normal);
    glUniform1i(glGetUniformLocation(shader.Program, "normalMap"), 6);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, especular);
    glUniform1i(glGetUniformLocation(shader.Program, "specMap"), 7);
    glActiveTexture(GL_TEXTURE0);
    glUniform1f(glGetUniformLocation(shader.Program, "usarMapas"), 1.0f);
}

void QuitarMapas(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.Program, "usarMapas"), 0.0f);
}


GLuint CargarCubemap(const char* carpeta)
{
    const char* caras[6] = { "px", "nx", "py", "ny", "pz", "nz" };
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    stbi_set_flip_vertically_on_load(false);          // las caras del cubemap no se voltean
    for (int i = 0; i < 6; i++)
    {
        std::string ruta = std::string(carpeta) + "/" + caras[i] + ".png";
        int w, h, n;
        unsigned char* img = stbi_load(ruta.c_str(), &w, &h, &n, 3);
        if (img)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
            stbi_image_free(img);
        }
        else
        {
            std::cout << "No se encontro la cara del cubemap " << ruta << std::endl;
            unsigned char negro[3] = { 0, 0, 0 };
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, negro);
        }
    }
    stbi_set_flip_vertically_on_load(true);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return tex;
}
