#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <map>
#include <unordered_map>
#include <fstream>
#include <time.h>

#include "log.h"
#include "config.h"
#include "food.h"
#include "snake.h"
#include "scene.h"

int curScore = 0;
int maxScore = 0;
int TimeStep = MIN_TIMESTEP - 40;
bool WallExit = true;
bool AutoRun = true;

void Init()
{
    srand((unsigned)time(NULL));

    InitLogFile(LOG_PATH);
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100 , MIX_DEFAULT_FORMAT, 2, 1024);

    font08 = TTF_OpenFont(TTF_FONT_PATH, 8);
    font16 = TTF_OpenFont(TTF_FONT_PATH, 16);
    font24 = TTF_OpenFont(TTF_FONT_PATH, 24);
    font32 = TTF_OpenFont(TTF_FONT_PATH, 32);
    font64 = TTF_OpenFont(TTF_FONT_PATH, 64);

    // 读取最高分
    std::ifstream fin(MAX_SCORE_FILE);
    if (!fin.is_open())  maxScore = 0;
    fin >> maxScore;
    fin.close();
}

void Quit(SDL_Window *window, SDL_Renderer *renderer) {
    
    SDL_Texture *texture = nullptr;
    for (auto &ky : imgTextureMap) {
        texture = ky.second;
        SDL_DestroyTexture(texture);
    }
    for (auto &ky : fontTextureMap) {
        texture = ky.second;
        SDL_DestroyTexture(texture);
    }
    
    TTF_CloseFont(font08);
    TTF_CloseFont(font16);
    TTF_CloseFont(font24);
    TTF_CloseFont(font32);
    TTF_CloseFont(font64);

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    CloseLogFile();
}

int main(int argc, char *argv[])
{
    Init();
    
    SDL_Window *window = SDL_CreateWindow("Snake", 200, 200, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_StopTextInput();    // 关闭文本输入

    GameState gameState = GameState::Title;
    
    while (gameState != GameState::Quit) {
        switch (gameState) {
            case GameState::Title : {
                TitleScene(renderer, gameState);
            } break;
            case GameState::Gaming : {
                GameScene(renderer, gameState);
            } break;
            case GameState::GameOver : {
                GameOverScene(renderer, gameState);
            } break;
            case GameState::Pause : {
                PauseScene(renderer, gameState);
            } break;
            case GameState::Quit : break;
        }
    }

    Quit(window, renderer);

    return 0;
}