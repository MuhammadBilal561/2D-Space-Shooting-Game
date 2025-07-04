// ================================
// Console Shooting Game
// ================================
// Muhammad BilAL 2025(S)-CS-26
// Description: Console-based shooting game with levels, enemies, bullets,
// and colored display in window console.
// ================================

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <fstream>

using namespace std;

// Game screen dimensions
const int WIDTH = 80;
const int HEIGHT = 26;

// Game state variables
int score = 0;
int lives = 20;
int level = 1;
int playerX;
int playerY;
bool energized = false;
clock_t energizerEndTime = 0;
int enemySpawnRate = 100;
int enemySpawnTimer = 0;
bool gameOver = false;
bool youWin = false;
clock_t lastShotTime = 0;
int enemyMoveTimer = 0;
int enemyMoveDelay = 5; // Controls enemy movement speed
// Player sprite
char playerSpriteData[3][9] = {
    "   /\\   ",
    "  <==>  ",
    " /_WW_\\ "};
const int playerWidth = 8;
const int playerHeight = 3;
// Enemy sprite
char enemySpriteData[2][7] = {
    "-[VV]-",
    "  vVv "};
const int enemyWidth = 6;
const int enemyHeight = 2;

// Bullet and enemy arrays
int bulletX[20];
int bulletY[20];
bool bulletActive[20];

int enemyX[30];
int enemyY[30];
bool enemyActive[30];

int enemyBulletX[30];
int enemyBulletY[30];
bool enemyBulletActive[30];

// Map and rendering buffers
char gameMap[HEIGHT][WIDTH + 1];
char screenBuffer[HEIGHT][WIDTH + 1];

// Console color constants
const int COLOR_DEFAULT = 7;
const int COLOR_ENEMY = 15;
const int COLOR_BULLET = 15;
const int COLOR_SCORE = 11;
const int COLOR_SCORE_ALT = 10;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// Move console cursor to specific coordinate
void gotoxy(int x, int y)
{
    COORD c = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// Hide the blinking text cursor
void hideCursor()
{
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

// Load map from text file
void loadMap()
{
    fstream file("map.txt", ios::in);
    string line;

    if (file.is_open())
    {
        for (int i = 0; i < HEIGHT; ++i)
        {
            if (getline(file, line))
            {
                for (int j = 0; j < WIDTH; ++j)
                {
                    gameMap[i][j] = (j < line.length()) ? line[j] : ' ';
                }
                gameMap[i][WIDTH] = '\0';
            }
            else
            {
                for (int j = 0; j <= WIDTH; ++j)
                    gameMap[i][j] = ' ';
            }
        }
        file.close();
    }

    // Copy map into screen buffer
    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j <= WIDTH; ++j)
        {
            screenBuffer[i][j] = gameMap[i][j];
        }
    }
}

// Set up initial game state
void initialize()
{
    srand(time(0));
    hideCursor();
    loadMap();

    playerX = WIDTH / 2 - playerWidth / 2;
    playerY = HEIGHT - playerHeight - 2;

    // Deactivate all bullets and enemies
    for (int i = 0; i < 20; ++i)
        bulletActive[i] = false;
    for (int i = 0; i < 30; ++i)
    {
        enemyActive[i] = false;
        enemyBulletActive[i] = false;
    }
}

// Fire a bullet from the player
void shoot()
{
    for (int i = 0; i < 20; ++i)
    {
        if (!bulletActive[i])
        {
            bulletActive[i] = true;
            bulletX[i] = playerX + playerWidth / 2;
            bulletY[i] = playerY - 1;
            return;
        }
    }
}

// Enemy shoots a bullet downward
void enemyShoot(int x, int y, int index)
{
    if (!enemyBulletActive[index])
    {
        enemyBulletActive[index] = true;
        enemyBulletX[index] = x + enemyWidth / 2;
        enemyBulletY[index] = y + enemyHeight;
    }
}

// Handle player movement and actions
void processInput()
{
    int moveSpeed = 2;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState('A') & 0x8000)
        if (playerX - moveSpeed > 0)
            playerX -= moveSpeed;

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState('D') & 0x8000)
        if (playerX + playerWidth + moveSpeed < WIDTH)
            playerX += moveSpeed;

    if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState('W') & 0x8000)
        if (playerY - 1 > 0)
            playerY--;

    if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState('S') & 0x8000)
        if (playerY + playerHeight < HEIGHT - 1)
            playerY++;

    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        if (clock() - lastShotTime > 150)
        {
            shoot();
            lastShotTime = clock();
        }

    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        gameOver = true;
}

