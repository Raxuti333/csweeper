#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#include "csweeper.h"

unsigned int createShader(const char* source, const size_t fs_offset, const unsigned int d_size, const unsigned int i_size)
{
    char* vs = decompress(source, d_size, i_size), *fs = vs + fs_offset;

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char * const*)&vs, NULL);
    glCompileShader(vertex);

    {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(vertex, GL_LINK_STATUS, &success);
        if(!success) { glGetShaderInfoLog(vertex, sizeof(infoLog), NULL, infoLog); printf("%s", infoLog); }
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char * const*)&fs, NULL);
    glCompileShader(fragment);

    {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(fragment, GL_LINK_STATUS, &success);
        if(!success) { glGetShaderInfoLog(fragment, sizeof(infoLog), NULL, infoLog); printf("%s", infoLog); }
    }

    unsigned int shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free(vs);

    return shader;
}