#include "glad/gl.h"
#include <string.h>

#include "csweeper.h"


UI initUI(const char* s1, const size_t fs_offset1, const unsigned int d_size1, const unsigned int i_size1, const char* s2, const size_t fs_offset2, const unsigned int d_size2, const unsigned int i_size2)
{
    const float square[] = 
    {
        /*X          Y          TX          TY*/
        -0.5f,        0.5f,      0.f,         0.f,
         0.5f,       -0.5f,      (1.f/8.f),   (1.f/8.f),
        -0.5f,       -0.5f,      0.f,         (1.f/8.f),
        /*X          Y          TX          TY*/
        -0.5f,       0.5f,       0.f,         0.f,
         0.5f,      -0.5f,       (1.f/8.f),   (1.f/8.f),
         0.5f,       0.5f,       (1.f/8.f),   0.f,
    };

    const float background[] = 
    {
        /*X          Y          TX          TY*/
        -1.0f,        1.0f,      (1.f/8.f),         2.f*(1.f/8.f),
         1.0f,       -1.0f,      3.f*(1.f/8.f),   4.f*(1.f/8.f),
        -1.0f,       -1.0f,      (1.f/8.f),         4.f*(1.f/8.f),
        /*X          Y          TX          TY*/
        -1.0f,       1.0f,       (1.f/8.f),         2.f*(1.f/8.f),
         1.0f,      -1.0f,       3.f*(1.f/8.f),   4.f*(1.f/8.f),
         1.0f,       1.0f,       3.f*(1.f/8.f),   2.f*(1.f/8.f),
    };

    UI ui;
    ui.bgShader = createShader(s1, fs_offset1, d_size1, i_size1);
    ui.textShader = createShader(s2, fs_offset2, d_size2, i_size2);
    ui.focus = 0;

    glGenVertexArrays(1, &ui.elementVAO);
    glGenBuffers(1, &ui.elementVBO);
    glBindVertexArray(ui.elementVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ui.elementVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(background), background, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void*)(sizeof(float[2])));
    glBindVertexArray(0);

    glGenVertexArrays(1, &ui.textVAO);
    glGenBuffers(1, &ui.textVBO);
    glBindVertexArray(ui.textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ui.textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void*)(sizeof(float[2])));
    glBindVertexArray(0);

    ui.shaderLocation[0] = glGetUniformLocation(ui.textShader, "info");
    ui.shaderLocation[1] = glGetUniformLocation(ui.textShader, "text");
    ui.shaderLocation[2] = glGetUniformLocation(ui.textShader, "wdh");
    ui.shaderLocation[3] = glGetUniformLocation(ui.bgShader, "wdh");

    memset(&ui.inputs[0], 0, sizeof(ui.inputs[0]));
    memset(&ui.inputs[1], 0, sizeof(ui.inputs[1]));
    memset(&ui.inputs[2], 0, sizeof(ui.inputs[1]));

    memset(&ui.time, 0, sizeof(ui.time));

    memcpy(ui.time.XYS, textXYS, sizeof(textXYS));

    for(unsigned int i = 0; i < (sizeof(ui.inputs)/sizeof(*ui.inputs)); ++i)
    {
        for(unsigned int j = 0; j < (sizeof(ui.inputs[0].input_data)/sizeof(*ui.inputs[0].input_data)); ++j)
        {
            ui.inputs[i].input_data[j] = FONT_CL;
        }
        memcpy(ui.inputs[i].XYS, inputXYS[i], sizeof(*inputXYS));
    }

    ui.inputs[0].i = 1;
    ui.inputs[0].input_data[0] = ARROW;

    ui.inputs[1].i = 1;
    ui.inputs[1].input_data[0] = OPEN_TILE;
    
    ui.inputs[2].i = 1;
    ui.inputs[2].input_data[0] = OPEN_TILE;

    return ui;
}

void deleteUi(UI* ui)
{
    if(ui == NULL) { return; }

    glDeleteProgram(ui->bgShader);
    glDeleteProgram(ui->textShader);

    glDeleteVertexArrays(1, &ui->elementVBO);
    glDeleteVertexArrays(1, &ui->textVAO);
    glDeleteBuffers(1, &ui->elementVBO);
    glDeleteBuffers(1, &ui->textVBO);
}