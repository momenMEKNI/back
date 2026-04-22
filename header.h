#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ══════════════════════════════════════════
   CONSTANTES
══════════════════════════════════════════ */
#define SCREEN_W         800
#define SCREEN_H         500
#define FPS              60
#define FRAME_DELAY      (1000 / FPS)

#define JOUEUR_W         40
#define JOUEUR_H         60
#define JOUEUR_VITESSE   3
#define JOUEUR_HP_MAX    100

#define ENNEMI_W         38
#define ENNEMI_H         55
#define ENNEMI_HP_MAX    60
#define ENNEMI_DEGATS    10
#define ATTAQUE_COOLDOWN 800
#define ATTAQUE_PORTEE   60
#define AGGRO_RANGE      160

#define MAX_ENNEMIS      5
#define MAX_ES           8

#define ES_W             22
#define ES_H             22
#define ES_SCORE_VAL     50
#define ES_VIE_VAL       15

#define SCORES_FILE      "scores.txt"

/* ══════════════════════════════════════════
   SPRITESHEET JOUEUR
   542x542 — 4 lignes x 8 colonnes
   Chaque frame : 67 x 135 px (largeur x hauteur)
══════════════════════════════════════════ */
#define SPR_FRAME_W      67
#define SPR_FRAME_H      135
#define SPR_COLS         8
#define SPR_ROW_IDLE     0   /* ligne 0 : 8 frames idle        */
#define SPR_ROW_RUN      1   /* ligne 1 : 8 frames course      */
#define SPR_ROW_ATTACK   2   /* ligne 2 : 8 frames attaque     */
#define SPR_ROW_DEATH    3   /* ligne 3 : 5 frames mort        */
#define SPR_FRAMES_IDLE  8
#define SPR_FRAMES_RUN   8
#define SPR_FRAMES_ATK   8
#define SPR_FRAMES_DEATH 5

/* Texture globale du spritesheet (initialisée dans init_sdl) */
extern SDL_Texture *g_joueur_sprite;

/* ══════════════════════════════════════════
   ÉNUMÉRATIONS
══════════════════════════════════════════ */
typedef enum { ETAT_MENU, ETAT_NOM, ETAT_JEU, ETAT_PAUSE, ETAT_GAME_OVER, ETAT_WIN } EtatJeu;
typedef enum { LEVEL_1 = 1, LEVEL_2 = 2 } Level;
typedef enum { DIR_HAUT, DIR_BAS, DIR_GAUCHE, DIR_DROITE } Direction;
typedef enum { SANTE_VIVANT, SANTE_BLESSE, SANTE_CRITIQUE } EtatSante;

/* ══════════════════════════════════════════
   STRUCTURES
══════════════════════════════════════════ */
typedef struct {
    int x, y, w, h;
    int hp, hp_max;
    int score;
    Direction dir;
    int en_mouvement;
    int en_attaque;
    int est_mort;       /* 1 = animation mort en cours */
    Uint32 attaque_timer;
    Uint32 anim_timer;
    int anim_frame;
    int anim_row;       /* ligne courante du spritesheet */
} Joueur;

typedef struct {
    int x, y, w, h;
    int hp, hp_max;
    int actif;
    Direction dir;
    int vitesse;
    int en_aggro;
    int dest_x, dest_y;
    int pt_x[2], pt_y[2];
    int phase;
    Uint32 attaque_timer;
    Uint32 anim_timer;
    int anim_frame;
} Ennemi;

typedef struct {
    int x, y, w, h;
    int actif;
    int est_vie;
} ES;

typedef struct {
    int score_total;
    int level_actuel;
    int ennemis_restants;
} ScoreInfo;

/* ══════════════════════════════════════════
   PROTOTYPES — source.c
══════════════════════════════════════════ */

/* Init SDL */
int  init_sdl(SDL_Window **win, SDL_Renderer **ren);
void quit_sdl(SDL_Window *win, SDL_Renderer *ren);

/* Dessin utilitaires */
void draw_filled(SDL_Renderer *r, int x, int y, int w, int h, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
void draw_border(SDL_Renderer *r, int x, int y, int w, int h, Uint8 R, Uint8 G, Uint8 B);
void draw_text  (SDL_Renderer *r, TTF_Font *f, const char *t, int x, int y, SDL_Color c);
void draw_bg    (SDL_Renderer *r, SDL_Texture *bg);

/* Joueur */
void init_joueur   (Joueur *j);
void update_joueur (Joueur *j, const Uint8 *keys);
void joueur_attaque(Joueur *j, Ennemi ennemis[], int n);
void draw_joueur   (SDL_Renderer *r, Joueur *j);

/* Ennemi */
void init_ennemis  (Ennemi e[], int n, Level lv);
void update_ennemis(Ennemi e[], int n, Joueur *j, Level lv);
void draw_ennemis  (SDL_Renderer *r, Ennemi e[], int n);

/* ES */
void init_es(ES es[], int n);
void draw_es(SDL_Renderer *r, ES es[], int n);

/* Collision — 1 seule fonction appelée 2 fois */
int  rect_col(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
void gerer_collisions(Joueur *j, Ennemi e[], int ne, ES es[], int nes, ScoreInfo *sc);

/* Santé */
EtatSante get_etat_sante (int hp, int hp_max);
void draw_barre_sante    (SDL_Renderer *r, int x, int y, int w, int h, int hp, int hp_max);
void draw_hud            (SDL_Renderer *r, TTF_Font *f, Joueur *j, ScoreInfo *sc);

/* Scores */
void sauvegarder_score  (const char *nom, int score);
void afficher_top_scores(SDL_Renderer *r, TTF_Font *f, int x, int y);

/* Niveau */
void init_niveau   (Joueur *j, Ennemi e[], ES es[], ScoreInfo *sc, Level lv);
int  niveau_termine(Ennemi e[], int n);

/* Bouton image */
void draw_btn(SDL_Renderer *r, SDL_Texture *btn_tex, TTF_Font *f,
              const char *label, int x, int y, int w, int h, int selected);

/* Menus */
void draw_menu      (SDL_Renderer *r, TTF_Font *f, TTF_Font *fs, SDL_Texture *bg, SDL_Texture *btn, int sel);
void draw_saisie_nom(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg, SDL_Texture *btn, const char *nom, Level lv);
void draw_pause     (SDL_Renderer *r, TTF_Font *f, SDL_Texture *btn);
void draw_game_over (SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg, SDL_Texture *btn, ScoreInfo *sc);
void draw_win       (SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg, SDL_Texture *btn, ScoreInfo *sc);

#endif /* HEADER_H */
