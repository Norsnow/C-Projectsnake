// C++Projectsnake.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "C++Projectsnake.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <windef.h>
#include <cmath>

#pragma comment(lib, "Msimg32.lib") // Link with Msimg32 for GradientFill usage

#define MAX_LOADSTRING 100

// 游戏常量
const int GRID_WIDTH = 25;
const int GRID_HEIGHT = 20;
const int CELL_SIZE = 25;
const int GAME_WIDTH = GRID_WIDTH * CELL_SIZE;
const int GAME_HEIGHT = GRID_HEIGHT * CELL_SIZE;

// 关卡定义
const int MAX_LEVEL = 3;
const int LEVEL_1_SPEED = 150;
const int LEVEL_2_SPEED = 100;
const int LEVEL_3_SPEED = 120;

// 关卡完成条件（需要吃到的苹果数量）
const int LEVEL_1_APPLES = 5;
const int LEVEL_2_APPLES = 7;
const int LEVEL_3_APPLES = 10;

// 全局变量:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 游戏函数前向声明
void GenerateFood();
void SetLevelSpeed();
void ResetGame();
void MoveFood();
void UpdateGame();
void DrawGame(HDC hdc, HWND hWnd);
void DrawButton(HDC hdc, const RECT& rect, const wchar_t* text, bool isHovered, COLORREF textColor = 0xFFFFFF);
void DrawRoundRect(HDC hdc, int x, int y, int w, int h, COLORREF color);

// 定义方向
enum Direction { UP, DOWN, LEFT, RIGHT };

// 游戏状态
enum GameState { MENU, PLAYING, PAUSED, GAME_OVER };

// 全局变量
std::vector<POINT> snake;
POINT food;
Direction dir = RIGHT;
Direction nextDir = RIGHT;
GameState gameState = GAME_OVER;
int score = 0;
int level = 1;
int foodEaten = 0;
int gameSpeed = LEVEL_1_SPEED;
int currentLevelApples = 0;
int foodMoveTimer = 0;
const int FOOD_MOVE_INTERVAL = 5;

// 动画与UI交互状态
int animationTick = 0; // 全局动画计时器，用于食物脉冲等效果
bool startButtonHover = false; // 菜单开始按钮悬停状态
int foodEatEffect = 0; // 吃到食物的特效计数器
POINT foodEatPosition; // 吃到食物的位置

// 按钮绘制/交互矩形（用于保证绘制与点击一致）
RECT g_startButtonRect = { GAME_WIDTH / 2 - 120, GAME_HEIGHT / 2, GAME_WIDTH / 2 + 120, GAME_HEIGHT / 2 + 60 };
RECT g_drawButtonRect = g_startButtonRect;

// 胜利标志
bool isVictory = false;

// 颜色定义 - 使用十六进制RGB值
#define COLOR_BACKGROUND 0x1E1E1E
#define COLOR_GRID       0x3C3C3C
#define COLOR_SNAKE_HEAD 0x4CAF50
#define COLOR_SNAKE_BODY 0x388E3C
#define COLOR_FOOD       0xE91E63  // 粉色食物
#define COLOR_TEXT       0xFFFFFF
#define COLOR_ACCENT     0x9C27B0  // 强调色
#define COLOR_SHADOW     0x121212  // 阴影色
#define COLOR_HIGHLIGHT  0xFFFFFF  // 高光色

// 重置游戏
void ResetGame()
{
    snake.clear();
    snake.push_back({ GRID_WIDTH / 2, GRID_HEIGHT / 2 });
    snake.push_back({ GRID_WIDTH / 2 - 1, GRID_HEIGHT / 2 });
    snake.push_back({ GRID_WIDTH / 2 - 2, GRID_HEIGHT / 2 });

    dir = RIGHT;
    nextDir = RIGHT;
    score = 0;
    foodEaten = 0;
    level = 1;
    currentLevelApples = 0;
    foodMoveTimer = 0;
    foodEatEffect = 0;
    isVictory = false; // reset victory flag
    g_drawButtonRect = g_startButtonRect; // reset drawn button rect
    
    // 根据关卡设置游戏速度
    SetLevelSpeed();
    GenerateFood();
}

// 初始化游戏
void InitGame()
{
    ResetGame();
    gameState = MENU;
}

// 根据当前关卡设置游戏速度
void SetLevelSpeed()
{
    switch (level)
    {
    case 1:
        gameSpeed = LEVEL_1_SPEED;
        break;
    case 2:
        gameSpeed = LEVEL_2_SPEED;
        break;
    case 3:
        gameSpeed = LEVEL_3_SPEED;
        break;
    }
}

