#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "raylib.h"

// TODO: Melhorar organização do código

#define BALL_SIZE      50
#define BALL_SPEED     600
#define PADDING_TEXT   50
#define PADDLE_LENGTH  BALL_SIZE 
#define PADDLE_HEIGHT  BALL_SIZE * 4
#define SCREEN_WIDTH   1200
#define SCREEN_HEIGHT  850
#define SPAWN_DELAY    300
#define AI_QUADRANT_MULTPLIER 3
#define PLAYER_QUADRANT_MULTIPLIER 1
#define MENU_ITEMS_LENGTH 3
#define AI_MATCHUP_ITEMS_LENGTH 3
#define SOUNDS 5

Vector2 ball_position = { SCREEN_WIDTH/2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 };
Vector2 ball_speed    = { BALL_SPEED, BALL_SPEED };

Vector2 paddle_left_position  = { BALL_SIZE, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2 };
Vector2 paddle_right_position = { SCREEN_WIDTH - BALL_SIZE * 2, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2 };

struct Rectangle paddle_right = {0};
struct Rectangle paddle_left = {0};
struct Rectangle ball = {0};

struct Rectangle AIDetectRec = {0};
struct Rectangle AIDetectRecUpAiQuadrant = {0};
struct Rectangle AIDetectRecDownAiQuadrant = {0};
struct Rectangle AIDetectRecUpPlayerQuadrant = {0};
struct Rectangle AIDetectRecDownPlayerQuadrant = {0};

enum MENU_LAYER 
{
    ML_MAIN,
    ML_AI_MATCHUP
};

enum GAME_MODE 
{
    AI,
    LOCAL,
    ONLINE
};

enum AI_MATCH_MODE
{
    INFINITE,
    LIMITED
};
 
// TODO: Implementar modo de jogo online
char *items[] = 
{
    "Você vs IA [1]",
    "Você vs jogador (local) [2]",
    "Você vs jogador (online) [Em desenvolvimento!] [3]",
};

char *ai_matchup_items[] = 
{
    "Tipo de partida: Infinito [1]",
    "Tipo de partida: Limitada [2]",
    "Voltar [3]"
};

char **menu_items = items;

int paddle_left_score = 0;
int paddle_right_score = 0;
int pause = 1;
int match_ended = 0;
struct Rectangle *winner = NULL;
int player_paddle_touched = 1;
int spawn_delay = SPAWN_DELAY;

enum GAME_MODE game_mode_selected = AI;
enum AI_MATCH_MODE ai_match_up_mode = INFINITE;
enum MENU_LAYER menu_layer = ML_MAIN;

Sound soundArray[SOUNDS] = {0};

static void load_sounds_array() 
{
    soundArray[0] = LoadSound("assets/sounds/menu_select.wav");
    soundArray[1] = LoadSound("assets/sounds/paddle_bump1.wav");
    soundArray[2] = LoadSound("assets/sounds/paddle_bump2.wav");
    soundArray[3] = LoadSound("assets/sounds/paddle_limit.wav");
    soundArray[4] = LoadSound("assets/sounds/scoring.wav");
}

static void restart()
{
    ball.x = ball_position.x;
    ball.y = ball_position.y;

    paddle_left.y = paddle_left_position.y;
    paddle_right.y = paddle_right_position.y;

    paddle_left_score = 0;
    paddle_right_score = 0;

    pause = 0;

    spawn_delay = SPAWN_DELAY;

    match_ended = 0;
    winner = NULL;
}

static void draw_menu_options(char **list_items, int total_items)
{
    for(int i = 0; i < total_items; ++i) 
    {
        DrawText(list_items[i], GetScreenWidth() / 3 - PADDING_TEXT * 3, GetScreenHeight() / 2 + PADDING_TEXT * i + 3, 32, BLACK);
    }
}

static void show_ml_main_menu()
{
    draw_menu_options(menu_items, MENU_ITEMS_LENGTH);

    if (IsKeyDown(KEY_KP_1)) 
    {
        game_mode_selected = AI;
        PlaySound(soundArray[0]);
        pause = 1;
        menu_layer = ML_AI_MATCHUP;

        WaitTime(0.5); // melhorar isso
    } 
    else if (IsKeyDown(KEY_KP_2)) 
    {
        game_mode_selected = LOCAL;
        PlaySound(soundArray[0]);
        pause = 0;

        WaitTime(0.5);
    } 
    else if (IsKeyDown(KEY_KP_3)) 
    {
        game_mode_selected = ONLINE;
        PlaySound(soundArray[0]);
    }

    DrawText("Aperte r para reiniciar", GetScreenWidth() / 2 - PADDING_TEXT * 3, GetScreenHeight() - PADDING_TEXT, 42, BLACK);
}

