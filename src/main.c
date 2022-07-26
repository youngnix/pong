#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define Vector2(T) struct{ T x; T y; }

#define clamp(value, min, max) (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)));

typedef struct RayCast {
    SDL_Point initial_point;
    SDL_Point final_point;
    SDL_Point radians;
    float length;
} RayCast;

unsigned char checkColision(SDL_Rect *rect_a, SDL_Rect *rect_b); 

int main(int argc, char* argv[]){
    unsigned int window_width = 512;
    unsigned int window_height = 256;

    unsigned char keys_pressed[512];
    unsigned char game_started = 0;
    unsigned char run_game = 1;

    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("Couldn't start SDL. SDL error: %s\n", SDL_GetError());
    }

    if(!IMG_Init(IMG_INIT_PNG)){
        printf("Couldn't start IMG. SDL error: %s\n", SDL_GetError());
    }

    if(TTF_Init() != 0){
        printf("Couldn't start TTF. SDL error: %s\n", SDL_GetError());
    }

    SDL_Window *window = SDL_CreateWindow("pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture *ball_texture = IMG_LoadTexture(renderer, "res/ball.png");
    SDL_Texture *paddle_texture = IMG_LoadTexture(renderer, "res/paddle.png");

    SDL_Rect ball = { 0, 0, 8, 8 };
    ball.x = (window_width / 2) - (ball.w / 2);
    ball.y = (window_height / 2) - (ball.h / 2);
    Vector2(char) ball_direction = { -1, -1 };

    SDL_Rect player = { 0, 0, 4, 32 };
    player.x = player.h;
    player.y = (window_height / 2) - (player.h / 2);
    Vector2(char) player_direction = { 0, 0 };

    SDL_Rect bot = { 0, 0, 4, 32 };
    bot.x = window_width - bot.h;
    bot.y = (window_height / 2) - (bot.h / 2);
    Vector2(char) bot_direction = { 0, 0 };
    SDL_Rect bot_area = { window_width / 2, 0, window_width / 2, window_height };

    TTF_Font* score_font = TTF_OpenFont("res/munro.ttf", 10);

    SDL_Color white = { 255, 255, 255, 255 };

    unsigned int player_score = 0;
    unsigned int bot_score = 0;

    char *player_score_text = calloc(4, sizeof(unsigned char));
    char *bot_score_text = calloc(4, sizeof(unsigned char));

    sprintf(player_score_text, "%i", player_score);
    sprintf(bot_score_text, "%i", bot_score);

    SDL_Surface* player_score_surface = TTF_RenderText_Solid(score_font, player_score_text, white);
    SDL_Surface* bot_score_surface = TTF_RenderText_Solid(score_font, bot_score_text, white);

    SDL_Texture* player_score_texture = SDL_CreateTextureFromSurface(renderer, player_score_surface);
    SDL_Texture* bot_score_texture = SDL_CreateTextureFromSurface(renderer, bot_score_surface);

    SDL_Rect player_score_rect = { 0, 0, 32, 32 };
    player_score_rect.x = (window_width / 2) - player_score_rect.w;
    SDL_Rect bot_score_rect = { 0, 0, 32, 32 };
    bot_score_rect.x = (window_width / 2);

    RayCast ball_raycast;
    ball_raycast.initial_point = (SDL_Point){ ball.x, ball.y };
    ball_raycast.radians = (SDL_Point){ ball_direction.x, ball_direction.y };
    ball_raycast.length = 96;

    while(run_game){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_KEYDOWN:
                    if(!event.key.repeat) keys_pressed[event.key.keysym.scancode] = 1;
                    break;
                case SDL_KEYUP:
                    keys_pressed[event.key.keysym.scancode] = 0;
                    break;
                case SDL_QUIT:
                    run_game = 0;
                    break;
            }
        }

        if(keys_pressed[SDL_SCANCODE_SPACE] && !game_started){
            game_started = !game_started;
        }

        if(game_started){
            player_direction.y = -(unsigned char)keys_pressed[SDL_SCANCODE_W] - -(unsigned char)keys_pressed[SDL_SCANCODE_S];

            ball_raycast.initial_point.x = ball.x + ball.h / 2;
            ball_raycast.initial_point.y = ball.y + ball.h / 2;
            ball_raycast.final_point.x = ball_raycast.initial_point.x;
            ball_raycast.final_point.y = ball_raycast.initial_point.y;
            ball_raycast.radians = (SDL_Point){ ball_direction.x, ball_direction.y };

            while((ball_raycast.final_point.x != ball_raycast.initial_point.x + ball_raycast.radians.x * ball_raycast.length)
            && (ball_raycast.final_point.y != ball_raycast.initial_point.y + ball_raycast.radians.y * ball_raycast.length)){
                ball_raycast.final_point.x += ball_raycast.radians.x;
                ball_raycast.final_point.y += ball_raycast.radians.y;
                if(SDL_PointInRect(&ball_raycast.final_point, &bot)){
                    break;
                }
            }

            if(checkColision(&ball, &bot_area) && ball_direction.x == 1){
                if(!SDL_PointInRect(&ball_raycast.final_point, &bot)){
                    if(ball_raycast.final_point.y > bot.y) bot_direction.y = 1;
                    if(ball_raycast.final_point.y < bot.y + bot.h) bot_direction.y = -1;

                    if(ball_raycast.final_point.x > bot.x + bot.w){
                        if(ball.y > bot.y + bot.h / 2) bot_direction.y = 1;
                        if(ball.y < bot.y + bot.h / 2) bot_direction.y = -1;
                    }
                } else {
                    bot_direction.y = 0;
                }
            } else {
                bot_direction.y = 0;
            }

            if(checkColision(&ball, &player) || checkColision(&ball, &bot)){
                ball_direction.x = -ball_direction.x;
            }

            if((ball.y >= 256 - ball.h) || (ball.y <= 0)){
                ball_direction.y = -ball_direction.y;
            }

            player.y += player_direction.y * 4;
            bot.y += bot_direction.y * 4;

            player.y = clamp(player.y, 0, window_height - player.h);
            bot.y = clamp(bot.y, 0, window_height - bot.h);

            ball.x += ball_direction.x * 4;
            ball.y += ball_direction.y * 4;

            ball.y = clamp(ball.y, 0, 256);

            if((ball.x - ball.w < 0) || ball.x > window_width){
                if(ball.x > window_width / 2) player_score++;
                if(ball.x < window_width / 2) bot_score++;

                sprintf(player_score_text, "%i", player_score);
                sprintf(bot_score_text, "%i", bot_score);

                player_score_surface = TTF_RenderText_Solid(score_font, player_score_text, white);
                bot_score_surface = TTF_RenderText_Solid(score_font, bot_score_text, white);

                player_score_texture = SDL_CreateTextureFromSurface(renderer, player_score_surface);
                bot_score_texture = SDL_CreateTextureFromSurface(renderer, bot_score_surface);

                ball.x = (window_width / 2) - (ball.w / 2);
                ball.y = (window_height / 2) - (ball.h / 2);
                player.x = player.h;
                player.y = (window_height / 2) - (player.h / 2);
                bot.x = window_width - bot.h;
                bot.y = (window_height / 2) - (bot.h / 2);
                game_started = !game_started;
            }
        }

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, player_score_texture, NULL, &player_score_rect);
        SDL_RenderCopy(renderer, bot_score_texture, NULL, &bot_score_rect);

        SDL_RenderCopy(renderer, paddle_texture, NULL, &player);
        SDL_RenderCopy(renderer, ball_texture, NULL, &ball);
        SDL_RenderCopy(renderer, paddle_texture, NULL, &bot);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderDrawLine(renderer, (window_width / 2) - 1, 0, (window_width / 2) - 1, window_height);
        SDL_RenderDrawLine(renderer, window_width / 2, 0, window_width / 2, window_height);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_RenderPresent(renderer);
    }

    free(player_score_text);
    free(bot_score_text);

    SDL_FreeSurface(bot_score_surface);
    SDL_FreeSurface(player_score_surface);

    SDL_DestroyTexture(bot_score_texture);
    SDL_DestroyTexture(player_score_texture);
    SDL_DestroyTexture(ball_texture);
    SDL_DestroyTexture(paddle_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    IMG_Quit();
    TTF_Quit();

    return 0;
}

unsigned char checkColision(SDL_Rect *rect_a, SDL_Rect *rect_b){
    if(rect_a->x < rect_b->x + rect_b->w
        && rect_a->x + rect_a->w > rect_b->x
        && rect_a->y < rect_b->y + rect_b->h
        && rect_a->y + rect_a->h > rect_b->y) return 1;
    return 0;
}
