#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "joueur.h"
#include "minimap.h"
#include "background.h"

#define WIN_W 800
#define WIN_H 600
#define GROUND_LEVEL 500

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        SDL_Quit(); return 1;
    }
    if (TTF_Init() != 0) {
        printf("TTF_Init error: %s\n", TTF_GetError());
        IMG_Quit(); SDL_Quit(); return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Jeu - Level 1",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf", 14);
    if (!font) font = TTF_OpenFont("arial.ttf", 14);

    SDL_Surface *surf = IMG_Load("images/BG_LEVEL_1_VECTOR_ART.png");
    SDL_Texture *gameBackground = NULL;
    int bgWidth = WIN_W, bgHeight = WIN_H;
    if (surf) {
        bgWidth = surf->w; bgHeight = surf->h;
        gameBackground = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    Joueur player1, player2;
    initJoueur(&player1, renderer, 1, 150, GROUND_LEVEL);
    initJoueur(&player2, renderer, 2, 600, GROUND_LEVEL);

    MiniMap minimap;
    initMiniMap(&minimap, renderer, gameBackground, bgWidth, bgHeight, 1);

    SDL_Event event;
    int running = 1;
    int isMultiplayer = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = 0;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) isMultiplayer = !isMultiplayer;
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        deplacerJoueur(&player1, event, keys, 0.016f);
        if (isMultiplayer) deplacerJoueur(&player2, event, keys, 0.016f);

        drawBackground(renderer, gameBackground);

        afficherJoueur(player1, renderer);
        if (isMultiplayer) afficherJoueur(player2, renderer);

        afficherMiniMap(&minimap, renderer, font,
                        &player1, player1.pos.x,
                        &player2, player2.pos.x,
                        isMultiplayer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    freeMiniMap(&minimap);
    freeJoueur(&player1);
    freeJoueur(&player2);
    if (gameBackground) SDL_DestroyTexture(gameBackground);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}

