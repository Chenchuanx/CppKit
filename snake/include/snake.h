#ifndef SNAKE_H_
#define SNAKE_H_


#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <queue>

#include "config.h"

class Food;

struct Point {
    int x, y;
};

enum class Direction {
    UP, DOWN, LEFT, RIGHT
};

enum class SnakeState {
    MOVING, EATING, CRASH_WALL, CRASH_SELF
};

class Snake {
public: 

private:
    struct Body {
        int x, y;
        Direction dir;

        Body(int x, int y, Direction dir) :
            x(x), y(y), dir(dir)
        {}
    };

    struct List {
        Body body;
        List *prev;
        List *next;

        List(int x, int y, Direction dir, List *prev, List *next) :
            body({x, y, dir}), prev(prev), next(next)
        {}
    };

public:
    Snake(SDL_Renderer *renderer);

    ~Snake();

    SnakeState Move(const Point &foodPoint, int foodScore);
    
    void Draw(SDL_Renderer *renderer) const;
    
    void UpdateDirection(Direction dir);
    
    void Reset();
    
    bool CheckCollision(int x, int y) const {
        if (x >= GAME_WIDTH || x < 0 || y >= GAME_HEIGHT || y < 0) return false;
        return occupy[y / PER_RECT][x / PER_RECT];
    }
    
    Direction GetDirection() const {
        return direction;
    }

private:

    int len;    // 蛇长
    List *head;     // 蛇头
    List *tail;     // 蛇尾
    SnakeState state;
    Direction direction;
    std::vector<std::vector<bool>> occupy; 

    SDL_Texture *textureHead;
    SDL_Texture *textureBody1;
    SDL_Texture *textureBody2;
    SDL_Texture *textureError;

    Mix_Music *musicEat;
    Mix_Music *musicCrash;
};

#endif  // SNAKE_H