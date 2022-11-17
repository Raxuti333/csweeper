#ifndef UI_H
#define UI_H

#include <cglm/cglm.h>

static const vec3 inputXYS[3] = {{0.022f, -0.025f, 7.0f}, {-0.6f, -0.025f, 7.0f}, {-0.2875f, -0.345f, 7.0f}};
static const vec3 textXYS = {0.425f, 0.85f, 7.25f};

typedef struct Input 
{
    unsigned int i;
    float input_data[6];

    vec3 XYS;
} Input;

typedef struct Text 
{
    float text_data[6];

    vec3 XYS;
} Text;

typedef struct UI 
{
    unsigned int focus;

    unsigned int shaderLocation[4];

    Input inputs[3];

    Text time;

    unsigned int bgShader, elementVAO, elementVBO;
    unsigned int textShader, textVAO, textVBO;
} UI;

#endif