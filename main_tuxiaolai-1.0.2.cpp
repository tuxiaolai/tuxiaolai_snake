// ^_～ Initialized
// All-Kill Automaton   Debug++
// [RUN] BUILD MAP SPAWN PLAYER...

// NOTE:暂不确定问题是否解决
// TODO:增加暂停

#define UNICODE
#define _UNICODE

#include <graphics.h>
#include <windows.h>
#include <chrono>
#include <deque>
#include <iostream>
#include <queue>
#include <random>

using ull = unsigned long long;
typedef std::chrono::time_point<std::chrono::system_clock> time_point_sc;

const int WINDOW_WIDTH = 80, WINDOW_HEIGHT = 45, SQUARE_SIZE = 10, BORDER_SIZE = 5; //用作游戏画面绘制的参数
const int FPS = 15; //每秒移动的格数，也可作为帧数
const COLORREF BKCOLOR = RGB(0xf5, 0xf5, 0xf5), LINECOLOR = RGB(0xc8, 0xc8, 0xc8); //背景颜色 网格颜色
const COLORREF HEADCOLOR1 = RGB(0x70, 0x1f, 0x7e), BODYCOLOR1 = RGB(0xe6, 0x29, 0x37); //玩家一 颜色
const COLORREF HEADCOLOR2 = RGB(0x00, 0x52, 0xac), BODYCOLOR2 = RGB(0x00, 0x79, 0xf1); //玩家二 颜色
const COLORREF FOODCOLOR = RGB(0x66, 0xbf, 0xff);
const int SPAWNX1 = 5, SPAWNY1 = 10; //玩家一出身点
const int SPAWNX2 = 5, SPAWNY2 = 12; //玩家二出身点
const int FOOD_NUM = 10;

//用作方向指示
const int STOP = 0x00;
const int UP = 0x01;
const int RIGHT = 0x02;
const int DOWN = 0x03;
const int LEFT = 0x04;

//用作方格种类指示

const int EMPTY = 0x10;
const int HEAD1 = 0x11;
const int HEAD2 = 0x12;
const int BODY1 = 0x21;
const int BODY2 = 0x22;
const int FOOD = 0x31;

struct Square {
    int x, y;

    Square(int x, int y) : x(x), y(y) {
    }

    bool operator==(const Square &other) const {
        return x == other.x && y == other.y;
    }
}; //方格

struct Player {
    Square head;
    int direction;
    int length;
    std::deque<Square> squares;

    //move
    bool change_direction = false;
    int direction2 = RIGHT;


    Player(Square square, int direction = RIGHT, int length = 0) : head(square), direction(direction),
                                                                   length(length) {
    }

    void InitUpdate(int playerid);

    void HeadMove(int playerid);

    void TailMove(int playerid);

    void Move(int playerid);
} player1 = Player(Square(SPAWNX1, SPAWNY1)), player2 = Player(Square(SPAWNX2, SPAWNY2)); //玩家


bool Stop = false;
int cnt[WINDOW_WIDTH + 10][WINDOW_HEIGHT + 10][256];
int changed_square_type[WINDOW_WIDTH + 10][WINDOW_HEIGHT + 10];
std::queue<Square> changed_squares;

std::random_device rd; // 用于生成种子
std::mt19937 gen(rd()); // 随机数引擎
std::uniform_int_distribution dis_x(1, WINDOW_WIDTH);
std::uniform_int_distribution dis_y(1, WINDOW_HEIGHT);


time_point_sc hrc_now(); //获取当前时间
ull diff_time(time_point_sc start, time_point_sc end); //返回时间差，单位ms

void setlfcolor(COLORREF color); //同时设置线条颜色和填充颜色
Square Next_Square(Square u, int direction); //该方格该方向的下一个格子

void DrawSquare(Square square, COLORREF color, bool flush); //填充方格
void ClearSquare(Square square, bool flush); //清空方格