static void show_ml_ai_matchup_menu() 
{
    draw_menu_options(ai_matchup_items, AI_MATCHUP_ITEMS_LENGTH);

    if (IsKeyDown(KEY_KP_1)) 
    {
        ai_match_up_mode = INFINITE;
        PlaySound(soundArray[0]);
        pause = 0;

        WaitTime(0.5);
    }
    else if (IsKeyDown(KEY_KP_2)) 
    {
        ai_match_up_mode = LIMITED;
        PlaySound(soundArray[0]);
        pause = 0;

        WaitTime(0.5);
    } 
    else if (IsKeyDown(KEY_KP_3)) 
    {
        PlaySound(soundArray[0]);
        menu_layer = ML_MAIN;

        WaitTime(0.5);
    }
}

static void show_ended_match_menu(struct Rectangle *pWinner) 
{
    if (pWinner == NULL) return;

    ClearBackground(WHITE);

    DrawText("Partida finalizada", GetScreenWidth() / 3, GetScreenHeight() / 3, 32, BLACK);
    
    if (game_mode_selected == AI) 
    {
        if (pWinner == &paddle_left) 
        {
            DrawText("Jogador venceu", GetScreenWidth() / 3, GetScreenHeight() / 3 + BALL_SIZE, 32, BLACK);
        } 
        else if (pWinner == &paddle_right)
        {
            DrawText("IA venceu", GetScreenWidth() / 3, GetScreenHeight() / 3 + BALL_SIZE, 32, BLACK);
        }
    }

    if (match_ended == 1 && IsKeyDown(KEY_R)) 
    {
        restart();
    }

    DrawText("Aperte r para reiniciar", GetScreenWidth() / 2 - PADDING_TEXT * 3, GetScreenHeight() - PADDING_TEXT, 42, BLACK);
}

static void show_menu() // futuramente tirar isso, menu não precisa saber vencedor
{
    ClearBackground(WHITE);

    if (menu_layer == ML_MAIN) 
    {
        show_ml_main_menu();
    }
    else if (menu_layer == ML_AI_MATCHUP) 
    {
        show_ml_ai_matchup_menu();
    }
}

static void show_score() 
{
    DrawText(TextFormat("%00i", paddle_left_score), GetScreenWidth() / 2 - 18 + PADDING_TEXT, PADDING_TEXT * 0.5, 72, WHITE);
    DrawText(TextFormat("%00i", paddle_right_score), GetScreenWidth() / 2 - 18 - PADDING_TEXT, PADDING_TEXT * 0.5, 72, WHITE);
}

static void restart_paddle_position(struct Rectangle *pPaddle, Vector2 position) 
{
    pPaddle->x = position.x;
    pPaddle->y = position.y;
}

static void restart_ball_position(struct Rectangle *pBall, Vector2 position) 
{
    pBall->x = position.x;
    pBall->y = position.y;
}

static void left_paddle_score(float current_ball_position) 
{
    if (current_ball_position < 0) 
    {
        PlaySound(soundArray[4]);
        paddle_left_score++;

        restart_paddle_position(&paddle_left, paddle_left_position);
        restart_paddle_position(&paddle_right, paddle_right_position);

        restart_ball_position(&ball, ball_position);
    }
}

static void right_paddle_score(float current_ball_position) 
{
    if (current_ball_position + BALL_SIZE > GetScreenWidth()) 
    {
        PlaySound(soundArray[4]);
        paddle_right_score++;
        
        restart_paddle_position(&paddle_left, paddle_left_position);
        restart_paddle_position(&paddle_right, paddle_right_position);

        restart_ball_position(&ball, ball_position);
    }
}

static int check_ball_wall_collision(float current_ball_position, int screen_height) 
{
    if (current_ball_position < 0 || current_ball_position + BALL_SIZE >= screen_height) 
    {
        return 1;
    }

    return 0;
}

static void bounce_ball(float current_ball_position, int screen_height, 
    struct Rectangle *pBall, Vector2 *pBall_speed) 
{
    if (check_ball_wall_collision(current_ball_position, screen_height) == 1) 
    {
        pBall_speed->y = -pBall_speed->y;
    } 
    else 
    {
        pBall->y = current_ball_position;
    }
}

static void doPause() 
{
    if (IsKeyPressed(KEY_SPACE) && pause == 0) 
    {
        pause = 1;
        menu_layer = ML_MAIN;
    } 
    else if (IsKeyPressed(KEY_SPACE) && pause == 1) 
    {
        pause = 0;
        menu_layer = ML_MAIN;
    }
}

