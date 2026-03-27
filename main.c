#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "background.h"
#include <stdio.h>

// ============================================================
//  POINT D'ENTREE
// ============================================================

int main(void)
{
    // ---- Init SDL ----
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();

    SDL_Window   *win  = SDL_CreateWindow("Jeu SDL2",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            800, 600, 0);
    SDL_Renderer *ren  = SDL_CreateRenderer(win, -1,
                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Chargement de la police (essai plusieurs chemins)
    TTF_Font *font = TTF_OpenFont("arial.ttf", 18);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 18);
    if (!font)
    {
        printf("Impossible de charger la police: %s\n", TTF_GetError());
        return 1;
    }

    // ---- Background ----
    Background bg;
    initBackground(&bg, ren, "background.png");
    initTemps(&bg);

    // ---- Niveau actuel ----
    Niveau niveauActuel = LEVEL1;

    // ---- Plateformes ----
    #define NB_PLAT 6
    Plateforme p[NB_PLAT];
    initPlateformes(p, NB_PLAT, niveauActuel);

    // ---- Guide ----
    SDL_Texture *guide = NULL;
    initGuide(&guide, ren, font);

    // ---- Variables de jeu ----
    int  running   = 1;
    int  mode      = 0;   // 0 = mono, 1 = split-screen
    int  showGuide = 0;
    int  score     = 0;
    char nomJoueur[50] = "";

    SDL_Event e;

    // ============================================================
    //  BOUCLE PRINCIPALE
    // ============================================================
    while (running)
    {
        // ---- Evenements ----
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    // Scrolling camera P1
                    case SDLK_RIGHT: scrolling(&bg, 0); break;
                    case SDLK_LEFT:  scrolling(&bg, 1); break;
                    case SDLK_DOWN:  scrolling(&bg, 2); break;
                    case SDLK_UP:    scrolling(&bg, 3); break;

                    // Scrolling camera P2 (split-screen)
                    case SDLK_d: bg.camera2.x += 10; break;
                    case SDLK_q: bg.camera2.x -= 10; break;
                    case SDLK_s: bg.camera2.y += 10; break;
                    case SDLK_z: bg.camera2.y -= 10; break;

                    // Basculer mono / split-screen
                    case SDLK_m: mode = !mode; break;

                    // Afficher / cacher le guide
                    case SDLK_h: showGuide = !showGuide; break;

                    // Chargement de niveau
                    case SDLK_1:
                        niveauActuel = LEVEL1;
                        initPlateformes(p, NB_PLAT, niveauActuel);
                        initTemps(&bg);
                        printf("Level 1 charge\n");
                        break;
                    case SDLK_2:
                        niveauActuel = LEVEL2;
                        initPlateformes(p, NB_PLAT, niveauActuel);
                        initTemps(&bg);
                        printf("Level 2 charge\n");
                        break;

                    // Frapper la premiere plateforme destructible
                    case SDLK_SPACE:
                        for (int i = 0; i < NB_PLAT; i++)
                            if (p[i].type == 2 && p[i].active)
                            {
                                detruirePlateforme(p, i);
                                score += 10;
                                break;
                            }
                        break;

                    // Sous-menu meilleurs scores
                    case SDLK_F2:
                        afficherScores(ren, font);
                        break;

                    // Quitter
                    case SDLK_ESCAPE:
                        running = 0;
                        break;

                    default: break;
                }
            }
        }

        // ---- Mise a jour ----
        updatePlateformes(p, NB_PLAT);

        // ---- Rendu ----
        SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
        SDL_RenderClear(ren);

        // Background
        afficherBackground(bg, ren, mode);

        // Plateformes
        afficherPlateformes(p, NB_PLAT, ren);

        // Temps (coin haut gauche)
        afficherTemps(&bg, ren, font);

        // ---- Score (coin haut droit) ----
        {
            char scoreStr[32];
            sprintf(scoreStr, "Score: %d", score);
            SDL_Color c = {255, 255, 255, 255};
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

        // ---- Indicateur niveau (haut centre) ----
        {
            char lvlStr[20];
            sprintf(lvlStr, "Level %d", (int)niveauActuel);
            SDL_Color c = {100, 220, 255, 255};
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

        // ---- Indicateur mode (bas gauche) ----
        {
            const char *modeStr = (mode == 0) ? "Mode: Mono [M]" : "Mode: Split [M]";
            SDL_Color c = {180, 180, 180, 255};
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

        // ---- Rappel touche guide (bas droite) ----
        {
            const char *hint = showGuide ? "[H] Fermer guide" : "[H] Aide";
            SDL_Color c = {120, 120, 120, 255};
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

        // ---- Guide (si actif) ----
        if (showGuide)
            afficherGuide(guide, ren, font);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    // ============================================================
    //  FIN DE JEU : saisie du nom et sauvegarde
    // ============================================================
    saisirNom(nomJoueur, 50, ren, font);
    saveScore(nomJoueur, score);

    // Classement final
    afficherScores(ren, font);

    // ---- Liberation ----
    libererGuide(&guide);
    libererBackground(&bg);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
