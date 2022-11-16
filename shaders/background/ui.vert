#version 450 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 textureCoord;

out vec2 coord;

void main()
{
    gl_Position = vec4(vertex.xy, 0.0f, 1.0f);

    coord = textureCoord;
}