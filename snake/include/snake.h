#ifndef SNAKE_H_
#define SNAKE_H_


#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <set>
#include <queue>
#include <vector>
#include <climits>
#include <sstream>
#include <algorithm>

#include "config.h"

class Food;

struct Point {
    int x, y;

    bool operator<(const Point &other) const {
        if (x == other.x) return y < other.y;
        return x < other.x; 
    }
};

enum class Direction {
    UNKNOWN = 0, UP, DOWN, LEFT, RIGHT
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
    
    void UpdateDirection(Direction newDirection);
    
    void Reset();
    
    // true表示有问题
    bool CheckCollision(int x, int y) const {
        if (x >= GAME_WIDTH || x < 0 || y >= GAME_HEIGHT || y < 0) return true;
        return occupied[y / PER_RECT][x / PER_RECT] > step - sLen + 1;
    }
    
    Direction GetDirection() const {
        return direction;
    }
    
private:
    // 自动模式下控制蛇的方向
    void Auto(const Point &foodPoint);
    
    int sLen;    // 蛇长
    int step;
    List *head;     // 蛇头
    List *tail;     // 蛇尾
    SnakeState state;
    Direction direction;
    std::vector<std::vector<int>> occupied; 

    SDL_Texture *textureHead;
    SDL_Texture *textureBody1;
    SDL_Texture *textureBody2;
    SDL_Texture *textureError;

    Mix_Music *musicEat;
    Mix_Music *musicCrash;
};

#endif  // SNAKE_H