// Axis-Aligned Bounding Box Collision
bool checkAABBCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Show level up message
void nextLevelMessage(int nextLevel)
{
    system("cls");
    gotoxy(WIDTH / 2 - 10, HEIGHT / 2);
    cout << "Level " << nextLevel - 1 << " Complete!";
    gotoxy(WIDTH / 2 - 10, HEIGHT / 2 + 1);
    cout << "Get Ready for Level " << nextLevel << "!";
    Sleep(3000);
}

// Update all game mechanics
void updateGame()
{
    // Move bullets upward
    for (int i = 0; i < 20; ++i)
    {
        if (bulletActive[i])
        {
            bulletY[i]--;
            if (bulletY[i] < 1 || gameMap[bulletY[i]][bulletX[i]] == '*')
                bulletActive[i] = false;
        }
    }

    // Move enemy bullets downward
    for (int i = 0; i < 30; ++i)
    {
        if (enemyBulletActive[i])
        {
            enemyBulletY[i]++;
            if (enemyBulletY[i] >= HEIGHT - 1)
                enemyBulletActive[i] = false;

            if (checkAABBCollision(enemyBulletX[i], enemyBulletY[i], 1, 1, playerX, playerY, playerWidth, playerHeight))
            {
                lives--;
                enemyBulletActive[i] = false;
            }
        }
    }

    // Spawn enemies per level rule
    enemySpawnTimer++;
    if (enemySpawnTimer >= enemySpawnRate)
    {
        enemySpawnTimer = 0;
        int numToSpawn = (level == 1) ? 1 : (level == 2) ? 3
                                                         : 5;
        int spawned = 0;
        for (int i = 0; i < 30 && spawned < numToSpawn; ++i)
        {
            if (!enemyActive[i])
            {
                enemyActive[i] = true;
                enemyY[i] = 1;
                enemyX[i] = (rand() % (WIDTH - enemyWidth - 2)) + 1;
                spawned++;
            }
        }
    }

    // Move active enemies
    enemyMoveTimer++;
    if (enemyMoveTimer > enemyMoveDelay)
    {
        enemyMoveTimer = 2;
        for (int i = 0; i < 30; ++i)
        {
            if (enemyActive[i])
            {
                enemyY[i]++;
                if (rand() % 4 == 0)
                    enemyShoot(enemyX[i], enemyY[i], i);
            }
        }
    }

    // Check bullet hit on enemy
    for (int i = 0; i < 30; ++i)
    {
        if (!enemyActive[i])
            continue;
        for (int j = 0; j < 20; ++j)
        {
            if (bulletActive[j] &&
                checkAABBCollision(enemyX[i], enemyY[i], enemyWidth, enemyHeight, bulletX[j], bulletY[j], 1, 1))
            {
                enemyActive[i] = false;
                bulletActive[j] = false;
                score += energized ? 20 : 10;
            }
        }
    }

    // Collect energizer if touched
    for (int y = 0; y < playerHeight; ++y)
        for (int x = 0; x < playerWidth; ++x)
            if (playerY + y < HEIGHT && playerX + x < WIDTH && gameMap[playerY + y][playerX + x] == 'E')
            {
                gameMap[playerY + y][playerX + x] = ' ';
                energized = true;
                energizerEndTime = clock() + 10 * CLOCKS_PER_SEC;
            }

    if (energized && clock() > energizerEndTime)
        energized = false;

    // Handle level transitions
    if (level == 1 && score >= 200)
    {
        level++;
        enemyMoveDelay = 4;
        nextLevelMessage(level);
        fill(begin(enemyActive), end(enemyActive), false);
    }
    else if (level == 2 && score >= 350)
    {
        level++;
        enemyMoveDelay = 3;
        nextLevelMessage(level);
        fill(begin(enemyActive), end(enemyActive), false);
    }
    else if (level == 3 && score >= 500)
    {
        youWin = true;
        return;
    }

    if (lives <= 0)
        gameOver = true;
}

