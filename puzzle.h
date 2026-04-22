#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ===== CONSTANTES ===== */
#define SEUIL_CIBLE     30
#define NB_PIECES       4
#define DUREE_CHRONO    10000
#define DUREE_SECOUSSE  500
#define PENALITE_TEMPS  2
#define ALERTE_SECONDS  3
#define LARGEUR_FENETRE 1200
#define HAUTEUR_FENETRE 800

/* ===== STRUCTURES ===== */

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     rect;
} Fond;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     rect;
    SDL_Rect     rect_originale;      
    int          est_glisse;
    int          decalage_glisse_x;
    int          decalage_glisse_y;
    int          visible;
    int          secoue;
    Uint32       debut_secousse;
} PiecePuzzle;

typedef struct {
    SDL_Texture *textures[11];
    SDL_Rect     rect;
    SDL_Rect     rect_originale;
    int          texture_actuelle;
    Uint32       temps_debut;
    Uint32       derniere_mise_a_jour;
    int          temps_ecoule;
    int          chrono_arrete;
    int          secondes_restantes;
    int          en_alerte;
    Uint32       dernier_clignotement;
} Chronometre;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     rect;
    int          visible;
    Uint32       temps_affichage;
    float        zoom;
    int          etat_zoom;
} Message;

typedef struct {
    Mix_Music *background_music;
    Mix_Chunk *success_sound;
    Mix_Chunk *fail_sound;
    Mix_Chunk *click_sound;
    Mix_Chunk *tick_sound;
} GameSounds;

typedef struct {
    int x;
    int y;
} PositionCible;

/* ===== PROTOTYPES ===== */

int  initialiserAudio(GameSounds *sounds);
void libererAudio(GameSounds *sounds);

void initialiserFond(Fond *fond, SDL_Renderer *renderer);

void initialiserPuzzle(PiecePuzzle pieces[], int numero_dossier, SDL_Renderer *renderer);
void libererPieces(PiecePuzzle pieces[], int nombre);
int  sourisSurPiece(PiecePuzzle *piece, int souris_x, int souris_y);
int  estSurPositionCible(PiecePuzzle *piece, int numero_dossier);
void ajusterPositionCible(PiecePuzzle *piece, int numero_dossier);
void demarrerSecousse(PiecePuzzle *piece);
void mettreAJourSecousse(PiecePuzzle *piece);

void initialiserChronometre(Chronometre *chrono, SDL_Renderer *renderer);
void mettreAJourChronometre(Chronometre *chrono, GameSounds *sounds);
void libererChronometre(Chronometre *chrono);

void initialiserMessage(Message *msg, TTF_Font *police, SDL_Color couleur,
                        const char *texte, SDL_Renderer *renderer);
void libererMessage(Message *msg);
void updateZoomMessage(Message *msg);

void appliquerEffetAlerte(SDL_Texture *texture, SDL_Renderer *renderer);
void renderTexture(SDL_Texture *texture, SDL_Renderer *renderer,
                   int x, int y, int w, int h);
void renderTextureScaled(SDL_Texture *texture, SDL_Renderer *renderer,
                         int x, int y, float scale);

PositionCible obtenirPositionCible(int numero_dossier);

#endif 
