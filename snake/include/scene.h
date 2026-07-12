#ifndef SRENE_H
#define SRENE_H

#include <SDL2/SDL.h>

#include <fstream>

#include "config.h"
#include "snake.h"
#include "food.h"
#include "render.h"

enum class GameState{
    Title, Gaming, Pause, GameOver, Quit
};

void PauseScene(SDL_Renderer *renderer, GameState &gameState);
void TitleScene(SDL_Renderer *renderer, GameState &gameState);
void GameOverScene(SDL_Renderer *renderer, GameState &gameState);
void GameScene(SDL_Renderer *renderer, GameState &gameState);

#endif  // SRENE_H