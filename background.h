#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// -------- BACKGROUND --------
typedef struct {
    SDL_Texture *texture;
    SDL_Rect     pos;
    SDL_Rect     camera;
    SDL_Rect     camera2;   // deuxieme camera pour split-screen
    Uint32       startTime;
} Background;

// -------- PLATEFORMES --------
typedef struct {
    SDL_Rect pos;
    int      type;       // 0 fixe, 1 mobile, 2 destructible
    int      active;
    int      direction;
    int      hits;       // nombre de coups pour destructible
} Plateforme;

// -------- SCORE --------
typedef struct {
    char nom[50];
    int  score;
} Score;

// -------- NIVEAU --------
typedef enum {
    LEVEL1 = 1,
    LEVEL2 = 2
} Niveau;

// -------- BACKGROUND --------
void initBackground(Background *bg, SDL_Renderer *renderer, const char *path);
void afficherBackground(Background bg, SDL_Renderer *renderer, int mode);
void scrolling(Background *bg, int direction);
void libererBackground(Background *bg);

// -------- TEMPS --------
void initTemps(Background *bg);
void afficherTemps(Background *bg, SDL_Renderer *renderer, TTF_Font *font);

// -------- PLATEFORMES --------
void initPlateformes(Plateforme p[], int n, Niveau niveau);
void afficherPlateformes(Plateforme p[], int n, SDL_Renderer *renderer);
void updatePlateformes(Plateforme p[], int n);
void detruirePlateforme(Plateforme p[], int index);

// -------- GUIDE --------
void initGuide(SDL_Texture **guide, SDL_Renderer *renderer, TTF_Font *font);
void afficherGuide(SDL_Texture *guide, SDL_Renderer *renderer, TTF_Font *font);
void libererGuide(SDL_Texture **guide);

// -------- SCORES --------
void saveScore(char nom[], int score);
void afficherScores(SDL_Renderer *renderer, TTF_Font *font);
int  chargerScores(Score scores[], int max);
void saisirNom(char nom[], int maxLen, SDL_Renderer *renderer, TTF_Font *font);

#endif
