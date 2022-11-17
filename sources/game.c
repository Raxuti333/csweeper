#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "csweeper.h"

Game InitGame(const char* source, const size_t fs_offset, const unsigned int d_size, const unsigned int i_size)
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
    glDeleteVertexArrays(1, &game->squareVAO);
    glDeleteBuffers(1, &game->squareVBO);
    glDeleteBuffers(1, &game->instanceVBO);

    if(game->field != NULL) { free(game->field); game->field = NULL; }
}

int startGame(Game* game, const unsigned int width, const unsigned int heigth, const unsigned int mines)
{
    glUseProgram(game->shader);

    game->heigth = heigth;
    game->width = width;
    game->wh = width * heigth;
    game->mines = mines;
    game->state = 1;

    if(game->wh <= mines) { fputs("width * height <= mines", stderr); return 0;}
    if(game->wh > RAND_MAX) { fputs("width * height > RAND_MAX", stderr); return 0; }

    game->field = malloc(game->wh * sizeof(*game->field));
    if(game->field == NULL) { fputs("in function startGame(): game->field = malloc(game->wh * sizeof(*game->field)); failed", stderr); return 0; }

    for(unsigned int i = 0; i < game->wh; ++i)
    {
        game->field[i] = CLOSED_TILE;
    }

    for(unsigned int c = 0; c < mines; ++c)
    {
        unsigned int index = rand() % game->wh;

        if(game->field[index] < 0) { --c;  continue; }
        game->field[index] *= -1.f; 
    }

    glUniform2f(glGetUniformLocation(game->shader, "field"), (float)width, (float)heigth);

    glBindVertexArray(game->squareVAO);

    glBindBuffer(GL_ARRAY_BUFFER, game->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * game->wh, game->field, GL_DYNAMIC_DRAW);

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