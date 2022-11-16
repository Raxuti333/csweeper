#version 450 core

in vec2 coord;

out vec4 color;

uniform sampler2D atlas;

void main()
{
    color = texture(atlas, coord);
}