#ifndef CSWEEPER_H
#define CSWEEPER_H

#include "atlas.h"
#include "shaders.h"
#include "ui.h"

#ifndef _WIN32
#include <time.h>
#endif

/* ATLAS INDEXES */
#define FONT_0          (float)0x0
#define FONT_1          (float)0x1
#define FONT_2          (float)0x2
#define FONT_3          (float)0x3
#define FONT_4          (float)0x4
#define FONT_5          (float)0x5
#define FONT_6          (float)0x6
#define FONT_7          (float)0x7
#define FONT_8          (float)0x8
#define FONT_9          (float)0x9
#define MINE            (float)-0xA
#define FLAG            (float)0xB
#define CLOSED_TILE     (float)0xC
#define OPEN_TILE       (float)0x3F
#define FONT_T          (float)0xE
#define FONT_CL         (float)0xF
#define FONT_DOT        (float)0x10
#define ARROW           (float)0x13

#define GAME_STATE_PLAYING      0x1
#define GAME_STATE_SPECTATING   0x0
#define GAME_STATE_IN_MENU      0x2
#define GAME_STATE_STARTING     0x3
#define GAME_STATE_EXITING      0x4

typedef struct Game 
{
    /* width of minefield */
    unsigned int width;

    /* heigth of minefield */
    unsigned int heigth;

    /* width * heigth */
    unsigned int wh;

    /* mine count */
    unsigned int mines;

    /* state of game */
    unsigned int state;

    unsigned int instanceVBO, squareVBO, squareVAO;

    unsigned int shader, uniformLocations[2];

    /* mine field array*/
    float* field;

    /* start and end time of game */
#ifdef _WIN32
    unsigned long startTime, endTime;
#else 
    struct timespec startTime, endTime;
#endif

    UI menu;

    vec4 camera;
} Game;

/**
 * @brief 
 * createAtlas prossecess a RGBA image data and turns it into opengl texture object
 * @param image compressed or decompressed image data
 * @param width width of image
 * @param heigth heigth of image
 * @param d_size leave 0 if image data is not compressed else compressed data size
 * @return opengl texture id
 */
unsigned int createAtlas(const unsigned char* image, const unsigned int width, const unsigned int heigth, const unsigned int d_size);

/**
 * @brief 
 * decompress decompresse data using zlib inflate
 * @param compressed compressed data
 * @param d_size compressed data size
 * @param i_size decompressed data size
 * @return decompressed data
 */
unsigned char* decompress(const unsigned char* compressed, const unsigned int d_size, const unsigned int i_size);

/**
 * @brief 
 * 
 * @param source compressed or decompressed source
 * @param fs_offset offset in bytes from begining of the file to start of fragment shader
 * @param d_size compressed data size
 * @param i_size decompressed data size
 * @return Shader
 */
unsigned int createShader(const char* source, const size_t fs_offset, const unsigned int d_size, const unsigned int i_size);

/**
 * @brief 
 * initializes game struct
 * @param source minefield shader compressed source
 * @param fs_offset minefield fragment shader offset
 * @param d_size compressed/deflated size
 * @param i_size uncompressed/inflated size
 * @return initialized game struct 
 */
Game InitGame(const char* source, const size_t fs_offset, const unsigned int d_size, const unsigned int i_size, vec2 screen);

/**
 * @brief 
 * Delete game object
 * 
 * @param game 
 * pointer initialized to game object
 */
void deleteGame(Game* game);

/**
 * @brief 
 * setup intialized game object
 * @param game pointer initialized to game object 
 * @param width width of minefield
 * @param heigth height of minefield
 * @param mines amount of mines if mines >= width * heigth fails
 */
int startGame(Game* game, const unsigned int width, const unsigned int heigth, const unsigned int mines);

/**
 * @brief 
 * checks tiles around i returns number of mines around tile
 * @param i index of tile
 * @param game pointer to game struct
 * @return number of mines around tile
 */
int checkTile(const unsigned int i, Game* game);

/**
 * @brief 
 * initializes ui struct
 * @param s1 background shader compressed sources
 * @param fs_offset1 background fragment shader offset
 * @param d_size1 compressed/deflated size
 * @param i_size1 uncompressed/inflated size
 * @param s2 text shader compressed sources
 * @param fs_offset2 background fragment shader offset
 * @param d_size2 compressed/deflated size
 * @param i_size2 uncompressed/inflated size
 * @return initialized UI struct
 */
UI initUI(const char* s1, const size_t fs_offset1, const unsigned int d_size1, const unsigned int i_size1, const char* s2, const size_t fs_offset2, const unsigned int d_size2, const unsigned int i_size2);

/**
 * @brief 
 * deletes initialized ui struct
 * @param ui pointer to UI struct
 */
void deleteUi(UI* ui);

/**
 * @brief
 * checks if any tile is clicked returns -1 if no else returns tile index
 * 
 * @param game pointer to game struct
 * @param mouse mouse position
 * @return returns -1 if not tile is clicked or open tiles is clicked else returns tile index
 */
unsigned int isTileClicked(Game* game, vec2 mouse);

/**
 * @brief 
 * sets game to its failed state
 * @param game pointer to game struct
 */
void gameFailed(Game* game);

/**
 * @brief 
 * opens tile and checks for game clear condition and opens empty tiles 
 * @param game pointer to game struct
 * @param i index of tile
 */
void openTile(Game* game, const unsigned int i);

/**
 * @brief 
 * returns a pseudo random 32bit unsigned integer
 * @param number non 0 seed value
 * @return random number 
 */
unsigned int random32(unsigned int number);

#endif