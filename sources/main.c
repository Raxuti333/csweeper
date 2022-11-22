#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <math.h>

#include "csweeper.h"

#ifdef _WIN32
#include "icon.h"
#include <stdlib.h>
#endif

vec2 screen = {640.f, 480.f};

Game game;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_size_callback(GLFWwindow* window, int x, int y);

unsigned int ftou(float* v, unsigned int l);

int main(int argc, char** argv)
{
    GLFWwindow* window;

    /* init glfw*/
    if (!glfwInit()) { return -1; }

    window = glfwCreateWindow(640, 480, "c-sweeper", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    #ifdef _WIN32
    /* decompress icon from icon.h */
    GLFWimage icon = {.width = ICON_WIDTH, .height = ICON_HEIGTH, decompress(ICON, sizeof(ICON), ICON_WIDTH * ICON_HEIGTH * 4U)};

    /* set icon as window icon */
    glfwSetWindowIcon(window, 1, &icon);

    free(icon.pixels);
    #endif

    glfwMakeContextCurrent(window);

    /* glad load opengl extensions */
    if (!gladLoaderLoadGL()) { return -1; }

    /* create and bind texture atlas */
    unsigned int atlas = createAtlas(ATLAS, ATLAS_WIDTH, ATLAS_HEIGTH, sizeof(ATLAS));
    glBindTexture(GL_TEXTURE_2D, atlas);

    /* init game */
    game = InitGame(FIELD, FIELD_FS_OFFSET, sizeof(FIELD), FIELD_I_LENGTH, screen);
    /* init menu */
    game.menu = initUI(BACKGROUND, BACKGROUND_FS_OFFSET, sizeof(BACKGROUND), BACKGROUND_I_LENGTH, TEXT, TEXT_FS_OFFSET, sizeof(TEXT), TEXT_I_LENGTH);

    /* set callbacks */
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    /* Enable blending for transparency */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* background color #006fb2 */
    glClearColor((float)(0x00)/255.f, (float)(0x6f)/255.f, (float)(0xb2)/255.f, 1.0f);

    /* last mouse position for allowing dragging the view */
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

            if(!startGame(&game, width, heigth, mines)) { game.state = GAME_STATE_IN_MENU; }
        }
        
        /* in game*/
        else if(game.state < GAME_STATE_IN_MENU)
        {
            glUniform4fv(game.uniformLocations[0], 1, game.camera);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, game.wh);

            glfwSwapBuffers(window);

            if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                if(lastMouse[0] == 0 && lastMouse[1] == 0);
                else
                {
                    game.camera[0] -= ((xpos - lastMouse[0]) / 50.f) / game.camera[3];
                    game.camera[1] += ((ypos - lastMouse[1]) / 50.f) / game.camera[3];
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

            if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)   { game.camera[1] += 0.1f; }
            if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { game.camera[1] -= 0.1f; }
            if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { game.camera[0] -= 0.1f; }
            if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){ game.camera[0] += 0.1f; }
            
            /* left click tile */
            if(game.state == GAME_STATE_PLAYING && thisFrame_left != GLFW_RELEASE && thisFrame_left != lastFrame_left)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                vec2 mouse = {(xpos - screen[0] / 2.f) / (screen[0] / 2.f), (ypos - screen[1] / 2.f) / (screen[1] / 2.f)};

                unsigned int i = isTileClicked(&game, mouse);

                if(i != (-1U) && fabsf(game.field[i]) != FLAG)
                {
                    if(game.field[i] < 0) { gameFailed(&game); }
                    else { openTile(&game, i); }

                    glBindBuffer(GL_ARRAY_BUFFER, game.instanceVBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game.wh, game.field, GL_DYNAMIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0); 
                }
            }

            /* rigth click tile */
            if(game.state == GAME_STATE_PLAYING && thisFrame_right != GLFW_RELEASE && thisFrame_right != lastFrame_right)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                float fieldz = (float)game.width;

                vec2 mouse = {(xpos - screen[0] / 2.f) / (screen[0] / 2.f), (ypos - screen[1] / 2.f) / (screen[1] / 2.f)};
                
                unsigned int i = isTileClicked(&game, mouse);

                if(i != (-1U)) 
                {
                    if(fabsf(game.field[i]) == CLOSED_TILE)
                    {
                        if(game.field[i] < 0) { game.field[i] = -FLAG; }
                        else { game.field[i] = FLAG; }
                    }

                    else if(fabsf(game.field[i]) == FLAG)
                    {
                        if(game.field[i] < 0) { game.field[i] = -CLOSED_TILE; }
                        else { game.field[i] = CLOSED_TILE; }
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, game.instanceVBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game.wh, game.field, GL_DYNAMIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }
            }

            if(game.state == GAME_STATE_SPECTATING && glfwGetKey(window, GLFW_KEY_ENTER)) { game.state = GAME_STATE_IN_MENU; }

            lastFrame_left = thisFrame_left;
            lastFrame_right = thisFrame_right;
        }
        
        /* in menu */
        else
        {
            /* render menu background */
            glUseProgram(game.menu.bgShader);
            glBindVertexArray(game.menu.elementVAO);
            glUniform1f(game.menu.shaderLocation[3], game.camera[3]);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glUseProgram(game.menu.textShader);
            glBindVertexArray(game.menu.textVAO);
            
            /* render input fields */
            glUniform1f(game.menu.shaderLocation[2], game.camera[3]);
            for(unsigned int i = 0; i < (sizeof(game.menu.inputs)/sizeof(*game.menu.inputs)); ++i)
            {
                glUniform3fv(game.menu.shaderLocation[0], 1, game.menu.inputs[i].XYS);
                glUniform1fv(game.menu.shaderLocation[1], 6, game.menu.inputs[i].input_data);

                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 6);
            }

            /* render time */
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