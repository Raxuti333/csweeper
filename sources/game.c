#include "glad/gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "csweeper.h"

Game InitGame(const char* source, const size_t fs_offset, const unsigned int d_size, const unsigned int i_size, vec2 screen)
{
    srand(time(NULL));

    const float square[] = 
    {
        /*X          Y          TX          TY*/
        -0.5f,        0.5f,       0.f,        0.f,
         0.5f,       -0.5f,       (1.f/8.f),  (1.f/8.f),
        -0.5f,       -0.5f,       0.f,        (1.f/8.f),
        /*X          Y          TX          TY*/
        -0.5f,       0.5f,       0.f,        0.f,
         0.5f,      -0.5f,       (1.f/8.f),  (1.f/8.f),
         0.5f,       0.5f,       (1.f/8.f),  0.f,
    };

    Game game;
    game.state = 2;
    game.shader = createShader(source, fs_offset, d_size, i_size);
    game.field = NULL;
    
    /* x y position */
    game.camera[0] = 0.f;
    game.camera[1] = 0.f;

    /* Zoom*/
    game.camera[2] = 10.f;

    /* aspect ratio*/
    game.camera[3] = screen[1] / screen[0];

    glGenBuffers(1, &game.instanceVBO);

    glGenVertexArrays(1, &game.squareVAO);
    glGenBuffers(1, &game.squareVBO);
    glBindVertexArray(game.squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, game.squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void*)(sizeof(float[2])));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, game.instanceVBO);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);
    glBindVertexArray(0);

    return game;
}

void deleteGame(Game* game)
{
    if(game == NULL) { return; }

    glDeleteVertexArrays(1, &game->squareVAO);
    glDeleteBuffers(1, &game->squareVBO);
    glDeleteBuffers(1, &game->instanceVBO);

    glDeleteProgram(game->shader);

    deleteUi(&game->menu);

    if(game->field != NULL) { free(game->field); game->field = NULL; }
}