void InitGame(); //初始化游戏变量及画面
void DrawFrame(); //渲染帧
bool KeyEvent(); //键盘事件 返回值true表示退出游戏
bool UpdateGame(); //游戏更新逻辑 返回值同KeyEvent();
void SpawnFood(); //随机生成食物
void EndGame(int end); //结束游戏

int main() {
    InitGame();
    while (true) {
        auto start = hrc_now();

        bool res = UpdateGame();
        if (res) {
            break;
        }
        auto end = hrc_now();
        ull diff = diff_time(start, end);
        if (diff < 1000 / FPS) {
            Sleep(1000 / FPS - diff);
        }
    }
    EndBatchDraw();
    return 0;
}

time_point_sc hrc_now() {
    return std::chrono::high_resolution_clock::now();
}

ull diff_time(time_point_sc start, time_point_sc end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void setlfcolor(COLORREF color) {
    setlinecolor(color);
    setfillcolor(color);
}

Square Next_Square(Square u, int direction) {
    if (direction == UP) {
        u.y--;
    } else if (direction == DOWN) {
        u.y++;
    } else if (direction == LEFT) {
        u.x--;
    } else if (direction == RIGHT) {
        u.x++;
    }
    if (u.x == 0) {
        u.x = WINDOW_WIDTH;
    }
    if (u.y == 0) {
        u.y = WINDOW_HEIGHT;
    }
    if (u.x == WINDOW_WIDTH + 1) {
        u.x = 1;
    }
    if (u.y == WINDOW_HEIGHT + 1) {
        u.y = 1;
    }
    return u;
}

void DrawSquare(Square square, COLORREF color, bool flush = true) {
    setlfcolor(color);
    fillrectangle((square.x - 1) * SQUARE_SIZE + BORDER_SIZE + 1, (square.y - 1) * SQUARE_SIZE + BORDER_SIZE + 1,
                  square.x * SQUARE_SIZE + BORDER_SIZE, square.y * SQUARE_SIZE + BORDER_SIZE);
    if (!flush) {
        return;
    }
    FlushBatchDraw();
}

void ClearSquare(Square square, bool flush = true) {
    setfillcolor(BKCOLOR);
    setlinecolor(LINECOLOR);
    solidrectangle((square.x - 1) * SQUARE_SIZE + BORDER_SIZE + 1, (square.y - 1) * SQUARE_SIZE + BORDER_SIZE + 1,
                   square.x * SQUARE_SIZE + BORDER_SIZE - 1, square.y * SQUARE_SIZE + BORDER_SIZE - 1);
    line((square.x - 1) * SQUARE_SIZE + BORDER_SIZE, square.y * SQUARE_SIZE + BORDER_SIZE,
         square.x * SQUARE_SIZE + BORDER_SIZE, square.y * SQUARE_SIZE + BORDER_SIZE);
    line(square.x * SQUARE_SIZE + BORDER_SIZE, (square.y - 1) * SQUARE_SIZE + BORDER_SIZE,
         square.x * SQUARE_SIZE + BORDER_SIZE, square.y * SQUARE_SIZE + BORDER_SIZE);
    if (!flush) {
        return;
    }
    FlushBatchDraw();
}

void Player::InitUpdate(int playerid) {
    change_direction = false;
    direction2 = STOP;
}

void Player::HeadMove(int playerid) {
    //将头部向前移，并将原头部换成身体
    Square new_head = Next_Square(head, direction);
    int Head = playerid == 1 ? HEAD1 : HEAD2;
    int Body = playerid == 1 ? BODY1 : BODY2;
    int x, y;

    x = head.x;
    y = head.y;
    if (length) {
        changed_square_type[x][y] = Body;
        cnt[x][y][Body]++;
        squares.push_front(head);
    } else {
        changed_square_type[x][y] = EMPTY;
    }
    cnt[x][y][Head]--;
    x = new_head.x;
    y = new_head.y;
    changed_square_type[x][y] = Head;
    cnt[x][y][Head]++;

    changed_squares.push(head);
    changed_squares.push(new_head);
    head = new_head;
}

void Player::TailMove(int playerid) {
    int Body = playerid == 1 ? BODY1 : BODY2;
    if (length) {
        int x, y;
        x = squares.back().x;
        y = squares.back().y;
        changed_squares.push(squares.back());
        changed_square_type[x][y] = EMPTY;
        cnt[x][y][Body]--;
        squares.pop_back();
    }
}

void Player::Move(int playerid) {
    if (!cnt[head.x][head.y][FOOD]) {
        TailMove(playerid);
    } else {
        cnt[head.x][head.y][FOOD]--;
        length++;
        SpawnFood();
    }
    HeadMove(playerid);
}

void InitGame() {
    initgraph(WINDOW_WIDTH * SQUARE_SIZE + BORDER_SIZE * 2, WINDOW_HEIGHT * SQUARE_SIZE + BORDER_SIZE * 2);
    BeginBatchDraw();

    setbkcolor(BKCOLOR); //背景颜色
    cleardevice();

    setlinecolor(LINECOLOR); //网格颜色
    for (int i = 0; i <= WINDOW_WIDTH; i++) {
        line(i * SQUARE_SIZE + BORDER_SIZE, BORDER_SIZE, i * SQUARE_SIZE + BORDER_SIZE,
             WINDOW_HEIGHT * SQUARE_SIZE + BORDER_SIZE);
    }
    for (int i = 0; i <= WINDOW_HEIGHT; i++) {
        line(BORDER_SIZE, i * SQUARE_SIZE + BORDER_SIZE, WINDOW_WIDTH * SQUARE_SIZE + BORDER_SIZE,
             i * SQUARE_SIZE + BORDER_SIZE);
    }
    FlushBatchDraw();

    cnt[player1.head.x][player1.head.y][HEAD1]++;
    cnt[player2.head.x][player2.head.y][HEAD2]++;
    changed_squares.push(player1.head);
    changed_squares.push(player2.head);
    for (int i = 1; i <= FOOD_NUM; i++) {
        SpawnFood();
    }
    DrawFrame();
    std::cout << "WINDOW_WIDTH: " << WINDOW_WIDTH << std::endl;
    std::cout << "WINDOW_HEIGHT: " << WINDOW_HEIGHT << std::endl;
    std::cout << "SQUARE_SIZE: " << SQUARE_SIZE << std::endl;
    std::cout << "BORDER_SIZE: " << BORDER_SIZE << std::endl;
    std::cout << "Init OK!" << std::endl;
}

void DrawFrame() {
    while (!changed_squares.empty()) {
        Square u = changed_squares.front();
        changed_squares.pop();
        int val = changed_square_type[u.x][u.y];
        if (val == EMPTY) {
            if (cnt[u.x][u.y][HEAD1]) {
                val = HEAD1;
            } else if (cnt[u.x][u.y][HEAD2]) {
                val = HEAD2;
            } else if (cnt[u.x][u.y][BODY1]) {
                val = BODY1;
            } else if (cnt[u.x][u.y][BODY2]) {
                val = BODY2;
            } else if (cnt[u.x][u.y][FOOD]) {
                val = FOOD;
            } else {
                ClearSquare(u, false);
                continue;
            }
        }
        if (val == HEAD1) {
            DrawSquare(u, HEADCOLOR1, false);
        } else if (val == BODY1) {
            DrawSquare(u, BODYCOLOR1, false);
        } else if (val == HEAD2) {
            DrawSquare(u, HEADCOLOR2, false);
        } else if (val == BODY2) {
            DrawSquare(u, BODYCOLOR2, false);
        } else if (val == FOOD) {
            DrawSquare(u, FOODCOLOR, false);
        }
    }
    FlushBatchDraw();
}

bool KeyEvent() {
    int direction1 = player1.direction;
    int direction2 = player2.direction;
    ExMessage msg;
    while (peekmessage(&msg)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.vkcode == VK_SPACE) {
                Stop ^= 1;
            }
            if (!Stop) {
                switch (msg.vkcode) {
                    case 'W':
                        if (direction1 == DOWN || direction1 == UP) {
                            player1.direction2 = UP;
                            break;
                        }
                        player1.change_direction = true;
                        player1.direction = UP;
                        break;
                    case 'S':
                        if (direction1 == UP || player1.direction == DOWN) {
                            player1.direction2 = DOWN;
                            break;
                        }
                        player1.change_direction = true;
                        player1.direction = DOWN;
                        break;
                    case 'A':
                        if (direction1 == RIGHT || direction1 == LEFT) {
                            player1.direction2 = LEFT;
                            break;
                        }
                        player1.change_direction = true;
                        player1.direction = LEFT;
                        break;
                    case 'D':
                        if (direction1 == LEFT || direction1 == RIGHT) {
                            player1.direction2 = RIGHT;
                            break;
                        }
                        player1.change_direction = true;
                        player1.direction = RIGHT;
                        break;

                    case VK_UP:
                        if (direction2 == DOWN || direction2 == UP) {
                            player2.direction2 = UP;
                            break;
                        }
                        player2.change_direction = true;
                        player2.direction = UP;
                        break;
                    case VK_DOWN:
                        if (direction2 == UP || direction2 == DOWN) {
                            player2.direction2 = DOWN;
                            break;
                        }
                        player2.change_direction = true;
                        player2.direction = DOWN;
                        break;
                    case VK_LEFT:
                        if (direction2 == RIGHT || direction2 == LEFT) {
                            player2.direction2 = LEFT;
                            break;
                        }
                        player2.change_direction = true;
                        player2.direction = LEFT;
                        break;
                    case VK_RIGHT:
                        if (direction2 == LEFT || direction2 == RIGHT) {
                            player2.direction2 = RIGHT;
                            break;
                        }
                        player2.change_direction = true;
                        player2.direction = RIGHT;
                        break;
                    case VK_ESCAPE:
                        return true;
                }
            }
        }
    }
    return false;
}

