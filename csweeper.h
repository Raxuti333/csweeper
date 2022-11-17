#ifndef CSWEEPER_H
#define CSWEEPER_H

#include "atlas.h"
#include "shaders.h"
#include "ui.h"
#include <time.h>

/* ATLAS INDEXES*/
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

typedef struct Game 
{
    unsigned int width, heigth, wh, mines, state;

    unsigned int instanceVBO, squareVBO, squareVAO, shader;
    float* field;

    time_t startTime, endTime;
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
 * Initialize game object
 */
Game InitGame(const char* source, const size_t fs_offset, const unsigned int d_size, const unsigned int i_size);

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
 * 
 * @param i 
 * @param game 
 * @return int 
 */
int checkTile(const unsigned int i, Game* game);

void nextTile(const unsigned int i, Game* game);

/**
 * @brief 
 * 
 * @param s1 
 * @param fs_offset1 
 * @param d_size1 
 * @param i_size1 
 * @param s2 
 * @param fs_offset2 
 * @param d_size2 
 * @param i_size2 
 * @return UI 
 */
UI initUI(const char* s1, const size_t fs_offset1, const unsigned int d_size1, const unsigned int i_size1, const char* s2, const size_t fs_offset2, const unsigned int d_size2, const unsigned int i_size2);

#endif