// 生成食物
void GenerateFood()
{
    bool validPosition = false;
    while (!validPosition)
    {
        food = { rand() % GRID_WIDTH, rand() % GRID_HEIGHT };
        validPosition = true;
        for (const auto& segment : snake)
        {
            if (food.x == segment.x && food.y == segment.y)
            {
                validPosition = false;
                break;
            }
        }
    }
}

// 移动食物（第3关）
void MoveFood()
{
    if (level != 3)
        return;

    foodMoveTimer++;
    if (foodMoveTimer < FOOD_MOVE_INTERVAL)
        return;

    foodMoveTimer = 0;

    // 随机选择一个方向移动食物
    int moveDir = rand() % 4;
    POINT newFood = food;

    switch (moveDir)
    {
    case 0: newFood.y--; break; // 上
    case 1: newFood.y++; break; // 下
    case 2: newFood.x--; break; // 左
    case 3: newFood.x++; break; // 右
    }

    // 检查食物是否在有效范围内
    if (newFood.x >= 0 && newFood.x < GRID_WIDTH && newFood.y >= 0 && newFood.y < GRID_HEIGHT)
    {
        // 检查食物是否和蛇重叠
        bool collision = false;
        for (const auto& segment : snake)
        {
            if (newFood.x == segment.x && newFood.y == segment.y)
            {
                collision = true;
                break;
            }
        }

        if (!collision)
        {
            food = newFood;
        }
    }
}

// 更新游戏状态
void UpdateGame()
{
    if (gameState != PLAYING)
        return;

    dir = nextDir;
    
    // 更新动画计时器
    animationTick++;

    // 计算蛇头的新位置
    POINT newHead = snake.front();
    switch (dir)
    {
    case UP:    newHead.y--; break;
    case DOWN:  newHead.y++; break;
    case LEFT:  newHead.x--; break;
    case RIGHT: newHead.x++; break;
    }

    // 检测碰撞 - 墙壁
    if (newHead.x < 0 || newHead.x >= GRID_WIDTH || newHead.y < 0 || newHead.y >= GRID_HEIGHT)
    {
        gameState = GAME_OVER;
        return;
    }

    // 检测碰撞 - 自身
    for (const auto& segment : snake)
    {
        if (newHead.x == segment.x && newHead.y == segment.y)
        {
            gameState = GAME_OVER;
            return;
        }
    }

    // 添加新头部
    snake.insert(snake.begin(), newHead);

    // 检测是否吃到食物
        if (newHead.x == food.x && newHead.y == food.y)
        {
            foodEaten++;
            currentLevelApples++;
            score += level * 10;
            
            // 触发吃到食物的特效
            foodEatEffect = 10;
            foodEatPosition = food;
            
            // 检查是否完成当前关卡
            bool levelCompleted = false;
            int requiredApples = 0;
            
            switch (level)
            {
            case 1:
                requiredApples = LEVEL_1_APPLES;
                break;
            case 2:
                requiredApples = LEVEL_2_APPLES;
                break;
            case 3:
                requiredApples = LEVEL_3_APPLES;
                break;
            }
            
            if (currentLevelApples >= requiredApples)
            {
                // 检查是否完成所有关卡
                if (level == MAX_LEVEL)
                {
                    // 游戏胜利
                    isVictory = true;
                    gameState = GAME_OVER;
                    return;
                }
                else
                {
                    // 进入下一关卡
                    level++;
                    currentLevelApples = 0;
                    SetLevelSpeed();
                    
                    // 重置蛇的位置
                    snake.clear();
                    snake.push_back({ GRID_WIDTH / 2, GRID_HEIGHT / 2 });
                    snake.push_back({ GRID_WIDTH / 2 - 1, GRID_HEIGHT / 2 });
                    snake.push_back({ GRID_WIDTH / 2 - 2, GRID_HEIGHT / 2 });
                    dir = RIGHT;
                    nextDir = RIGHT;
                }
            }
            
            GenerateFood();
        }
    else
    {
        // 移除尾部
        snake.pop_back();
    }
    
    // 第3关：移动食物
    MoveFood();
}

