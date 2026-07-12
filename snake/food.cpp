#include "food.h"

Food::Food(int score, const Snake &snake, SDL_Renderer *renderer) : score(score) {
    Reset(snake);
    SDL_Surface *surface = IMG_Load(FOOD_PNG_PATH);
    textureFood = SDL_CreateTextureFromSurface(renderer, surface);
}

Food::~Food() {
    SDL_DestroyTexture(textureFood);
}

void Food::Draw(SDL_Renderer *renderer) {
    SDL_Log("[Snake Reset] start ...");
    
    SDL_Rect rect = {x , y, PER_RECT, PER_RECT};
    SDL_RenderCopy(renderer, textureFood, NULL, &rect);
}

void Food::Reset(const Snake &snake) {
    SDL_Log("[Snake Reset] start ...");
    do {
        x = (rand() % (GAME_WIDTH / PER_RECT)) * PER_RECT;
        y = (rand() % (GAME_HEIGHT / PER_RECT)) * PER_RECT;
    } while (snake.CheckCollision(x, y));
}