int startGame(Game* game, const unsigned int width, const unsigned int heigth, const unsigned int mines)
{
    glUseProgram(game->shader);

    game->uniformLocations[0] = glGetUniformLocation(game->shader, "camera");
    game->uniformLocations[1] = glGetUniformLocation(game->shader, "field");

    /* set game data */
    game->heigth = heigth;
    game->width = width;
    game->wh = width * heigth;
    game->mines = mines;
    game->state = GAME_STATE_PLAYING;

    /* x y position */
    game->camera[0] = 0.f;
    game->camera[1] = 0.f;

    /* Zoom*/
    game->camera[2] = 10.f;

    /* check for valid game data */
    if(game->wh <= mines) { fputs("width * height <= mines\n", stderr); return 0;}
    if(game->wh > RAND_MAX) { fputs("width * height > RAND_MAX\n", stderr); return 0; }
    if(game->mines == 0) { fputs("mines == 0", stderr); return 0; }

    if(game->field != NULL) { free(game->field); game->field = NULL; }
    game->field = malloc(game->wh * sizeof(*game->field));
    if(game->field == NULL) { fputs("in function startGame(): game->field = malloc(game->wh * sizeof(*game->field)); failed", stderr); return 0; }

    /* set all tiles to closed tiles */
    for(unsigned int i = 0; i < game->wh; ++i) { game->field[i] = CLOSED_TILE; }

    /* set random tiles as mines */
    for(unsigned int c = 0; c < mines; ++c)
    {
        unsigned int index = rand() % game->wh;

        if(game->field[index] < 0) { --c;  continue; }
        game->field[index] *= -1.f; 
    }

    glUniform2f(game->uniformLocations[1], (float)width, (float)heigth);

    glBindVertexArray(game->squareVAO);

    glBindBuffer(GL_ARRAY_BUFFER, game->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game->wh, game->field, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    game->startTime = time(NULL);

    return 1;
}

int checkTile(const unsigned int i, Game* game)
{
    if(i < 0 || i > game->wh - 1) { return 0; }

    int num = 0;

    if(i >= game->width && game->field[i - game->width] < 0) { ++num; }
    if(i < game->wh - game->width && game->field[i + game->width] < 0) { ++num; }
    if(i != 0 && i % game->width != 0 && game->field[i - 1] < 0) { ++num; }
    if(i != game->wh - 1 && (i + 1) % game->width != 0 && game->field[i + 1] < 0) { ++num; }

    if(i > game->width && i % game->width != 0 && game->field[i - game->width - 1] < 0) { ++num; }
    if(i >= game->width && (i + 1) % game->width != 0 && game->field[i - game->width + 1] < 0) { ++num; }
    if(i < game->wh - game->width && (i + 1) % game->width != 0 && game->field[i + game->width + 1] < 0) { ++num; }
    if(i < game->wh - game->width && i % game->width != 0 && game->field[i + game->width - 1] < 0) { ++num; }

    return num;
}

void nextTile(const unsigned int i, Game* game)
{
    if(i < 0 || i > game->wh - 1) { return; }

    if(i >= game->width && game->field[i - game->width] == CLOSED_TILE)
    {
        int j = i - game->width;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }

    if(i < game->wh - game->width && game->field[i + game->width] == CLOSED_TILE) 
    { 
        int j = i + game->width;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }

    if(i != 0 && i % game->width != 0 && game->field[i - 1] == CLOSED_TILE) 
    {
        int j = i - 1;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }

    if(i != game->wh - 1 && (i + 1) % game->width != 0 && game->field[i + 1] == CLOSED_TILE) 
    {
        int j = i + 1;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }

    if(i > game->width && i % game->width != 0 && game->field[i - game->width - 1] == CLOSED_TILE) 
    {
        int j = i - game->width - 1;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }

    if(i >= game->width && (i + 1) % game->width != 0 && game->field[i - game->width + 1] == CLOSED_TILE) 
    {
        int j = i - game->width + 1;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }

    if(i < game->wh - game->width && (i + 1) % game->width != 0 && game->field[i + game->width + 1] == CLOSED_TILE) 
    {
        int j = i + game->width + 1;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }
    
    if(i < game->wh - game->width && i % game->width != 0 && game->field[i + game->width - 1] == CLOSED_TILE) 
    {
        int j = i + game->width - 1;
        int num = checkTile(j, game);

        if(!num)
        {
            game->field[j] = OPEN_TILE;
            nextTile(j, game);
        }
        else
        {
            game->field[j] = (float)num;
        }
    }
}

unsigned int isTileClicked(Game* game, vec2 mouse)
{
    for(unsigned int i = 0; i < game->wh; ++i)
    {
        float j = (float)i;
        float w = (float)game->width;

        vec2 a = {((-0.5f + (j - w * floorf(j / w))) * game->camera[3] - game->camera[0]) / game->camera[2] , (-0.5 - floorf(j / w) + game->camera[1]) / game->camera[2] };
        vec2 b = {((0.5f + (j - w * floorf(j / w))) * game->camera[3] - game->camera[0]) / game->camera[2] , (0.5 - floorf(j / w) + game->camera[1]) / game->camera[2]  };

        if(a[0] < mouse[0] && a[1] < mouse[1] && b[0] > mouse[0] && b[1] > mouse[1])
        {
            if(game->field[i] == OPEN_TILE) { break; }

            return i;
        }
    }

    return (-1U);
}

void openTile(Game* game, const unsigned int i)
{
    unsigned int num = checkTile(i, game);

    if(!num) 
    {
        game->field[i] = OPEN_TILE; 

        nextTile(i, game);
    }

    else { game->field[i] = (float)num;}

    int cleared = 0;

    /* loop trough tiles */
    for(unsigned int i = 0; i < game->wh; ++i)
    {
        if(game->field[i] == CLOSED_TILE || game->field[i] == FLAG) { ++cleared; }
    }

    /* if cleared is 0 all mines have been found*/
    if(cleared == 0)
    {
        game->state = GAME_STATE_IN_MENU;
        game->endTime = time(NULL);

        char tmp[10];

        snprintf(tmp, sizeof(tmp),"T:%f", (double)(game->endTime - game->startTime));

        for(unsigned int t = 0; t < 6; ++t)
        {
            if(tmp[t] == '.') 
            {
                game->menu.time.text_data[t] = FONT_DOT;
            }
            else if(tmp[t] == 'T')
            {
                game->menu.time.text_data[t] = FONT_T;
            }
            else if(tmp[t] == ':')
            {
                game->menu.time.text_data[t] = FONT_CL;
            }
            else if(tmp[t] >= '0' && tmp[t] <= '9')
            {
                game->menu.time.text_data[t] = (float)(tmp[t] - '0');
            }
        }
    }
}

void gameFailed(Game* game)
{
    for(unsigned int i = 0; i < game->wh; ++i)
    {
        if(game->field[i] < 0) { game->field[i] = MINE; }
    }

    game->state = GAME_STATE_SPECTATING;
    game->endTime = 0;
}