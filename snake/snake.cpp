#include "snake.h"

Snake::Snake(SDL_Renderer *renderer) {
    sLen = 1;
    step = 1;
    head = new List(GAME_WIDTH / 2, GAME_HEIGHT / 2, Direction::RIGHT, nullptr, nullptr);
    tail = head;
    state = SnakeState::MOVING;
    direction = Direction::RIGHT;
    occupied =
        std::vector<std::vector<int>>(GAME_ROWS, std::vector<int>(GAME_COLS, 0));
    // (特判)假装这是起点,防止游戏启动时计算出的最短路径要往左走
    occupied[head->body.y / PER_RECT][(head->body.x - 1) / PER_RECT] = step;   
    step++;
    occupied[head->body.y / PER_RECT][head->body.x / PER_RECT] = step;

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
    
    if (AutoRun) {
        Auto(foodPoint);
    }

    const char* dirName = "";
    switch (direction) {
        case Direction::UP:    dirName = "UP";    break;
        case Direction::DOWN:  dirName = "DOWN";  break;
        case Direction::LEFT:  dirName = "LEFT"; break;
        case Direction::RIGHT: dirName = "RIGHT"; break;
        default: dirName = "UNKNOWN";
    }
    SDL_Log("[Auto] NewDir = %s", dirName);

    // 头前进
    SDL_Log("[Snake Move] Head");
    List* node = new List(head->body.x, head->body.y, direction, nullptr, nullptr);
    
    switch (direction) {
        case Direction::UP : node->body.y -= PER_RECT; break;
        case Direction::DOWN : node->body.y += PER_RECT; break;
        case Direction::LEFT : node->body.x -= PER_RECT; break;
        case Direction::RIGHT : node->body.x += PER_RECT; break;
        default: SDL_Log("[Snake Move] ERROR direction not available"); exit(1);
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
    step++;
    if (occupied[node->body.y / PER_RECT][node->body.x / PER_RECT] >= step - sLen + 1) {
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
    occupied[head->body.y / PER_RECT][head->body.x / PER_RECT] = step;

    // 吃到果（尾需要保留）
    int foodX = foodPoint.x;
    int foodY = foodPoint.y;
    if (foodX == node->body.x && foodY == node->body.y) {
        SDL_Log("[Snake Move] Eating food, score = %d", curScore);
        Mix_PlayMusic(musicEat, 1);
        sLen++;
        state = SnakeState::EATING;
        return state;
    }
    
    // 尾前进
    SDL_Log("[Snake Move] Tail");
    List *del = tail;
    
    tail->prev->next = nullptr;
    tail = tail->prev;
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
        default: SDL_Log("[Snake Draw] ERROR direction not available"); exit(1);
    }
    SDL_Log("[Snake Draw] HEAD");
    SDL_RenderCopyEx(renderer, textureHead, NULL, &rectHead, angle, &pointHead, SDL_FLIP_NONE);

    // 蛇身
    SDL_Log("[Snake Draw] Body");
    for (int i = sLen; i > 1; i--) {
        Body body = bodys->body;
        SDL_Rect rectBody = {body.x, body.y, PER_RECT, PER_RECT};
        
        switch (body.dir) {
            case Direction::UP : angle = 0; break;
            case Direction::DOWN : angle = 180; break;
            case Direction::LEFT : angle = 270; break;
            case Direction::RIGHT : angle = 90; break;
            default: SDL_Log("[Snake Draw] ERROR direction not available"); exit(1);
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

void Snake::UpdateDirection(Direction newDirection) {
    
    if (newDirection == Direction::UNKNOWN) return;

    const char* dirName = "";
    switch (newDirection) {
        case Direction::UP:    dirName = "UP";    break;
        case Direction::DOWN:  dirName = "DOWN";  break;
        case Direction::LEFT:  dirName = "LEFT"; break;
        case Direction::RIGHT: dirName = "RIGHT"; break;
        default: dirName = "UNKNOWN";
    }
    SDL_Log("[UpdateDirection] try update NewDir = %s", dirName);

    // 更新的方向不能为蛇当前方向的反方向
    // 当方向不是Direction::UNKNOWN,有且仅有相反方向的枚举值相加为3或7
    int sum = static_cast<int>(newDirection) + static_cast<int>(direction);
    if (sum == 3 || sum == 7) return;


    direction = newDirection;
}

void Snake::Reset() {
    SDL_Log("[Snake Reset] Start");
    while (head != tail) {
        tail = tail->prev;
        delete(tail->next);
    }
    delete(head);
    
    sLen = 1;
    step = 1;
    direction = Direction::RIGHT;
    state = SnakeState::MOVING;
    head = new List(GAME_WIDTH / 2, GAME_HEIGHT / 2, Direction::RIGHT, nullptr, nullptr);
    tail = head;
    for (auto& row : occupied) {
        std::fill(row.begin(), row.end(), 0);
    }
    occupied[head->body.y / PER_RECT][(head->body.x - 1) / PER_RECT] = step;
    step++;
    occupied[head->body.y / PER_RECT][head->body.x / PER_RECT] = step;
    SDL_Log("[Snake Reset] End");
}

using GridInt = std::vector<std::vector<int>>;
using GridBool = std::vector<std::vector<bool>>;
const std::vector<Direction> DIRS = {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT};

int ManhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 / PER_RECT - x2 / PER_RECT) + abs(y1 / PER_RECT - y2 / PER_RECT);
}

struct Candidate {
    int fScore;
    Point location;
    std::queue<Direction> path; // 计算出的方向

    bool operator< (const Candidate &other) const {
        return fScore > other.fScore;
    }
};

struct NeighborInfo
{
    int x;
    int y;
    Direction dir;
};

void GetValidNeighbors(int minStep, const GridInt &occupied,
                        const Point &currPoint, std::vector<NeighborInfo> &neighbors)
{
    for (int i = 0; i < 4; i++) {
        int nx = currPoint.x;
        int ny = currPoint.y;
        switch(DIRS[i]) {
            case Direction::UP    : ny -= PER_RECT; break;
            case Direction::DOWN  : ny += PER_RECT; break;
            case Direction::LEFT  : nx -= PER_RECT; break;
            case Direction::RIGHT : nx += PER_RECT; break;
            default: SDL_Log("[CalculateOpenSpace] ERROR direction not available"); exit(1);
        }
        // 边界
        if (nx >= GAME_WIDTH || nx < 0 || ny >= GAME_HEIGHT || ny < 0) {
            continue;
        }
        // 黑名单检查,
        if (occupied[ny / PER_RECT][nx / PER_RECT] >= minStep) {
            continue;
        }
        neighbors.push_back({nx, ny, DIRS[i]});
    }
}

// 找最小路径
std::queue<Direction> AstarFindPath(const Point &foodPoint, const Point &headPoint, 
                        int step, const GridInt &occupied, int sLen) 
{
    int iterations = 0;
    int nStep = step + 1;
    const int MAX_ITERATIONS = 600;
    std::priority_queue<Candidate> frontier;    // 小顶堆
    GridInt costGrid = GridInt(GAME_ROWS, std::vector<int>(GAME_COLS, INT_MAX));
    
    costGrid[headPoint.y / PER_RECT][headPoint.x / PER_RECT] = 0;
    frontier.push({0, {headPoint.x, headPoint.y}, std::queue<Direction>()});  // 起始
    while (!frontier.empty() && iterations++ < MAX_ITERATIONS) {
        auto item = frontier.top();
        int topX = item.location.x;
        int topY = item.location.y;
        std::queue<Direction> topPath = std::move(item.path);
        frontier.pop();
        
        // 到达目标
        if (topX == foodPoint.x && topY == foodPoint.y) {
            SDL_Log("[Auto] compute successful");
            if (iterations > MAX_ITERATIONS * 0.8) SDL_Log("[Auto] iterations = %d", iterations);
            return topPath;
        }

        // 探索四个方向
        std::vector<NeighborInfo> neighbors;
        if (sLen == 1) GetValidNeighbors(nStep - sLen, occupied, {topX, topY}, neighbors); // 特判: 尾部离开的位置也不走
        else GetValidNeighbors(nStep - sLen + 1, occupied, {topX, topY}, neighbors);
        for (int i = 0; i < (int)neighbors.size(); i++) {
            int nx = neighbors[i].x;
            int ny = neighbors[i].y;
            int cost = costGrid[topY / PER_RECT][topX / PER_RECT] + 1;
            if (cost < costGrid[ny / PER_RECT][nx / PER_RECT]) {
                int nFScore = cost + ManhattanDistance(nx, ny, foodPoint.x, foodPoint.y);
                std::queue<Direction> nPath = topPath;
                
                nPath.push(neighbors[i].dir);
                costGrid[ny / PER_RECT][nx / PER_RECT] = cost;
                frontier.push({nFScore, {nx, ny}, nPath});
            }
        }
    }
    return std::queue<Direction>();
}

// 得到新headPoint、occupied
void VirtualMove(std::queue<Direction> path, Point &headPoint, GridInt &occupied, int &step) 
{
    std::stringstream text;
    text << "[VirtualMove] Path: ";
    while (!path.empty()) {
        Direction nDirection = path.front();
        switch(nDirection) {
            case Direction::UP    : headPoint.y -= PER_RECT; text << "UP, "; break;
            case Direction::DOWN  : headPoint.y += PER_RECT; text << "DOWN, "; break;
            case Direction::LEFT  : headPoint.x -= PER_RECT; text << "LEFT, "; break;
            case Direction::RIGHT : headPoint.x += PER_RECT; text << "RIGHT, "; break;
            default: SDL_Log("[CalculateOpenSpace] ERROR direction not available"); exit(1);
        }
        step++;
        occupied[headPoint.y / PER_RECT][headPoint.x / PER_RECT] = step;
        path.pop();
    }
    SDL_Log("%s", text.str().c_str());
}

bool TryToFindTail(const GridInt &occupied, const Point &headPoint, int tail) 
{
    std::queue<Point> pointQue;
    GridBool visited(GAME_ROWS, std::vector<bool>(GAME_COLS, false));

    pointQue.push(headPoint);
    visited[headPoint.y / PER_RECT][headPoint.x / PER_RECT] = true;
    while (!pointQue.empty()) {
        Point currPoint = pointQue.front();
        int currX = currPoint.x;
        int currY = currPoint.y;
        pointQue.pop();

        std::vector<NeighborInfo> neighbors;    
        GetValidNeighbors(tail + 1, occupied, {currX, currY}, neighbors);
        for (int i = 0; i < (int)neighbors.size(); i++) {
            int nx = neighbors[i].x;
            int ny = neighbors[i].y;
            int gx = nx / PER_RECT;
            int gy = ny / PER_RECT;
            // 找到尾巴
            if (occupied[gy][gx] == tail) {
                return true;
            }
            if (visited[gy][gx]) continue;
            visited[gy][gx] = true;
            pointQue.push({nx, ny});
        }
    }
    SDL_Log("[TryToFindTail] cant find ?\n");
    for (int i = 0; i < GAME_ROWS; i++) {
        std::stringstream ss;
        for (int j = 0; j < GAME_COLS; j++) {
            if (i == headPoint.y / PER_RECT && j == headPoint.x / PER_RECT) {
                ss << "a" << ", ";
            } else ss << visited[i][j] << ", ";
        }
        SDL_Log("%s", ss.str().c_str());
    }
    return false;
}

// 由尾找头
bool TryToFindHead(const GridInt &occupied, const Point &headPoint, int tail) 
{
    std::queue<Point> pointQue;
    GridBool visited(GAME_ROWS, std::vector<bool>(GAME_COLS, false));

    pointQue.push(headPoint);
    visited[headPoint.y / PER_RECT][headPoint.x / PER_RECT] = true;
    while (!pointQue.empty()) {
        Point currPoint = pointQue.front();
        int currX = currPoint.x;
        int currY = currPoint.y;
        pointQue.pop();

        std::vector<NeighborInfo> neighbors;    
        GetValidNeighbors(tail + 1, occupied, {currX, currY}, neighbors);
        for (int i = 0; i < (int)neighbors.size(); i++) {
            int nx = neighbors[i].x;
            int ny = neighbors[i].y;
            // 找到尾巴
            if (occupied[ny / PER_RECT][nx / PER_RECT] == tail) {
                return true;
            }
            visited[ny / PER_RECT][nx / PER_RECT] = true;
            pointQue.push({nx, ny});
        }
    }

    return false;
}

void BuildTailGrid(const GridInt &occupied, const Point &tailPoint, GridInt &costGrid, int tailStep)
{
    int cost = 0;
    int qSize = 0;
    std::queue<Point> pointQue;
    
    pointQue.push({tailPoint.x, tailPoint.y});
    costGrid[tailPoint.y / PER_RECT][tailPoint.x / PER_RECT] = cost;
    while (!pointQue.empty()) {
        if (qSize == 0) {
            cost++;
            qSize = pointQue.size();
        }
        Point currPoint = pointQue.front();
        int currX = currPoint.x;
        int currY = currPoint.y;
        pointQue.pop();
        qSize--;

        std::vector<NeighborInfo> neighbornts;    
        GetValidNeighbors(tailStep, occupied, {currX, currY}, neighbornts);
        for (int i = 0; i < (int)neighbornts.size(); i++) {
            int nx = neighbornts[i].x;
            int ny = neighbornts[i].y;
            
            if (costGrid[ny / PER_RECT][nx / PER_RECT] != INT_MAX) continue;    // 不是第一次来
            pointQue.push({nx, ny});
            costGrid[ny / PER_RECT][nx / PER_RECT] = cost;
        }
    }

    SDL_Log("[BuildTailGrid] cant find ?\n");
    for (int i = 0; i < GAME_ROWS; i++) {
        std::stringstream ss;
        for (int j = 0; j < GAME_COLS; j++) {
            if (i == tailPoint.x / PER_RECT && j == tailPoint.y / PER_RECT) {
                ss << "a" << ", ";
            } else ss << costGrid[i][j] << ", ";
        }
        SDL_Log("%s", ss.str().c_str());
    }
}

// step - sLen + 1      当前蛇尾位置
// (step+1) - sLen + 1  走一步后,蛇尾位置
void Snake::Auto(const Point &foodPoint) 
{
    static std::queue<Direction> foundFood;
    Direction nextDirection = Direction::UNKNOWN;

    // 提前返回
    if (!foundFood.empty()) {
        nextDirection = foundFood.front();
        UpdateDirection(nextDirection);
        foundFood.pop();
        return;
    }

    Point headPoint = {head->body.x, head->body.y};
    std::queue<Direction> nextPath = AstarFindPath(foodPoint, headPoint, step, occupied, sLen);

    // 能吃到食物
    if (!nextPath.empty()) {
        SDL_Log("[Auto] can eat food");

        // 模拟
        SDL_Log("[Auto] virtual move");
        int virtualStep = step;
        Point virtualHeadPoint = headPoint;
        GridInt virtualOccupied = occupied;
        VirtualMove(nextPath, virtualHeadPoint, virtualOccupied, virtualStep);

        // 找尾巴
        SDL_Log("[Auto] try to find tail");
        int virtualTailStep = virtualStep - (sLen + 1) + 1; // 模拟尾巴所在位置的step值
        bool virtualFindTail = TryToFindTail(virtualOccupied, virtualHeadPoint, virtualTailStep); 
        
        // 能找到尾巴
        if (virtualFindTail) { 
            SDL_Log("[Auto] can find tail");
            foundFood = move(nextPath);
            nextDirection = foundFood.front();
            foundFood.pop();
            UpdateDirection(nextDirection);
            return;
        }
        SDL_Log("[Auto] can eat food, can't find tail");
    }
    else {
        SDL_Log("[Auto] can't eat food");
    }

    SDL_Log("[Auto] find farthest path to tail");
    // 各点到蛇尾距离网格
    int tailStep = step - sLen + 1;
    Point tailPoint = {tail->body.x, tail->body.y};
    GridInt costGrid = GridInt(GAME_ROWS, std::vector<int>(GAME_COLS, INT_MAX));
    BuildTailGrid(occupied, tailPoint, costGrid, tailStep);

    // 判断是否联通 + 选择最远方向
    int maxCost = -1;   // cost最小为零,即头尾相邻时
    std::vector<NeighborInfo> neighbors;    
    GetValidNeighbors((step + 1) - sLen + 1, occupied, headPoint, neighbors);
    for (int i = 0; i < (int)neighbors.size(); i++) {
        NeighborInfo neighbor = neighbors[i];
        int curCost = costGrid[neighbor.y / PER_RECT][neighbor.x / PER_RECT];
        if (curCost != INT_MAX && maxCost < curCost) {
            maxCost = curCost;
            nextDirection = neighbor.dir;
        }
    }

    // 能到尾巴，向蛇尾走一步
    if (maxCost != -1) {
        UpdateDirection(nextDirection);
        return;
    }

    // 都不行,有问题
    SDL_Log("[Auto] ERROR can't find any way");
}