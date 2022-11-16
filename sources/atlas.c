#include <GL/glew.h>
#include <stdlib.h>
#include <math.h>

#include "csweeper.h"

unsigned int createAtlas(const unsigned char* img, const unsigned int width, const unsigned int heigth, const unsigned int d_size)
{
    unsigned char* data = (unsigned char*)img;

    if(d_size != 0) { data = decompress(img, d_size, width * heigth * 4U); }

    unsigned int texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, heigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if(d_size != 0) { free(data); }

    return texture;
}