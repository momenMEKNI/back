#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "background.h"
#include <stdio.h>

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();

    SDL_Window   *win = SDL_CreateWindow("Jeu SDL2",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            800, 600, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Font *font = TTF_OpenFont("arial.ttf", 18);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 18);
    if (!font)
    {
        printf("Impossible de charger la police: %s\n", TTF_GetError());
        return 1;
    }

    GameState gs;
    int niveauActuel = LEVEL1;

    initBackground(&gs, ren, "background.png", niveauActuel);
    initGuide(&gs, ren, font);

    int  running   = 1;
    int  mode      = 0;
    int  showGuide = 0;
    int  score     = 0;
    char nomJoueur[50] = "";

    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_RIGHT: scrolling(&gs, 0); break;
                    case SDLK_LEFT:  scrolling(&gs, 1); break;
                    case SDLK_DOWN:  scrolling(&gs, 2); break;
                    case SDLK_UP:    scrolling(&gs, 3); break;

                    case SDLK_d: gs.camera2.x += 10; break;
                    case SDLK_q: gs.camera2.x -= 10; break;
                    case SDLK_s: gs.camera2.y += 10; break;
                    case SDLK_z: gs.camera2.y -= 10; break;

                    case SDLK_m: mode = !mode; break;

                    case SDLK_h: showGuide = !showGuide; break;

                    case SDLK_1:
                        niveauActuel = LEVEL1;
                        initBackground(&gs, ren, "background.png", niveauActuel);
                        break;
                    case SDLK_2:
                        niveauActuel = LEVEL2;
                        initBackground(&gs, ren, "background.png", niveauActuel);
                        break;

                    case SDLK_F2:
                        gererScores(&gs, ren, font, nomJoueur, score, 1);
                        break;

                    case SDLK_ESCAPE:
                        running = 0;
                        break;

                    default: break;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
        SDL_RenderClear(ren);

        afficherBackground(&gs, ren, mode);
        afficherTemps(&gs, ren, font);

        {
            char scoreStr[32];
            SDL_Color c = {255, 255, 255, 255};
            sprintf(scoreStr, "Score: %d", score);
            SDL_Surface *sf = TTF_RenderUTF8_Blended(font, scoreStr, c);
            if (sf)
            {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, sf);
                SDL_Rect pos = {800 - sf->w - 10, 10, sf->w, sf->h};
                SDL_RenderCopy(ren, tex, NULL, &pos);
                SDL_FreeSurface(sf);
                SDL_DestroyTexture(tex);
            }
        }

        {
            char lvlStr[20];
            SDL_Color c = {100, 220, 255, 255};
            sprintf(lvlStr, "Level %d", niveauActuel);
            SDL_Surface *sf = TTF_RenderUTF8_Blended(font, lvlStr, c);
            if (sf)
            {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, sf);
                SDL_Rect pos = {400 - sf->w/2, 10, sf->w, sf->h};
                SDL_RenderCopy(ren, tex, NULL, &pos);
                SDL_FreeSurface(sf);
                SDL_DestroyTexture(tex);
            }
        }

        {
            SDL_Color c = {180, 180, 180, 255};
            const char *modeStr = (mode == 0) ? "Mode: Mono [M]" : "Mode: Split [M]";
            SDL_Surface *sf = TTF_RenderUTF8_Blended(font, modeStr, c);
            if (sf)
            {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, sf);
                SDL_Rect pos = {10, 575, sf->w, sf->h};
                SDL_RenderCopy(ren, tex, NULL, &pos);
                SDL_FreeSurface(sf);
                SDL_DestroyTexture(tex);
            }
        }

        {
            SDL_Color c = {120, 120, 120, 255};
            const char *hint = showGuide ? "[H] Fermer guide" : "[H] Aide";
            SDL_Surface *sf = TTF_RenderUTF8_Blended(font, hint, c);
            if (sf)
            {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, sf);
                SDL_Rect pos = {800 - sf->w - 10, 575, sf->w, sf->h};
                SDL_RenderCopy(ren, tex, NULL, &pos);
                SDL_FreeSurface(sf);
                SDL_DestroyTexture(tex);
            }
        }

        if (showGuide)
            gererScores(&gs, ren, font, nomJoueur, score, 3);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    gererScores(&gs, ren, font, nomJoueur, score, 0);
    gererScores(&gs, ren, font, nomJoueur, score, 1);
    gererScores(&gs, ren, font, nomJoueur, score, 2);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
