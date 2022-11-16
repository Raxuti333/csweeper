#ifndef UI_H
#define UI_H

#include <cglm/cglm.h>

typedef struct Input 
{
    unsigned int i;
    float input_data[6];

    vec4 info;
} Input;

typedef struct Text 
{
    float text_data[6];

    vec4 info;
} Text;

typedef struct UI 
{
    unsigned int focus;

    unsigned int shaderLocation[2];

    Input inputs[3];

    Text time;

    unsigned int bgShader, elementVAO, elementVBO;
    unsigned int textShader, textVAO, textVBO;
} UI;

#endif