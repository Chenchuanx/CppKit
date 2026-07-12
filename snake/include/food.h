#ifndef FOOD_H
#define FOOD_H

#include <SDL2/SDL.h>

#include <utility>

#include "config.h"
#include "snake.h"

class Food {
public:
    Food(int score, const Snake &snake, SDL_Renderer *renderer);

    ~Food();

    void Draw(SDL_Renderer *renderer);
    
    void Reset(const Snake &snake);

    Point GetPoint() const {
        return {x, y};
    }

    int GetScore() const {
        return score;
    }

private:
    int x, y, score;
    SDL_Texture *textureFood;
};

#endif  // FOOD_H