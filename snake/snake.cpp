#include "snake.h"

Snake::Snake(SDL_Renderer *renderer) {
    len = 1;
    head = new List(GAME_WIDTH / 2, GAME_HEIGHT / 2, Direction::RIGHT, nullptr, nullptr);
    tail = head;
    state = SnakeState::MOVING;
    direction = Direction::RIGHT;
    occupy =
        std::vector<std::vector<bool>>(GAME_HEIGHT / PER_RECT, std::vector<bool>(GAME_WIDTH / PER_RECT, false));
    occupy[head->body.y / PER_RECT][head->body.x / PER_RECT] = true;

    SDL_Surface *surfaceHead = IMG_Load(HEAD_PNG_PATH);
    SDL_Surface *surfaceBody1 = IMG_Load(BODY1_PNG_PATH);
    SDL_Surface *surfaceBody2 = IMG_Load(BODY2_PNG_PATH);
    SDL_Surface *surfaceError = IMG_Load(ERROR_PNG_PATH);

    textureHead = SDL_CreateTextureFromSurface(renderer, surfaceHead);
    textureBody1 = SDL_CreateTextureFromSurface(renderer, surfaceBody1);
    textureBody2 = SDL_CreateTextureFromSurface(renderer, surfaceBody2);
    textureError = SDL_CreateTextureFromSurface(renderer, surfaceError);
    
    musicEat = Mix_LoadMUS(EAT_WAV_PATH);
    musicCrash = Mix_LoadMUS(CRASH_WAV_PATH);

    SDL_FreeSurface(surfaceError);
    SDL_FreeSurface(surfaceBody2);
    SDL_FreeSurface(surfaceBody1);
    SDL_FreeSurface(surfaceHead);
}

Snake::~Snake() {
    Mix_FreeMusic(musicEat);
    SDL_DestroyTexture(textureError);
    SDL_DestroyTexture(textureBody2);
    SDL_DestroyTexture(textureBody1);
    SDL_DestroyTexture(textureHead);
}

SnakeState Snake::Move(const Point &foodPoint, int foodScore) {
    SDL_Log("[Snake Move] Start ...");
    if (state != SnakeState::MOVING && state != SnakeState::EATING) {
        return state;
    }
    
    // 头前进
    SDL_Log("[Snake Move] Head");
    List* node = new List(head->body.x, head->body.y, direction, nullptr, nullptr);
    
    switch (direction) {
        case Direction::UP : {
            node->body.y -= PER_RECT;
        } break;
        case Direction::DOWN : {
            node->body.y += PER_RECT;
        } break;
        case Direction::LEFT : {
            node->body.x -= PER_RECT;
        } break;
        case Direction::RIGHT : {
            node->body.x += PER_RECT;
        } break;
    }
    if (!WallExit) {
        node->body.x = (node->body.x + GAME_WIDTH) % GAME_WIDTH;
        node->body.y = (node->body.y + GAME_HEIGHT) % GAME_HEIGHT;
    }
    
    // 撞墙
    if (node->body.x >= GAME_WIDTH || node->body.x < 0 || node->body.y >= GAME_HEIGHT || node->body.y < 0) {
        SDL_Log("[Snake Move] Crash Wall");
        delete(node);
        Mix_PlayMusic(musicCrash, 1);
        state = SnakeState::CRASH_WALL;
        return state;
    }
    // 吃自己
    if (occupy[node->body.y / PER_RECT][node->body.x / PER_RECT]) {
        SDL_Log("[Snake Move] Crash Self");
        delete(node);
        Mix_PlayMusic(musicCrash, 1);
        state = SnakeState::CRASH_SELF;
        return state;
    }

    // 头前进
    node->next = head;
    head->prev = node;
    head = node;
    occupy[head->body.y / PER_RECT][head->body.x / PER_RECT] = true;

    // 吃到果（尾需要保留）
    int foodX = foodPoint.x;
    int foodY = foodPoint.y;
    if (foodX == node->body.x && foodY == node->body.y) {
        SDL_Log("[Snake Move] Eat Retrun");
        Mix_PlayMusic(musicEat, 1);
        len++;
        state = SnakeState::EATING;
        return state;
    }
    
    // 尾前进
    SDL_Log("[Snake Move] Tail");
    List *del = tail;
    
    tail->prev->next = nullptr;
    tail = tail->prev;
    occupy[del->body.y / PER_RECT][del->body.x / PER_RECT] = false;
    delete(del);
    state = SnakeState::MOVING;
    
    return state;
}

void Snake::Draw(SDL_Renderer *renderer) const {
    SDL_Log("[Snake Draw] Start ...");

    // 蛇头
    double angle = 0;
    List *bodys = tail;
    Body body = head->body;
    SDL_Point pointHead = {PER_RECT / 2, PER_RECT / 2};
    SDL_Rect rectHead = {body.x, body.y, PER_RECT, PER_RECT};
    
    switch (direction) {
        case Direction::UP : angle = 0; break;
        case Direction::DOWN : angle = 180; break;
        case Direction::LEFT : angle = 270; break;
        case Direction::RIGHT : angle = 90; break;
    }
    SDL_Log("[Snake Draw] HEAD");
    SDL_RenderCopyEx(renderer, textureHead, NULL, &rectHead, angle, &pointHead, SDL_FLIP_NONE);

    // 蛇身
    SDL_Log("[Snake Draw] Body");
    for (int i = len; i > 1; i--) {
        Body body = bodys->body;
        SDL_Rect rectBody = {body.x, body.y, PER_RECT, PER_RECT};
        
        switch (body.dir) {
            case Direction::UP : angle = 0; break;
            case Direction::DOWN : angle = 180; break;
            case Direction::LEFT : angle = 270; break;
            case Direction::RIGHT : angle = 90; break;
        }
        if (i % 2 != 1) SDL_RenderCopyEx(renderer, textureBody1, NULL, &rectBody, angle, &pointHead, SDL_FLIP_NONE);
        else SDL_RenderCopyEx(renderer, textureBody2, NULL, &rectBody, angle, &pointHead, SDL_FLIP_NONE);

        bodys = bodys->prev;
    }

    // 死亡
    if (state == SnakeState::CRASH_WALL || state == SnakeState::CRASH_SELF) {
        SDL_Log("[Snake Draw] Dead");
        List *pervHead = head;
        SDL_Rect rectHead = {pervHead->body.x, pervHead->body.y, PER_RECT, PER_RECT};
        SDL_RenderCopy(renderer, textureError, NULL, &rectHead);    // 蛇头处画X
    }
    SDL_Log("[Snake Draw] End");
}

void Snake::UpdateDirection(Direction dir) {
    direction = dir;
}

void Snake::Reset() {
    SDL_Log("[Snake Reset] Start");
    while (head != tail) {
        occupy[tail->body.y / PER_RECT][tail->body.x / PER_RECT] = false;
        tail = tail->prev;
        delete(tail->next);
    }
    occupy[head->body.y / PER_RECT][head->body.x / PER_RECT] = false;
    delete(head);
    
    len = 1;
    direction = Direction::RIGHT;
    state = SnakeState::MOVING;
    head = new List(GAME_WIDTH / 2, GAME_HEIGHT / 2, Direction::RIGHT, nullptr, nullptr);
    tail = head;
    occupy[head->body.y / PER_RECT][head->body.x / PER_RECT] = true;
    SDL_Log("[Snake Reset] End");
}