// 绘制圆角矩形
void DrawRoundRect(HDC hdc, int x, int y, int w, int h, COLORREF color)
{
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    // 绘制带圆角的矩形
    RoundRect(hdc, x, y, x + w, y + h, 8, 8);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

// 绘制渐变填充的圆角矩形
void DrawGradientRoundRect(HDC hdc, int x, int y, int w, int h, COLORREF startColor, COLORREF endColor)
{
    // 创建渐变画刷
    TRIVERTEX vertex[2];
    vertex[0].x = x;
    vertex[0].y = y;
    vertex[0].Red = GetRValue(startColor) << 8;
    vertex[0].Green = GetGValue(startColor) << 8;
    vertex[0].Blue = GetBValue(startColor) << 8;
    vertex[0].Alpha = 0x0000;
    
    vertex[1].x = x + w;
    vertex[1].y = y + h;
    vertex[1].Red = GetRValue(endColor) << 8;
    vertex[1].Green = GetGValue(endColor) << 8;
    vertex[1].Blue = GetBValue(endColor) << 8;
    vertex[1].Alpha = 0x0000;
    
    GRADIENT_RECT gRect;
    gRect.UpperLeft = 0;
    gRect.LowerRight = 1;
    
    // 绘制渐变背景
    GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
    
    // 绘制边框
    HPEN hPen = CreatePen(PS_SOLID, 2, startColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    RoundRect(hdc, x, y, x + w, y + h, 8, 8);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

// 绘制吃到食物的特效
void DrawFoodEatEffect(HDC hdc)
{
    if (foodEatEffect <= 0)
        return;
    
    int x = foodEatPosition.x * CELL_SIZE + CELL_SIZE / 2;
    int y = foodEatPosition.y * CELL_SIZE + CELL_SIZE / 2;
    int radius = (10 - foodEatEffect) * 5;
    
    // 创建半透明画笔
    HPEN hPen = CreatePen(PS_SOLID, 2, COLOR_HIGHLIGHT);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    // 绘制扩散的圆环
    Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // 减少特效计数器
    foodEatEffect--;
}

// 绘制按钮
void DrawButton(HDC hdc, const RECT& rect, const wchar_t* text, bool isHovered, COLORREF textColor)
{
    COLORREF buttonColor = isHovered ? 0x66BB6A : 0x4CAF50;
    COLORREF buttonColor2 = isHovered ? 0x4CAF50 : 0x388E3C;

    // 创建阴影效果
    HBRUSH hShadowBrush = CreateSolidBrush(COLOR_SHADOW);
    RECT shadowRect = { rect.left + 3, rect.top + 3, rect.right + 3, rect.bottom + 3 };
    RoundRect(hdc, shadowRect.left, shadowRect.top, shadowRect.right, shadowRect.bottom, 10, 10);
    FillRect(hdc, &shadowRect, hShadowBrush);
    DeleteObject(hShadowBrush);
    
    // 绘制按钮背景为单色以简化开始界面视觉
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 10, 10);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    // 绘制按钮文本（悬停时字体稍大并加粗）
    HFONT hFont = CreateFontW(isHovered ? 20 : 18, 0, 0, 0, isHovered ? FW_EXTRABOLD : FW_BOLD, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);
    
    RECT textRect = rect;
    DrawTextW(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// 绘制游戏
void DrawGame(HDC hdc, HWND hWnd)
{
    // 获取客户区大小
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    // 双缓冲：创建内存 DC 和位图，避免闪烁
    HDC origHdc = hdc;
    HDC memDC = CreateCompatibleDC(origHdc);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    HBITMAP hbm = CreateCompatibleBitmap(origHdc, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hbm);

    // 将绘图目标切换到内存 DC
    hdc = memDC;

    // 绘制渐变背景
    TRIVERTEX vertex[2];
    vertex[0].x = 0;
    vertex[0].y = 0;
    vertex[0].Red = GetRValue(COLOR_BACKGROUND) << 8;
    vertex[0].Green = GetGValue(COLOR_BACKGROUND) << 8;
    vertex[0].Blue = GetBValue(COLOR_BACKGROUND) << 8;
    vertex[0].Alpha = 0x0000;
    
    vertex[1].x = clientRect.right;
    vertex[1].y = clientRect.bottom;
    // Use integer arithmetic to avoid floating point in compile-time expressions
    vertex[1].Red = ((GetRValue(COLOR_BACKGROUND) * 8) / 10) << 8; // *0.8
    vertex[1].Green = ((GetGValue(COLOR_BACKGROUND) * 8) / 10) << 8; // *0.8
    vertex[1].Blue = ((GetBValue(COLOR_BACKGROUND) * 6) / 5) << 8; // *1.2
    vertex[1].Alpha = 0x0000;
    
    GRADIENT_RECT gRect;
    gRect.UpperLeft = 0;
    gRect.LowerRight = 1;
    GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

    // 根据游戏状态绘制不同界面
    if (gameState == MENU)
    {
        // 绘制装饰性顶部条
        // DrawGradientRoundRect(hdc, 0, 0, GAME_WIDTH, 70, 0x2C3E50, 0x34495E);

        // 绘制游戏标题
        HFONT hTitleFont = CreateFontW(56, 0, 0, 0, FW_EXTRABOLD, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);

        SetTextColor(hdc, 0x4CAF50);
        SetBkMode(hdc, TRANSPARENT);

        RECT titleRect = { 0, GAME_HEIGHT / 2 - 120, GAME_WIDTH, GAME_HEIGHT / 2 - 40 };
        
        // 添加标题阴影
        RECT titleShadowRect = titleRect;
        titleShadowRect.left += 2;
        titleShadowRect.top += 2;
        SetTextColor(hdc, COLOR_SHADOW);
        DrawTextW(hdc, L"🐍  贪吃蛇游戏  🐍", -1, &titleShadowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 绘制主标题
        SetTextColor(hdc, 0x4CAF50);
        DrawTextW(hdc, L"🐍  贪吃蛇游戏  🐍", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
        DeleteObject(hTitleFont);

        // 绘制开始按钮
        RECT startButtonRect = { GAME_WIDTH / 2 - 120, GAME_HEIGHT / 2, GAME_WIDTH / 2 + 120, GAME_HEIGHT / 2 + 60 };
        
        // 根据悬停状态调整按钮绘制
        RECT drawButtonRect = startButtonRect;
        if (startButtonHover)
        {
            // 悬停时放大
            drawButtonRect.left -= 6;
            drawButtonRect.top -= 6;
            drawButtonRect.right += 6;
            drawButtonRect.bottom += 6;
        }

        // 按钮主体（传入悬停状态）
        // 文本为黑色以满足要求
        DrawButton(hdc, drawButtonRect, L"▶  开始游戏", startButtonHover, 0x000000);

        // 保存实际绘制的按钮矩形用于点击检测
        g_drawButtonRect = drawButtonRect;

        // 绘制关卡说明 - 增强版本
        HFONT hInfoFont = CreateFontW(18, 0, 0, 0, FW_BOLD, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        SelectObject(hdc, hInfoFont);

        SetTextColor(hdc, COLOR_TEXT);

        RECT infoRect;
        wchar_t infoText[200];

        // 第1关
        infoRect = { 50, GAME_HEIGHT / 2 + 100, GAME_WIDTH - 50, GAME_HEIGHT / 2 + 125 };
        swprintf_s(infoText, 200, L"⭐  第1关: 静态苹果，需要吃到 %d 个", LEVEL_1_APPLES);
        SetTextColor(hdc, 0xFFC107);
        DrawTextW(hdc, infoText, -1, &infoRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 第2关
        infoRect.top += 40;
        infoRect.bottom += 40;
        swprintf_s(infoText, 200, L"⭐⭐  第2关: 速度加快，需要吃到 %d 个", LEVEL_2_APPLES);
        SetTextColor(hdc, 0xFF9800);
        DrawTextW(hdc, infoText, -1, &infoRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 第3关
        infoRect.top += 40;
        infoRect.bottom += 40;
        swprintf_s(infoText, 200, L"⭐⭐⭐  第3关: 苹果移动，需要吃到 %d 个", LEVEL_3_APPLES);
        SetTextColor(hdc, 0xF44336);
        DrawTextW(hdc, infoText, -1, &infoRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
        DeleteObject(hInfoFont);

        // 绘制底部提示
        HFONT hTipsFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        SelectObject(hdc, hTipsFont);
        SetTextColor(hdc, 0x9E9E9E);
        
        RECT tipsRect = { 0, GAME_HEIGHT - 50, GAME_WIDTH, GAME_HEIGHT };
        DrawTextW(hdc, L"💡 按 [Space] 或点击按钮开始游戏 | 按 [←→↑↓] 控制蛇的方向", -1, &tipsRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, hOldFont);
        DeleteObject(hTipsFont);
        
        // 绘制装饰性网格背景
        HPEN hGridPen = CreatePen(PS_SOLID, 1, 0x2C2C2C);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
        for (int i = 0; i < GAME_WIDTH; i += 20)
        {
            MoveToEx(hdc, i, 0, NULL);
            LineTo(hdc, i, GAME_HEIGHT);
        }
        for (int i = 0; i < GAME_HEIGHT; i += 20)
        {
            MoveToEx(hdc, 0, i, NULL);
            LineTo(hdc, GAME_WIDTH, i);
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(hGridPen);
    }
    else
    {
        // 绘制游戏区域装饰边框
        HPEN hBorderPen = CreatePen(PS_SOLID, 3, 0x4CAF50);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
        Rectangle(hdc, 0, 0, GAME_WIDTH, GAME_HEIGHT);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBorderPen);

        // 绘制网格背景
        HPEN hGridPen = CreatePen(PS_SOLID, 1, COLOR_GRID);
        hOldPen = (HPEN)SelectObject(hdc, hGridPen);

        for (int i = 0; i <= GRID_WIDTH; i++)
        {
            MoveToEx(hdc, i * CELL_SIZE, 0, NULL);
            LineTo(hdc, i * CELL_SIZE, GAME_HEIGHT);
        }
        for (int i = 0; i <= GRID_HEIGHT; i++)
        {
            MoveToEx(hdc, 0, i * CELL_SIZE, NULL);
            LineTo(hdc, GAME_WIDTH, i * CELL_SIZE);
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hGridPen);

        // 绘制蛇 - 更精美的视觉效果
        for (size_t i = 0; i < snake.size(); i++)
        {
            COLORREF color = (i == 0) ? COLOR_SNAKE_HEAD : COLOR_SNAKE_BODY;
            int x = snake[i].x * CELL_SIZE + 2;
            int y = snake[i].y * CELL_SIZE + 2;
            int w = CELL_SIZE - 4;
            int h = CELL_SIZE - 4;

            // 蛇身阴影
            HBRUSH hShadowBrush = CreateSolidBrush(COLOR_SHADOW);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hShadowBrush);
            Rectangle(hdc, x + 2, y + 2, x + w + 2, y + h + 2);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hShadowBrush);

            // 蛇身主体 - 使用渐变效果
            COLORREF bodyColor2 = (i == 0) ? 0x66BB6A : 0x4CAF50;
            DrawGradientRoundRect(hdc, x, y, w, h, color, bodyColor2);

            // 蛇头细节
            if (i == 0)
            {
                HBRUSH hEyeBrush = CreateSolidBrush(0xFFFFFF);
                hOldBrush = (HBRUSH)SelectObject(hdc, hEyeBrush);
                
                int eyeSize = 4;
                int pupilSize = 2;
                
                // 眼睛高光
                switch (dir)
                {
                case UP:
                    // 左眼
                    Ellipse(hdc, x + 4, y + 3, x + 8, y + 7);
                    // 右眼
                    Ellipse(hdc, x + w - 8, y + 3, x + w - 4, y + 7);
                    // 瞳孔
                    {
                        HBRUSH hPupilBrush = CreateSolidBrush(0x000000);
                        HBRUSH hOldPupil = (HBRUSH)SelectObject(hdc, hPupilBrush);
                        Ellipse(hdc, x + 5, y + 4, x + 7, y + 6);
                        Ellipse(hdc, x + w - 7, y + 4, x + w - 5, y + 6);
                        SelectObject(hdc, hOldPupil);
                        DeleteObject(hPupilBrush);
                    }
                    break;
                case DOWN:
                    // 左眼
                    Ellipse(hdc, x + 4, y + h - 7, x + 8, y + h - 3);
                    // 右眼
                    Ellipse(hdc, x + w - 8, y + h - 7, x + w - 4, y + h - 3);
                    // 瞳孔
                    {
                        HBRUSH hPupilBrush = CreateSolidBrush(0x000000);
                        HBRUSH hOldPupil = (HBRUSH)SelectObject(hdc, hPupilBrush);
                        Ellipse(hdc, x + 5, y + h - 6, x + 7, y + h - 4);
                        Ellipse(hdc, x + w - 7, y + h - 6, x + w - 5, y + h - 4);
                        SelectObject(hdc, hOldPupil);
                        DeleteObject(hPupilBrush);
                    }
                    break;
                case LEFT:
                    // 左眼
                    Ellipse(hdc, x + 3, y + 4, x + 7, y + 8);
                    // 右眼
                    Ellipse(hdc, x + 3, y + h - 8, x + 7, y + h - 4);
                    // 瞳孔
                    {
                        HBRUSH hPupilBrush = CreateSolidBrush(0x000000);
                        HBRUSH hOldPupil = (HBRUSH)SelectObject(hdc, hPupilBrush);
                        Ellipse(hdc, x + 4, y + 5, x + 6, y + 7);
                        Ellipse(hdc, x + 4, y + h - 7, x + 6, y + h - 5);
                        SelectObject(hdc, hOldPupil);
                        DeleteObject(hPupilBrush);
                    }
                    break;
                case RIGHT:
                    // 左眼
                    Ellipse(hdc, x + w - 7, y + 4, x + w - 3, y + 8);
                    // 右眼
                    Ellipse(hdc, x + w - 7, y + h - 8, x + w - 3, y + h - 4);
                    // 瞳孔
                    {
                        HBRUSH hPupilBrush = CreateSolidBrush(0x000000);
                        HBRUSH hOldPupil = (HBRUSH)SelectObject(hdc, hPupilBrush);
                        Ellipse(hdc, x + w - 6, y + 5, x + w - 4, y + 7);
                        Ellipse(hdc, x + w - 6, y + h - 7, x + w - 4, y + h - 5);
                        SelectObject(hdc, hOldPupil);
                        DeleteObject(hPupilBrush);
                    }
                    break;
                }
                
                SelectObject(hdc, hOldBrush);
                DeleteObject(hEyeBrush);
                
                // 绘制蛇信子
                HPEN hPen = CreatePen(PS_SOLID, 2, 0xF44336);
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                
                switch (dir)
                {
                case UP:
                    MoveToEx(hdc, x + w / 2, y + h, NULL);
                    LineTo(hdc, x + w / 2 - 3, y + h + 8);
                    MoveToEx(hdc, x + w / 2, y + h, NULL);
                    LineTo(hdc, x + w / 2 + 3, y + h + 8);
                    break;
                case DOWN:
                    MoveToEx(hdc, x + w / 2, y, NULL);
                    LineTo(hdc, x + w / 2 - 3, y - 8);
                    MoveToEx(hdc, x + w / 2, y, NULL);
                    LineTo(hdc, x + w / 2 + 3, y - 8);
                    break;
                case LEFT:
                    MoveToEx(hdc, x + w, y + h / 2, NULL);
                    LineTo(hdc, x + w + 8, y + h / 2 - 3);
                    MoveToEx(hdc, x + w, y + h / 2, NULL);
                    LineTo(hdc, x + w + 8, y + h / 2 + 3);
                    break;
                case RIGHT:
                    MoveToEx(hdc, x, y + h / 2, NULL);
                    LineTo(hdc, x - 8, y + h / 2 - 3);
                    MoveToEx(hdc, x, y + h / 2, NULL);
                    LineTo(hdc, x - 8, y + h / 2 + 3);
                    break;
                }
                
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);
            }
        }

        // 绘制食物 - 动态发光效果
        {
            int x = food.x * CELL_SIZE + 2;
            int y = food.y * CELL_SIZE + 2;
            int w = CELL_SIZE - 4;
            int h = CELL_SIZE - 4;
            
            // 脉动光晕效果
            int pulseSize = 2 + (animationTick % 10) / 2;
            
            // 外层发光效果
            for (int i = 0; i < 3; i++)
            {
                HPEN hGlowPen = CreatePen(PS_SOLID, 1, 0xF06292);
                HPEN hOldPen = (HPEN)SelectObject(hdc, hGlowPen);
                Ellipse(hdc, x - pulseSize - i, y - pulseSize - i, x + w + pulseSize + i, y + h + pulseSize + i);
                SelectObject(hdc, hOldPen);
                DeleteObject(hGlowPen);
            }

            // 食物主体 - 渐变效果
            COLORREF foodColor2 = 0xF06292;
            DrawGradientRoundRect(hdc, x, y, w, h, COLOR_FOOD, foodColor2);

            // 食物高光效果
            HBRUSH hHighlight = CreateSolidBrush(0xFFB74D);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hHighlight);
            Ellipse(hdc, x + 4, y + 4, x + w - 6, y + h - 6);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hHighlight);

            // 食物反光点
            HBRUSH hShineBrush = CreateSolidBrush(0xFFFFFF);
            hOldBrush = (HBRUSH)SelectObject(hdc, hShineBrush);
            Ellipse(hdc, x + 6, y + 6, x + 9, y + 9);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hShineBrush);
        }
        
        // 绘制吃到食物的特效
        DrawFoodEatEffect(hdc);

        // 绘制侧栏背景
        int sidebarX = GAME_WIDTH + 20;
        RECT sidebarBg = { GAME_WIDTH + 2, 0, clientRect.right, clientRect.bottom };
        HBRUSH hSidebarBg = CreateSolidBrush(0x263238);
        FillRect(hdc, &sidebarBg, hSidebarBg);
        DeleteObject(hSidebarBg);

        // 绘制侧栏顶部装饰
        HBRUSH hTopDecor = CreateSolidBrush(0x4CAF50);
        RECT decorRect = { GAME_WIDTH + 2, 0, clientRect.right, 3 };
        FillRect(hdc, &decorRect, hTopDecor);
        DeleteObject(hTopDecor);

        // 创建字体
        HFONT hFont = CreateFontW(18, 0, 0, 0, FW_BOLD, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        SetTextColor(hdc, COLOR_TEXT);
        SetBkMode(hdc, TRANSPARENT);

        // 绘制标题
        TextOutW(hdc, sidebarX, 20, L"🎮 游戏统计", 8);

        // 绘制分数 - 带图标
        wchar_t scoreText[50];
        swprintf_s(scoreText, 50, L"⭐ 分数: %d", score);
        TextOutW(hdc, sidebarX, 60, scoreText, wcslen(scoreText));

        // 绘制等级 - 带图标
        wchar_t levelText[50];
        swprintf_s(levelText, 50, L"📍 关卡: %d", level);
        TextOutW(hdc, sidebarX, 90, levelText, wcslen(levelText));

        // 绘制长度 - 带图标
        wchar_t lengthText[50];
        swprintf_s(lengthText, 50, L"📏 长度: %d", (int)snake.size());
        TextOutW(hdc, sidebarX, 120, lengthText, wcslen(lengthText));

        // 绘制当前关卡进度 - 带进度条
        wchar_t progressText[100];
        int requiredApples = 0;
        switch (level)
        {
        case 1: requiredApples = LEVEL_1_APPLES; break;
        case 2: requiredApples = LEVEL_2_APPLES; break;
        case 3: requiredApples = LEVEL_3_APPLES; break;
        }
        
        // 绘制进度条背景
        RECT progressBg = { sidebarX, 155, sidebarX + 200, 175 };
        HBRUSH hProgressBg = CreateSolidBrush(0x455A64);
        FillRect(hdc, &progressBg, hProgressBg);
        DeleteObject(hProgressBg);

        // 绘制进度条
        int progressWidth = 0;
        if (requiredApples > 0)
            progressWidth = (currentLevelApples * 200) / requiredApples;
         RECT progressBar = { sidebarX, 155, sidebarX + progressWidth, 175 };
        HBRUSH hProgress = CreateSolidBrush(0x4CAF50);
        FillRect(hdc, &progressBar, hProgress);
        DeleteObject(hProgress);

        // 进度条文字
        HFONT hSmallFont = CreateFontW(11, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        SelectObject(hdc, hSmallFont);
        
        swprintf_s(progressText, 100, L"🍎 进度: %d/%d", currentLevelApples, requiredApples);
        TextOutW(hdc, sidebarX, 180, progressText, wcslen(progressText));

        // 绘制状态信息
        SelectObject(hdc, hSmallFont);

        if (gameState == PLAYING)
        {
            SetTextColor(hdc, 0x4CAF50);
            TextOutW(hdc, sidebarX, 220, L"✓ 游戏中...", 7);
            SetTextColor(hdc, COLOR_TEXT);
            TextOutW(hdc, sidebarX, 245, L"[空格] 暂停", 8);
            TextOutW(hdc, sidebarX, 270, L"[R] 重新开始", 10);
            TextOutW(hdc, sidebarX, 295, L"[ESC] 返回菜单", 9);
        }
        else if (gameState == PAUSED)
        {
            SetTextColor(hdc, 0xFFC107);
            TextOutW(hdc, sidebarX, 220, L"⏸ 已暂停", 5);
            SetTextColor(hdc, COLOR_TEXT);
            TextOutW(hdc, sidebarX, 245, L"[空格] 继续", 8);
            TextOutW(hdc, sidebarX, 270, L"[R] 重新开始", 10);
            TextOutW(hdc, sidebarX, 295, L"[ESC] 返回菜单", 9);
        }
        else if (gameState == GAME_OVER)
        {
            SetTextColor(hdc, 0xF44336);
            
            HFONT hLargeFont = CreateFontW(32, 0, 0, 0, FW_BOLD, 0, 0, 0,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            SelectObject(hdc, hLargeFont);
            
            if (isVictory)
            {
                TextOutW(hdc, GAME_WIDTH / 2 - 100, GAME_HEIGHT / 2 - 50, L"🎉 游戏胜利!", 7);
            }
            else
            {
                TextOutW(hdc, GAME_WIDTH / 2 - 100, GAME_HEIGHT / 2 - 50, L"💀 游戏结束", 7);
            }
            
            SelectObject(hdc, hSmallFont);
            SetTextColor(hdc, COLOR_TEXT);
            
            wchar_t finalScore[100];
            if (isVictory)
            {
                swprintf_s(finalScore, 100, L"🏆 恭喜通关! 总分: %d", score);
            }
            else
            {
                swprintf_s(finalScore, 100, L"📊 最终分数: %d | 到达关卡: %d", score, level);
            }
            TextOutW(hdc, GAME_WIDTH / 2 - 140, GAME_HEIGHT / 2 + 20, finalScore, wcslen(finalScore));
            
            TextOutW(hdc, GAME_WIDTH / 2 - 100, GAME_HEIGHT / 2 + 70, L"[R] 再次游戏", 8);
            
            DeleteObject(hLargeFont);
        }

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        DeleteObject(hSmallFont);
    }

    // 清理：将内存 DC 的内容绘制到窗口
    HDC hdcWin = origHdc;
    BitBlt(origHdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, hOldBitmap);
    DeleteObject(hbm);
    DeleteDC(memDC);

    // 注意：不要在此处 DeleteObject(origHdc) — origHdc 是窗口DC，由系统管理
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CPROJECTSNAKE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CPROJECTSNAKE));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CPROJECTSNAKE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CPROJECTSNAKE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, GAME_WIDTH + 280, GAME_HEIGHT + 40, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT_PTR timerID = 0;

    switch (message)
    {
    case WM_CREATE:
        InitGame();
        srand((unsigned)time(NULL));
        timerID = SetTimer(hWnd, 1, gameSpeed, nullptr);
        break;

    case WM_TIMER:
        // 动画计时器增加，用于食物脉动和其他微动画
        animationTick++;
        if (animationTick > 1000000) animationTick = 0;

        if (gameState == PLAYING)
        {
            UpdateGame();
        }
        InvalidateRect(hWnd, nullptr, FALSE);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:
            if (gameState == PLAYING && dir != RIGHT)
                nextDir = LEFT;
            break;
        case VK_RIGHT:
            if (gameState == PLAYING && dir != LEFT)
                nextDir = RIGHT;
            break;
        case VK_UP:
            if (gameState == PLAYING && dir != DOWN)
                nextDir = UP;
            break;
        case VK_DOWN:
            if (gameState == PLAYING && dir != UP)
                nextDir = DOWN;
            break;
        case VK_SPACE:
            if (gameState == PLAYING)
            {
                gameState = PAUSED;
            }
            else if (gameState == PAUSED)
            {
                gameState = PLAYING;
            }
            else if (gameState == MENU)
            {
                gameState = PLAYING;
                // 重新启动或更新定时器以匹配当前速度
                if (timerID != 0) KillTimer(hWnd, timerID);
                timerID = SetTimer(hWnd, 1, gameSpeed, nullptr);
            }
            break;
        case 0x52: // R键
            ResetGame();
            if (gameState != MENU)
            {
                gameState = PLAYING;
                // 重新启动或更新定时器以匹配当前速度
                if (timerID != 0) KillTimer(hWnd, timerID);
                timerID = SetTimer(hWnd, 1, gameSpeed, nullptr);
            }
            break;
        case VK_ESCAPE:
            if (gameState != MENU)
            {
                gameState = MENU;
            }
            break;
        }
        break;

    case WM_MOUSEMOVE:
        if (gameState == MENU)
        {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            // 使用实际绘制的按钮矩形进行悬停检测，保证一致性
            bool hover = (mx >= g_drawButtonRect.left && mx <= g_drawButtonRect.right && my >= g_drawButtonRect.top && my <= g_drawButtonRect.bottom);
             if (hover != startButtonHover)
             {
                 startButtonHover = hover;
                 InvalidateRect(hWnd, nullptr, FALSE);
             }
         }
         break;

     case WM_LBUTTONDOWN:
         if (gameState == MENU)
         {
             // 检查是否点击了开始按钮
             int x = LOWORD(lParam);
             int y = HIWORD(lParam);

             // 使用实际绘制的按钮矩形进行点击检测
             if (x >= g_drawButtonRect.left && x <= g_drawButtonRect.right && 
                 y >= g_drawButtonRect.top && y <= g_drawButtonRect.bottom)
             {
                 gameState = PLAYING;
                 // 重新启动或更新定时器以匹配当前速度
                 if (timerID != 0) KillTimer(hWnd, timerID);
                 timerID = SetTimer(hWnd, 1, gameSpeed, nullptr);
             }
         }
         break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawGame(hdc, hWnd);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        KillTimer(hWnd, timerID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
