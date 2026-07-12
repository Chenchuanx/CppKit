#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>

#include <sstream>

#include "config.h"
#include "snake.h"
#include "food.h"

extern TTF_Font *font08;
extern TTF_Font *font16;
extern TTF_Font *font24;
extern TTF_Font *font32;
extern TTF_Font *font64;

extern std::unordered_map<std::string, SDL_Texture*> imgTextureMap;
extern std::map<std::pair<int, std::string>, SDL_Texture*> fontTextureMap;

// 渲染工具函数
void ScreenPrint(SDL_Renderer *renderer, int ptSize, const std::string &text, SDL_Rect rect);
void ScreenImg(SDL_Renderer *renderer, std::string filePath, SDL_Rect &rect);

// 渲染界面
void RenderTitleScene(SDL_Renderer *renderer);
void RenderGameScene(SDL_Renderer *renderer, Food &food, Snake &snake);
void RenderGameOverScene(SDL_Renderer *renderer);
void RenderPauseScene(SDL_Renderer *renderer);

#endif  // RENDER_H