bool UpdateGame() {
    player1.InitUpdate(1);
    player2.InitUpdate(2);

    bool res = KeyEvent();
    if (res) {
        return true;
    }
    if (Stop) {
        return false;
    }
    player1.Move(1);
    player2.Move(2);

    if (player1.change_direction && player1.direction2 != STOP) {
        player1.direction = player1.direction2;
    }
    if (player2.change_direction && player2.direction2 != STOP) {
        player2.direction = player2.direction2;
    }

    bool die1 = cnt[player1.head.x][player1.head.y][HEAD2] || cnt[player1.head.x][player1.head.y][BODY2];
    bool die2 = cnt[player2.head.x][player2.head.y][HEAD1] || cnt[player2.head.x][player2.head.y][BODY1];
    if (player1.head == player2.head || (die1 && die2)) {
        EndGame(0);
        return true;
    }
    if (die1) {
        EndGame(2);
        return true;
    }
    if (die2) {
        EndGame(1);
        return true;
    }

    DrawFrame();
    return false;
}

void SpawnFood() {
    int x = dis_x(gen);
    int y = dis_y(gen);
    cnt[x][y][FOOD]++;
    changed_square_type[x][y] = EMPTY;
    changed_squares.push(Square(x, y));
}

void EndGame(int end) {
    Sleep(500);
    setbkcolor(BKCOLOR);
    cleardevice();
    if (end == 0) {
        settextcolor(LINECOLOR);
        outtextxy(10, 10,_T("Draw"));
    } else if (end == 1) {
        settextcolor(BODYCOLOR1);
        outtextxy(10, 10,_T("Player1 Win"));
    } else if (end == 2) {
        settextcolor(BODYCOLOR2);
        outtextxy(10, 10,_T("Player2 Win"));
    }
    FlushBatchDraw();
    Sleep(1000);
}
