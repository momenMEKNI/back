#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NB_PIECES 4
#define LARGEUR_FENETRE 1200
#define HAUTEUR_FENETRE 800
#define SEUIL_CIBLE 30
#define PENALITE_TEMPS 2

typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
} Fond;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Rect original;
    int visible;
    int secoue;
    Uint32 debut;
} Piece;

typedef struct {
    SDL_Texture *textures[11];
    SDL_Rect rect;
    SDL_Rect original;
    int index;
    int secondes;
    int fini;
    int alerte;
    Uint32 dernier;
} Chrono;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
    float zoom;
    int visible;
} Message;

typedef struct {
    Fond fond;
    Piece pieces[NB_PIECES];
    Chrono chrono;
    Message victoire;
    Message erreur;
    Message temps;
    int dossier;
    int fini;
    Piece *active;
    Uint32 tempsErreur;
} Jeu;

void initJeu(Jeu *jeu, SDL_Renderer *renderer, TTF_Font *font);
void updateJeu(Jeu *jeu);
void renderJeu(Jeu *jeu, SDL_Renderer *renderer);
void handleEvents(Jeu *jeu, SDL_Event *e);
void cleanJeu(Jeu *jeu);
int positionOK(Piece *p, int dossier);

#endif
