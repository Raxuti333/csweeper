#version 450

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 textureCoord;

uniform vec3 info;
uniform float wdh;
uniform float text[6];

out vec2 coord;

void main()
{
    gl_Position = vec4(((vertex.x + (gl_InstanceID / 1.5f)) / info.z + info.x) * wdh, vertex.y / info.z + info.y, 0.0f, 1.0f);

    coord = vec2(textureCoord.x + (0.125f) * (abs(text[gl_InstanceID]) - 8.0f * floor(abs(text[gl_InstanceID]) / 8.0f)), textureCoord.y + (0.125f) * floor(abs(text[gl_InstanceID]) / 8.0f));
}

