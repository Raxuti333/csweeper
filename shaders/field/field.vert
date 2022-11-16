#version 450 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 textureCoord;
layout (location = 2) in float textureIndex;

uniform vec4 field;
uniform vec4 view;

out vec2 coord;

void main()
{
    float x = ((vertex.x + (gl_InstanceID - field.z * floor(gl_InstanceID / field.z)) + field.x) * view.w - view.x) / view.z;
    float y = (vertex.y + floor(gl_InstanceID / field.z) + field.y - view.y) / view.z;

    gl_Position = vec4(x, y, 0.0f, 1.0f);

    coord = vec2(textureCoord.x + (0.125f) * (abs(textureIndex) - 8.0f * floor(abs(textureIndex) / 8.0f)), textureCoord.y + (0.125f) * floor(abs(textureIndex) / 8.0f));
}