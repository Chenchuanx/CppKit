#include "render.h"

TTF_Font *font08 = nullptr;
TTF_Font *font16 = nullptr;
TTF_Font *font24 = nullptr;
TTF_Font *font32 = nullptr;
TTF_Font *font64 = nullptr;

std::unordered_map<std::string, SDL_Texture*> imgTextureMap;
std::map<std::pair<int, std::string>, SDL_Texture*> fontTextureMap;

void ScreenPrint(SDL_Renderer *renderer, int ptSize, const std::string &text, SDL_Rect rect)
{
    TTF_Font *font = nullptr;
    SDL_Color color = {0xff, 0xff, 0xff, 0xff};
    SDL_Texture *texture = nullptr;

    switch (ptSize) {
        case 8  : font = font08; break;
        case 16 : font = font16; break;
        case 24 : font = font24; break;
        case 32 : font = font32; break;
        case 64 : font = font64; break;
        default : {
            SDL_Log("[ScreenPrint] ERROR 找不到font");
            return;
        }
    }
    if (font == NULL) {
        SDL_Log("[ScreenPrint] ERROR 打开失败");
        return;
    }

    std::pair<int, std::string> key = {ptSize, text};
    if (fontTextureMap.find(key) == fontTextureMap.end()) {
        SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        fontTextureMap[key] = texture;  // 减少 surface/texture 的申请/释放次数
        SDL_FreeSurface(surface);
    }
    else {
        texture = fontTextureMap[key];
    }
    
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    if (rect.x == -1) rect.x = (WINDOW_WIDTH - rect.w) / 2;   // 约定x=-1时，需要调整文字位置
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void ScreenImg(SDL_Renderer *renderer, std::string filePath, SDL_Rect &rect)
{
    SDL_Texture *texture = nullptr;

    if (imgTextureMap.find(filePath) == imgTextureMap.end()) {
        SDL_Surface *surface = IMG_Load(filePath.c_str());

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        imgTextureMap[filePath] = texture;  // 减少 surface/texture 的申请/释放次数
        SDL_FreeSurface(surface);
    }
    else {
        texture = imgTextureMap[filePath];
    }

    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

// 渲染界面
void RenderTitleScene(SDL_Renderer *renderer)
{
    // 初始页面
    SDL_SetRenderDrawColor(renderer, 0x1A, 0x4D, 0x2C, 0xFF);
    SDL_RenderClear(renderer);

    SDL_Rect rect_title({(WINDOW_WIDTH - TiTLE_PNG_W) / 2, 60, TiTLE_PNG_W, TiTLE_PNG_H});
    ScreenImg(renderer, TITLE_PNG_PATH, rect_title);

    ScreenPrint(renderer, 32, "贪吃蛇",  {-1, 200, 0, 0});
    ScreenPrint(renderer, 16, "按下任空格开始游戏",  {-1, 320, 0, 0});

    SDL_RenderPresent(renderer);
}

void RenderGameScene(SDL_Renderer *renderer, Food &food, Snake &snake)
{
    // 清空
    SDL_SetRenderDrawColor(renderer, 0x1A, 0x4D, 0x2C, 0xff);
    SDL_RenderClear(renderer);

    // 果实
    food.Draw(renderer);

    // 蛇
    snake.Draw(renderer);
    
    // 分数
    SDL_Rect rectText = {10, GAME_HEIGHT + 28, 0, 0};
    std::stringstream ss;

    ss << "当前分数: " << curScore << "      " << "历史记录: " << maxScore;
    ScreenPrint(renderer, 24, ss.str(), rectText);

    // 边界
    if (WallExit) SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    else SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_Rect rectGame = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderDrawRect(renderer, &rectGame);

    SDL_RenderPresent(renderer);
}

void RenderGameOverScene(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0x1A, 0x4D, 0x2C, 0xff);  // 清屏
    SDL_RenderClear(renderer);
    
    ScreenPrint(renderer, 32, "游戏结束", {-1, 120, 0, 0});

    std::stringstream ss;
    ss << "你得到了" << curScore << "分";
    if (curScore > maxScore) ss << ",新的历史记录!";
    ScreenPrint(renderer, 24, ss.str(), {-1, 180, 0, 0});
    
    ScreenPrint(renderer, 16, "按下空格重新开始", {-1, 240, 0, 0});
    
    SDL_RenderPresent(renderer);
}

void RenderPauseScene(SDL_Renderer *renderer) {
    SDL_Rect rectTitleText = {-1, 40, 0, 0};
    SDL_Rect rectSpeedText = {WINDOW_WIDTH / 2 - 200, 120, 0, 0};
    SDL_Rect rectSub = {WINDOW_WIDTH / 2 + 100, 120, BUTTON_W, BUTTON_H};
    SDL_Rect rectAdd = {WINDOW_WIDTH / 2 + 150, 120, BUTTON_W, BUTTON_H};

    SDL_Rect rectWallText = {WINDOW_WIDTH / 2 - 200, 200, 0, 0};
    SDL_Rect rectBut = {WINDOW_WIDTH / 2 + 125, 200, BUTTON_W, BUTTON_H};

    // 背景底色
    SDL_SetRenderDrawColor(renderer, 0x1A, 0x4D, 0x2C, 0xff);
    SDL_RenderClear(renderer);

    // 绘制顶部标题文字
    ScreenPrint(renderer, 32, "设置", rectTitleText);

    // 实时绘制描述文字
    char textBuf[64] = {0};
    sprintf(textBuf, "速度: %d", MAX_TIMESTEP - TimeStep + 20);
    ScreenPrint(renderer, 24, textBuf, rectSpeedText);
    if (WallExit) sprintf(textBuf, "墙: 开启");
    else sprintf(textBuf, "墙: 关闭");
    ScreenPrint(renderer, 24, textBuf, rectWallText);

    // 绘制按钮图片
    ScreenImg(renderer, SUB_PNG_PATH, rectSub);
    ScreenImg(renderer, ADD_PNG_PATH, rectAdd);
    ScreenImg(renderer, BUT_PNG_PATH, rectBut);

    // 画面刷新
    SDL_RenderPresent(renderer);
}
