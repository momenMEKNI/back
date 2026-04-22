#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Texture *bg;
    SDL_Texture *play;
    SDL_Texture *quit;
    SDL_Rect posPlay;
    SDL_Rect posQuit;
} Menu;

void initMenu(Menu *m, SDL_Renderer *renderer);
void afficherMenu(Menu m, SDL_Renderer *renderer);
int handleMenu(Menu m, SDL_Event event);

#endif
