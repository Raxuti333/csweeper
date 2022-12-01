#version 450

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 textureCoord;
layout (location = 2) in float textureIndex;

uniform vec2 field;
uniform vec4 camera;

out vec2 coord;

void main()
{
    float x = ((vertex.x + (gl_InstanceID - field.x * floor(gl_InstanceID / field.x))) * camera.w - camera.x) / camera.z;
    float y = (vertex.y + floor(gl_InstanceID / field.x) - camera.y) / camera.z;

    gl_Position = vec4(x, y, 0.0f, 1.0f);

    coord = vec2(textureCoord.x + (0.125f) * (abs(textureIndex) - 8.0f * floor(abs(textureIndex) / 8.0f)), textureCoord.y + (0.125f) * floor(abs(textureIndex) / 8.0f));
}

