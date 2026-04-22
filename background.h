#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define LEVEL1 1
#define LEVEL2 2

typedef struct {

    SDL_Texture *texture;
    SDL_Rect     pos;
    SDL_Rect     camera;
    SDL_Rect     camera2;
    Uint32       startTime;

    struct {
        char nom[50];
        int  score;
    } scores[100];
    int nbScores;

    SDL_Texture *guide;

} GameState;

void initBackground(GameState *gs, SDL_Renderer *renderer, const char *path, int niveau);
void afficherBackground(GameState *gs, SDL_Renderer *renderer, int mode);
void scrolling(GameState *gs, int direction);
void afficherTemps(GameState *gs, SDL_Renderer *renderer, TTF_Font *font);
void initGuide(GameState *gs, SDL_Renderer *renderer, TTF_Font *font);
void gererScores(GameState *gs, SDL_Renderer *renderer, TTF_Font *font, char nom[], int score, int action);

#endif