static void force_stop()
{
    ball.x = ball_position.x;
    ball.y = ball_position.y;

    paddle_left.y = paddle_left_position.y;
    paddle_right.y = paddle_right_position.y;

    paddle_left_score = 0;
    paddle_right_score = 0;

    pause = 0;

    spawn_delay = SPAWN_DELAY;
}

char *get_game_mode(enum GAME_MODE mode) 
{
    switch(mode) 
    {
        case AI: return "Contra IA";
        case LOCAL: return "Local";
        case ONLINE: return "online";
        default: return "Contra IA";
    }
}

char *get_ai_matchup(enum AI_MATCH_MODE mode) 
{
    switch(mode) 
    {
        case INFINITE: return "Partida infinita";
        case LIMITED: return "Partida limitada: 3 pontos";
        default: return "Partida infinita";
    }
}

int main(void) 
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong por Gsuc");
        
    paddle_left.height = PADDLE_HEIGHT;
    paddle_left.width = PADDLE_LENGTH;
    paddle_left.x = paddle_left_position.x;
    paddle_left.y = paddle_left_position.y;

    paddle_right.height = PADDLE_HEIGHT;
    paddle_right.width = PADDLE_LENGTH;
    paddle_right.x = paddle_right_position.x;
    paddle_right.y = paddle_right_position.y;

    ball.height = BALL_SIZE;
    ball.width = BALL_SIZE;
    ball.x = ball_position.x;
    ball.y = ball_position.y;

    AIDetectRecUpAiQuadrant.width = GetScreenWidth() / 2;
    AIDetectRecUpAiQuadrant.height = GetScreenHeight() / 2;
    AIDetectRecUpAiQuadrant.x = GetScreenWidth() / 2;
    AIDetectRecUpAiQuadrant.y = 0;
    
    AIDetectRecDownAiQuadrant.width = GetScreenWidth() / 2;
    AIDetectRecDownAiQuadrant.height = GetScreenHeight() / 2;
    AIDetectRecDownAiQuadrant.x = GetScreenWidth() / 2;
    AIDetectRecDownAiQuadrant.y = GetScreenHeight() / 2;

    AIDetectRecUpPlayerQuadrant.width = GetScreenWidth() / 2;
    AIDetectRecUpPlayerQuadrant.height = GetScreenHeight() / 2;
    AIDetectRecUpPlayerQuadrant.x = 0;
    AIDetectRecUpPlayerQuadrant.y = 0;

    AIDetectRecDownPlayerQuadrant.width = GetScreenWidth() / 2;
    AIDetectRecDownPlayerQuadrant.height = GetScreenHeight() / 2;
    AIDetectRecDownPlayerQuadrant.x = 0;
    AIDetectRecDownPlayerQuadrant.y = GetScreenHeight() / 2;

    InitAudioDevice();

    load_sounds_array();

    SetTargetFPS(60);

    while(!WindowShouldClose()) {

        if (pause == 0) 
        {
            spawn_delay -= 1;
        }

        doPause();

        if (pause == 1 && IsKeyPressed(KEY_R)) 
        {
            ball.x = ball_position.x;
            ball.y = ball_position.y;

            paddle_left.y = paddle_left_position.y;
            paddle_right.y = paddle_right_position.y;

            paddle_left_score = 0;
            paddle_right_score = 0;

            pause = 0;

            spawn_delay = SPAWN_DELAY;
        }

        if (spawn_delay < 0) 
        {
            if (pause == 0) 
            {
                float ball_v_x = ball_v_x = ball.x + ball_speed.x * GetFrameTime();
                float ball_v_y = ball_v_y = ball.y + ball_speed.y * GetFrameTime();
        
                ball.x = ball_v_x;
        
                left_paddle_score(ball_v_x);
                right_paddle_score(ball_v_x);
        
                bounce_ball(ball_v_y, GetScreenHeight(), &ball, &ball_speed);
                
                if (paddle_right.y >= (GetScreenHeight() - PADDLE_HEIGHT)) 
                {
                    paddle_right.y = GetScreenHeight() - PADDLE_HEIGHT;
                }
                
                if (paddle_right.y <= 0) 
                {
                    paddle_right.y = 0;
                }
        
                if (paddle_left.y >= (GetScreenHeight() - PADDLE_HEIGHT)) 
                {
                    paddle_left.y = GetScreenHeight() - PADDLE_HEIGHT;
                }
        
                if (paddle_left.y <= 0) 
                {
                    paddle_left.y = 0;
                }

                if (IsKeyDown(KEY_W)) paddle_left.y -= (BALL_SPEED * 2) * GetFrameTime(); 
                if (IsKeyDown(KEY_S)) paddle_left.y += (BALL_SPEED * 2) * GetFrameTime();

                int paddle_left_collision = CheckCollisionRecs(ball, paddle_left);
                int paddle_right_collision = CheckCollisionRecs(ball, paddle_right);

                if (paddle_right_collision == 1) 
                {
                    ball_speed.x = -ball_v_x;
                    player_paddle_touched = 0;
                    PlaySound(soundArray[rand() % (2 - 1 + 1) + 1]); // melhorar isso
                }
        
                if (paddle_left_collision == 1)
                {
                    ball_speed.x = ball_v_x * 8;
                    player_paddle_touched = 1;
                    PlaySound(soundArray[rand() % (2 - 1 + 1) + 1]); // melhorar isso
                }

                if (game_mode_selected == AI) 
                {
                    if (ai_match_up_mode == LIMITED && paddle_right_score >= 3) 
                    {
                        match_ended = 1;
                        winner = &paddle_left;
                    }

                    if (ai_match_up_mode == LIMITED && paddle_left_score >= 3) 
                    {
                        match_ended = 1;
                        winner = &paddle_right;
                    }

                    int detect_collision_upaward_ai_quadrant = CheckCollisionRecs(ball, AIDetectRecUpAiQuadrant);
                    int detect_collision_downaward_ai_quadrant = CheckCollisionRecs(ball, AIDetectRecDownAiQuadrant);
                    int detect_collision_upaward_player_quadrant = CheckCollisionRecs(ball, AIDetectRecUpPlayerQuadrant);
                    int detect_collision_downaward_player_quadrant = CheckCollisionRecs(ball, AIDetectRecDownPlayerQuadrant);

                    if (detect_collision_upaward_ai_quadrant == 1) 
                    {
                        paddle_right.y -= BALL_SPEED * AI_QUADRANT_MULTPLIER * GetFrameTime();
                    }

                    if (detect_collision_downaward_ai_quadrant == 1) 
                    {
                        paddle_right.y += BALL_SPEED * AI_QUADRANT_MULTPLIER * GetFrameTime();
                    }

                    if (detect_collision_upaward_player_quadrant == 1) 
                    {
                        paddle_right.y -= BALL_SPEED * PLAYER_QUADRANT_MULTIPLIER * GetFrameTime();
                    }

                    if (detect_collision_downaward_player_quadrant == 1) 
                    {
                        paddle_right.y += BALL_SPEED * PLAYER_QUADRANT_MULTIPLIER * GetFrameTime();
                    }
                }
                else if (game_mode_selected == LOCAL) 
                {
                    if (IsKeyDown(KEY_KP_2)) paddle_right.y += (BALL_SPEED * 2) * GetFrameTime(); 
                    if (IsKeyDown(KEY_KP_8)) paddle_right.y -= (BALL_SPEED * 2) * GetFrameTime(); 
                }
                else if (game_mode_selected == ONLINE) 
                {

                }
            }
        }

        BeginDrawing();

            ClearBackground(BLACK);

            DrawRectangleRec(ball, WHITE);

            DrawRectangleRec(paddle_right, WHITE);
            DrawRectangleRec(paddle_left, WHITE);

            if (game_mode_selected == AI)
            {
                DrawRectangleRec(AIDetectRecUpAiQuadrant, BLANK);
                DrawRectangleRec(AIDetectRecDownAiQuadrant, BLANK);
                DrawRectangleRec(AIDetectRecUpPlayerQuadrant, BLANK);
                DrawRectangleRec(AIDetectRecDownPlayerQuadrant, BLANK);
            }

            DrawLine(GetScreenWidth() / 2, 0, GetScreenWidth() / 2, GetScreenHeight(), WHITE);
            
            DrawText(get_game_mode(game_mode_selected), 0 , 0, 32, WHITE);

            DrawText(get_ai_matchup(ai_match_up_mode), 0 , BALL_SIZE, 32, WHITE);

            show_score();

            if (match_ended == 1) 
            {
                force_stop();
                show_ended_match_menu(winner);
            }

            if (pause == 1)
            {
                show_menu();
            }

            if (spawn_delay >= 0 && pause == 0) {
                DrawText(TextFormat("%00i", spawn_delay / 60), GetScreenWidth() / 2, GetScreenHeight() / 2 + PADDING_TEXT * 2, 42, WHITE);
            }
            

        EndDrawing();
    }

    CloseWindow();

    return 0;
}