#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "csweeper.h"

vec2 screen = {640.f, 480.f};

Game game;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_size_callback(GLFWwindow* window, int x, int y);
unsigned int ftou(float* v, unsigned int l);

inline int inSquare(vec2 m, vec2 a, vec2 b)
{
    if(a[0] < m[0] && a[1] < m[1] && b[0] > m[0] && b[1] > m[1]) { return 1; }
    
    return 0;
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    if (!glfwInit()) { return -1; }

    window = glfwCreateWindow(640, 480, "csweep", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    if (!gladLoaderLoadGL()) { return -1; }

    unsigned int atlas = createAtlas(ATLAS, ATLAS_WIDTH, ATLAS_HEIGTH, sizeof(ATLAS));

    game = InitGame(FIELD, FIELD_FS_OFFSET, sizeof(FIELD), FIELD_I_LENGTH, screen);
    game.menu = initUI(BACKGROUND, BACKGROUND_FS_OFFSET, sizeof(BACKGROUND), BACKGROUND_I_LENGTH, TEXT, TEXT_FS_OFFSET, sizeof(TEXT), TEXT_I_LENGTH);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, atlas);
    glBindVertexArray(game.squareVAO);

    unsigned int viewloc = glGetUniformLocation(game.shader, "view");

    //#006fb2
    glClearColor(00.f/255.f, (float)(0x6f)/255.f, (float)(0xb2)/255.f, 1.0f);

    vec2 lastMouse;

    int lastFrame_left;
    int lastFrame_right;

    while (!glfwWindowShouldClose(window) && game.state != GAME_STATE_EXITING)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if(game.state == GAME_STATE_STARTING)
        {
            unsigned int width = ftou(game.menu.inputs[0].input_data + 1, (sizeof(game.menu.inputs[0].input_data)/sizeof(*game.menu.inputs[0].input_data)));
            unsigned int heigth = ftou(game.menu.inputs[1].input_data + 1, (sizeof(game.menu.inputs[0].input_data)/sizeof(*game.menu.inputs[0].input_data)));
            unsigned int mines = ftou(game.menu.inputs[2].input_data + 1, (sizeof(game.menu.inputs[0].input_data)/sizeof(*game.menu.inputs[0].input_data)));

            if(!startGame(&game, width, heigth, mines)) { game.state = 2; }
        }
        else if(game.state < GAME_STATE_IN_MENU)
        {
            glUniform4fv(viewloc, 1, game.camera);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, game.wh);

            glfwSwapBuffers(window);

            if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                if(lastMouse[0] == 0 && lastMouse[1] == 0);
                else
                {
                    game.camera[0] -= ((xpos - lastMouse[0]) / 50.f) / (screen[0] / 300.f);
                    game.camera[1] += ((ypos - lastMouse[1]) / 50.f) / (screen[1] / 300.f);
                }

                lastMouse[0] = xpos;
                lastMouse[1] = ypos;
            }
            else
            {
                lastMouse[0] = 0.f;
                lastMouse[1] = 0.f;
            }

            /* bit spaghetti click and game logic clean later */
            int thisFrame_left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            int thisFrame_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

            if(game.state == GAME_STATE_PLAYING && thisFrame_left != GLFW_RELEASE && thisFrame_left != lastFrame_left)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                float fieldz = (float)game.width;

                vec2 m = {(xpos - screen[0] / 2.f) / (screen[0] / 2.f), (ypos - screen[1] / 2.f) / (screen[1] / 2.f)};

                for(unsigned int i = 0; i < game.wh; ++i)
                {
                    float j = (float)i;

                    vec2 a = {((-0.5f + (j - fieldz * floorf(j / fieldz))) * game.camera[3] - game.camera[0]) / game.camera[2] , (-0.5 - floorf(j / fieldz) + game.camera[1]) / game.camera[2] };
                    vec2 b = {((0.5f + (j - fieldz * floorf(j / fieldz))) * game.camera[3] - game.camera[0]) / game.camera[2] , (0.5 - floorf(j / fieldz) + game.camera[1]) / game.camera[2]  };

                    if(inSquare(m, a, b)) 
                    {
                        if(fabsf(game.field[i]) == FLAG) { break; }

                        if(game.field[i] < 0) 
                        {
                            for(unsigned int k = 0; k < game.wh; ++k)
                            {
                                if(game.field[k] < 0) { game.field[k] = MINE; }
                            }
                            game.state = 0;
                            game.endTime = 0;
                        }
                        else if(game.field[i] != OPEN_TILE)
                        { 
                            unsigned int num = checkTile(i, &game);

                            if(!num) 
                            {
                                game.field[i] = OPEN_TILE; 

                                nextTile(i, &game);
                            }

                            else { game.field[i] = (float)num;}

                            int clear = 0;

                            for(unsigned int k = 0; k < game.wh; ++k)
                            {
                                if(game.field[k] == CLOSED_TILE || game.field[k] == FLAG) { ++clear; }
                            }

                            if(clear == 0)
                            {
                                game.state = 2;
                                game.endTime = time(NULL);

                                char tmp[10];

                                snprintf(tmp, sizeof(tmp),"T:%f", (double)(game.endTime - game.startTime));

                                /* CLEAN victory */

                                for(unsigned int t = 0; t < 6; ++t)
                                {
                                    if(tmp[t] == '.') 
                                    {
                                        game.menu.time.text_data[t] = FONT_DOT;
                                    }
                                    else if(tmp[t] == 'T')
                                    {
                                        game.menu.time.text_data[t] = FONT_T;
                                    }
                                    else if(tmp[t] == ':')
                                    {
                                        game.menu.time.text_data[t] = FONT_CL;
                                    }
                                    else if(tmp[t] >= '0' && tmp[t] <= '9')
                                    {
                                        game.menu.time.text_data[t] = (float)(tmp[t] - '0');
                                    }
                                }
                            }
                        }

                        glBindBuffer(GL_ARRAY_BUFFER, game.instanceVBO);
                        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game.wh, game.field, GL_DYNAMIC_DRAW);
                        glBindBuffer(GL_ARRAY_BUFFER, 0); 

                        break; 
                    }
                }
            }

            if(game.state == GAME_STATE_PLAYING && thisFrame_right != GLFW_RELEASE && thisFrame_right != lastFrame_right)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                float fieldz = (float)game.width;

                vec2 m = {(xpos - screen[0] / 2.f) / (screen[0] / 2.f), (ypos - screen[1] / 2.f) / (screen[1] / 2.f)};

                for(unsigned int i = 0; i < game.wh; ++i)
                {
                    float j = (float)i;

                    vec2 a = {((-0.5f + (j - fieldz * floorf(j / fieldz))) * game.camera[3] - game.camera[0]) / game.camera[2] , (-0.5 - floorf(j / fieldz) + game.camera[1]) / game.camera[2] };
                    vec2 b = {((0.5f + (j - fieldz * floorf(j / fieldz))) * game.camera[3] - game.camera[0]) / game.camera[2] , (0.5 - floorf(j / fieldz) + game.camera[1]) / game.camera[2]  };

                    if(inSquare(m, a, b)) 
                    {
                        if(fabsf(game.field[i]) == CLOSED_TILE)
                        {
                            if(game.field[i] < 0) { game.field[i] = -FLAG; }
                            else { game.field[i] = FLAG; }

                            glBindBuffer(GL_ARRAY_BUFFER, game.instanceVBO);
                            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game.wh, game.field, GL_DYNAMIC_DRAW);
                            glBindBuffer(GL_ARRAY_BUFFER, 0);
                        }
                        else if(fabsf(game.field[i]) == FLAG)
                        {
                            if(game.field[i] < 0) { game.field[i] = -CLOSED_TILE; }
                            else { game.field[i] = CLOSED_TILE; }

                            glBindBuffer(GL_ARRAY_BUFFER, game.instanceVBO);
                            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game.wh, game.field, GL_DYNAMIC_DRAW);
                            glBindBuffer(GL_ARRAY_BUFFER, 0);
                        }
                        break;
                    }
                }
            }

            if(game.state == GAME_STATE_SPECTATING && glfwGetKey(window, GLFW_KEY_ENTER)) { game.state = 2; }

            lastFrame_left = thisFrame_left;
            lastFrame_right = thisFrame_right;
        }
        else
        {
            glUseProgram(game.menu.bgShader);
            glBindVertexArray(game.menu.elementVAO);
            glUniform1f(game.menu.shaderLocation[3], game.camera[3]);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glUseProgram(game.menu.textShader);
            glBindVertexArray(game.menu.textVAO);

            glUniform1f(game.menu.shaderLocation[2], game.camera[3]);
            for(unsigned int i = 0; i < (sizeof(game.menu.inputs)/sizeof(*game.menu.inputs)); ++i)
            {
                glUniform3fv(game.menu.shaderLocation[0], 1, game.menu.inputs[i].XYS);
                glUniform1fv(game.menu.shaderLocation[1], 6, game.menu.inputs[i].input_data);

                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 6);
            }

            glUniform3fv(game.menu.shaderLocation[0], 1, game.menu.time.XYS);
            glUniform1fv(game.menu.shaderLocation[1], 6, game.menu.time.text_data);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 6);

            glfwSwapBuffers(window);

            if(glfwGetKey(window, GLFW_KEY_ESCAPE)) { game.state = GAME_STATE_EXITING; }
        }

        glfwPollEvents();
    }

    deleteGame(&game);

    gladLoaderUnloadGL();

    glfwTerminate();
    return 0;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    game.camera[2] += yoffset;
    if(game.camera[2] < 0.5f) { game.camera[2] = 0.5f; }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key >= GLFW_KEY_0 && key <= GLFW_KEY_9 && game.menu.inputs[game.menu.focus].i != 6 && action == GLFW_PRESS)
    {
        game.menu.inputs[game.menu.focus].input_data[game.menu.inputs[game.menu.focus].i++] = (float)(key - GLFW_KEY_0);
    }
    if(key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
    {
        if(game.menu.inputs[game.menu.focus].i != 1) { --game.menu.inputs[game.menu.focus].i; }
        game.menu.inputs[game.menu.focus].input_data[game.menu.inputs[game.menu.focus].i] = FONT_CL;
    }
    if(key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        if(game.menu.focus != 0) 
        { 
            game.menu.inputs[game.menu.focus].input_data[0] = OPEN_TILE; 
            game.menu.focus--; 
            game.menu.inputs[game.menu.focus].input_data[0] = ARROW;
        }
    }
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        if(game.menu.focus != 2) 
        { 
            game.menu.inputs[game.menu.focus].input_data[0] = OPEN_TILE; 
            game.menu.focus++; 
            game.menu.inputs[game.menu.focus].input_data[0] = ARROW;
        }
    }
    if(key == GLFW_KEY_ENTER && action == GLFW_PRESS && game.state == 2)
    {
        game.state = GAME_STATE_STARTING;
    }
}

unsigned int ftou(float* v, unsigned int l)
{
    unsigned int u = 0;

    for(unsigned int i = 0; i < l; ++i)
    {
        if(v[i] == FONT_CL) { break; }
        u = (u * 10) + (unsigned int)v[i];
    }

    return u;
}

void window_size_callback(GLFWwindow* window, int x, int y)
{
    screen[0] = (float)x;
    screen[1] = (float)y;

    game.camera[3] = screen[1] / screen[0];

    glViewport(0,0, x, y);
}