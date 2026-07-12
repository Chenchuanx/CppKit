#pragma once

#include <SDL2/SDL_ttf.h>

#include <unordered_map>
#include <map>

#define PER_RECT 20

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 480

#define GAME_WIDTH 600
#define GAME_HEIGHT 400

#define TITLE_PNG_PATH "./res/images/title.png"
#define TiTLE_PNG_W 528
#define TiTLE_PNG_H 156

#define FOOD_PNG_PATH "./res/images/food.png"
#define FOOD_PNG_W 20
#define FOOD_PNG_H 20


#define ERROR_PNG_PATH "./res/images/error.png"
#define ERROR_PNG_W 20
#define ERROR_PNG_H 20

#define HEAD_PNG_PATH "./res/images/snake_head.png"
#define HEAD_PNG_W 20
#define HEAD_PNG_H 20

#define BODY1_PNG_PATH "./res/images/snake_body1.png"
#define BODY2_PNG_PATH "./res/images/snake_body2.png"
#define BODY_PNG_W 20
#define BODY_PNG_H 20

#define ADD_PNG_PATH "./res/images/add.png"
#define SUB_PNG_PATH "./res/images/sub.png"
#define BUT_PNG_PATH "./res/images/button.png"
#define BUTTON_W 40
#define BUTTON_H 40

#define EAT_WAV_PATH "./res/audios/eat_apple.wav"
#define CRASH_WAV_PATH "./res/audios/snake_crash.wav"

#define TTF_FONT_PATH "./res/zpix.ttf"

#define LOG_PATH "./data/snake.log"

#define MAX_SCORE_FILE "./data/maxscore.txt"


extern int curScore;
extern int maxScore;

extern bool WallExit;
extern int TimeStep;

const int STEP_OFFSET  = 20;
const int MAX_TIMESTEP = 180;
const int MIN_TIMESTEP = 80;
