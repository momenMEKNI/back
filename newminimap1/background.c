#include "background.h"

void drawBackground(SDL_Renderer *renderer, SDL_Texture *bg) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);
    if (bg) {
        SDL_Rect dst = {0,0,800,600};
        SDL_RenderCopy(renderer, bg, NULL, &dst);
    }
}

