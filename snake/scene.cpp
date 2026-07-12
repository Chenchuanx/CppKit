#include "scene.h"

void PauseScene(SDL_Renderer *renderer, GameState &gameState)
{
    SDL_Event event;

    SDL_Rect rectSub = {WINDOW_WIDTH / 2 + 100, 120, BUTTON_W, BUTTON_H};
    SDL_Rect rectAdd = {WINDOW_WIDTH / 2 + 150, 120, BUTTON_W, BUTTON_H};
    SDL_Rect rectBut = {WINDOW_WIDTH / 2 + 125, 200, BUTTON_W, BUTTON_H};
    
    bool sceneRunning = true;

    while (sceneRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState = GameState::Quit;
                return;
            }

            // ESC 退出暂停
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                sceneRunning = false;
                gameState = GameState::Gaming;
                break;
            }
            // 鼠标点击交互
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                SDL_Point mouse = {event.button.x, event.button.y};
                // 减速
                if (SDL_PointInRect(&mouse, &rectSub))
                {
                    TimeStep += STEP_OFFSET;
                    if (TimeStep > MAX_TIMESTEP) TimeStep = MAX_TIMESTEP;
                }
                // 加速
                if (SDL_PointInRect(&mouse, &rectAdd))
                {
                    TimeStep -= STEP_OFFSET;
                    if (TimeStep < MIN_TIMESTEP) TimeStep = MIN_TIMESTEP;
                }
                // 开关墙
                if (SDL_PointInRect(&mouse, &rectBut))
                {
                    WallExit = !WallExit;
                }
            }
        }

        RenderPauseScene(renderer);

        SDL_Delay(5);
    }
}

void TitleScene(SDL_Renderer *renderer, GameState &gameState) {
    SDL_Event event;

    RenderTitleScene(renderer);
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gameState = GameState::Quit;
            return;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
            gameState = GameState::Gaming;
            return;
        }

        SDL_Delay(5);
    }
}

void GameOverScene(SDL_Renderer *renderer, GameState &gameState) {
    SDL_Log("[Main] Restart");
    SDL_Event event;
    
    RenderGameOverScene(renderer);
    
    if (curScore > maxScore) {
        std::ofstream fout(MAX_SCORE_FILE);
        maxScore = curScore;
        fout << maxScore;
        fout.close();
    }
    curScore = 0;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gameState = GameState::Quit;
            return;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
            gameState = GameState::Gaming;
            return;
        }

        SDL_Delay(5);
    }
}

void GameScene(SDL_Renderer *renderer, GameState &gameState) {
    SDL_Log("游戏开始");
    static Snake snake(renderer);
    static Food food(1, snake, renderer);
    static Direction bufDir = Direction::RIGHT;
    RenderGameScene(renderer, food, snake);
    
    SDL_Event event;
    bool game_running = true;
    Uint32 lastOpTick = 0;
    const Uint32 OP_COOLDOWN = 50; // 操作冷却
    
    while (game_running) {
        // 控制蛇
        while (SDL_PollEvent(&event)) {
            Uint32 type = event.type;
            if (type == SDL_QUIT) {
                gameState = GameState::Quit;
                return;
            }
            if (type != SDL_KEYDOWN) continue;
            switch(event.key.keysym.sym) {
                case SDLK_w: {
                    if (snake.GetDirection() != Direction::DOWN) 
                        bufDir = Direction::UP;
                } break;
                case SDLK_a: {
                    if (snake.GetDirection() != Direction::RIGHT) 
                        bufDir = Direction::LEFT;
                } break;
                case SDLK_s: {
                    if (snake.GetDirection() != Direction::UP) 
                        bufDir = Direction::DOWN;
                } break;
                case SDLK_d: {
                    if (snake.GetDirection() != Direction::LEFT) 
                        bufDir = Direction::RIGHT;
                } break;
                case SDLK_ESCAPE: {
                    gameState = GameState::Pause;
                } return;
                default: break;
            }
        }
        
        // 除去与前一个操作的时间间隔少的操作
		if (SDL_GetTicks() - lastOpTick < OP_COOLDOWN) {
            continue;
        }
        // 蛇操作
        snake.UpdateDirection(bufDir);
        SnakeState state = snake.Move(food.GetPoint(), food.GetScore());
        RenderGameScene(renderer, food, snake);

        // 撞墙/吃自己
        if (state == SnakeState::CRASH_WALL || state == SnakeState::CRASH_SELF) {
            SDL_Delay(500);
            snake.Reset();
            food.Reset(snake);
            bufDir = Direction::RIGHT;
            gameState = GameState::GameOver;
            return;
        }

        // 吃果
        if (state == SnakeState::EATING) {
            curScore += food.GetScore();
            food.Reset(snake);
        }
        
        SDL_Delay(TimeStep);
    }
}