// Draw any sprite onto buffer
void drawToBuffer(const char *sprite_ptr, int spriteHeight, int spriteWidth, int spriteRowStride, int x, int y)
{
    for (int i = 0; i < spriteHeight; ++i)
        for (int j = 0; j < spriteWidth; ++j)
        {
            if (y + i < HEIGHT && x + j < WIDTH)
            {
                char ch = *(sprite_ptr + i * spriteRowStride + j);
                if (ch != ' ')
                    screenBuffer[y + i][x + j] = ch;
            }
        }
}

// Draw all entities and HUD
void render()
{
    // Copy static map to buffer
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j <= WIDTH; ++j)
            screenBuffer[i][j] = gameMap[i][j];

    // Draw player
    drawToBuffer(&playerSpriteData[0][0], playerHeight, playerWidth, 9, playerX, playerY);

    // Draw enemies
    SetConsoleTextAttribute(hConsole, COLOR_ENEMY);
    for (int i = 0; i < 30; ++i)
        if (enemyActive[i])
            drawToBuffer(&enemySpriteData[0][0], enemyHeight, enemyWidth, 7, enemyX[i], enemyY[i]);

    // Draw player bullets
    SetConsoleTextAttribute(hConsole, COLOR_BULLET);
    for (int i = 0; i < 20; ++i)
        if (bulletActive[i] && bulletY[i] >= 0 && bulletY[i] < HEIGHT && bulletX[i] >= 0 && bulletX[i] < WIDTH)
            screenBuffer[bulletY[i]][bulletX[i]] = '^';

    // Draw enemy bullets
    SetConsoleTextAttribute(hConsole, COLOR_ENEMY);
    for (int i = 0; i < 30; ++i)
        if (enemyBulletActive[i] && enemyBulletY[i] >= 0 && enemyBulletY[i] < HEIGHT && enemyBulletX[i] >= 0 && enemyBulletX[i] < WIDTH)
            screenBuffer[enemyBulletY[i]][enemyBulletX[i]] = 'v';

    // Draw game stats
    SetConsoleTextAttribute(hConsole, COLOR_SCORE);
    string stats = "LVL: " + to_string(level) + " | SCORE: " + to_string(score) + " | LIVES: " + to_string(lives);
    if (energized)
    {
        stats += " | ENERGIZED!";
        SetConsoleTextAttribute(hConsole, COLOR_SCORE_ALT);
    }

    for (int j = 0; j < stats.length() && j + 2 < WIDTH; ++j)
        screenBuffer[HEIGHT - 1][j + 2] = stats[j];
    for (int j = stats.length() + 2; j < WIDTH - 1; ++j)
        screenBuffer[HEIGHT - 1][j] = ' ';
    screenBuffer[HEIGHT - 1][WIDTH] = '\0';

    // Print full buffer to screen
    gotoxy(0, 0);
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
    for (int i = 0; i < HEIGHT; ++i)
    {
        if (i == HEIGHT - 1)
            SetConsoleTextAttribute(hConsole, energized ? COLOR_SCORE_ALT : COLOR_SCORE);
        cout << screenBuffer[i] << (i == HEIGHT - 1 ? "" : "\n");
        if (i == HEIGHT - 1)
            SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
    }
}

// Splash title screen
void showTitle()
{
    system("cls");
    SetConsoleTextAttribute(hConsole, COLOR_SCORE_ALT);

    cout << R"(

          ____   ____   ____   ____  ______ 
         _       _   _ _    _ _      _     
          ___    ____  ______ _      ____  
             _   _     _    _ _      _     
          ____   _     _    _  ____  ______ 

           ____   _   _   ____    ____   ______ __  _   _   ____ 
         _        _   _   _    _  _    _   __   __  __  _  _     
          ______  _____  _    _  _    _    __   __  _ _ _  _ ___ 
               _  _   _  _    _  _    _    __   __  _  __  _   _ 
          ____    _   _   ____    ____     __   __  _   _   ____                                                          

    )";

    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
    cout << "\n\nPress any key to start...";
    _getch();
}

// Entry point
int main()
{
    showTitle();
    initialize();

    while (!gameOver && !youWin)
    {
        processInput();
        updateGame();
        render();
        Sleep(20);
    }

    // Game end message
    system("cls");
    gotoxy(WIDTH / 2 - 10, HEIGHT / 2);
    if (youWin)
        cout << "YOU WIN! Final Score: " << score << endl
             << endl;
    else
        cout << "GAME OVER! Final Score: " << score << endl
             << endl;

    _getch();
    return 0;
}
