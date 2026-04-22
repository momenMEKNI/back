/*
 * source.c — Toutes les fonctions du jeu INCEPTION (Saito)
 *
 * Contient :
 *   - init SDL / quit
 *   - utilitaires dessin
 *   - joueur (init, update, attaque, draw)
 *   - ennemi (init, update, draw)  — trajectoire aleatoire L1 / patrouille 2pts L2
 *   - ES collectibles (init, draw)
 *   - collision BB — rect_col() appelee 2 fois dans gerer_collisions()
 *   - sante (etats vivant/blesse/critique, barres, HUD)
 *   - scores (sauvegarde fichier, affichage top)
 *   - niveau (init L1/L2, detection fin)
 *   - menus (menu, saisie nom, pause, game over, victoire)
 */

#include "header.h"

/* Texture globale du spritesheet joueur */
SDL_Texture *g_joueur_sprite = NULL;

/* ═══════════════════════════════════════════════════════════════
   INIT SDL
═══════════════════════════════════════════════════════════════ */
int init_sdl(SDL_Window **win, SDL_Renderer **ren) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError()); SDL_Quit(); return 0;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError()); IMG_Quit(); SDL_Quit(); return 0;
    }
    *win = SDL_CreateWindow("INCEPTION — Saito",
               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!*win) { fprintf(stderr, "Window: %s\n", SDL_GetError()); return 0; }

    *ren = SDL_CreateRenderer(*win, -1,
               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*ren) { fprintf(stderr, "Renderer: %s\n", SDL_GetError()); return 0; }

    SDL_SetRenderDrawBlendMode(*ren, SDL_BLENDMODE_BLEND);
    srand((unsigned)time(NULL));

    /* Charger le spritesheet joueur (version fond transparent) */
    g_joueur_sprite = IMG_LoadTexture(*ren, "assets/spri_transparent.png");
    if (!g_joueur_sprite)
        fprintf(stderr, "Spritesheet joueur: %s\n", IMG_GetError());

    return 1;
}

void quit_sdl(SDL_Window *win, SDL_Renderer *ren) {
    if (g_joueur_sprite) { SDL_DestroyTexture(g_joueur_sprite); g_joueur_sprite = NULL; }
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
}

/* ═══════════════════════════════════════════════════════════════
   UTILITAIRES DESSIN
═══════════════════════════════════════════════════════════════ */
void draw_filled(SDL_Renderer *r, int x, int y, int w, int h,
                 Uint8 R, Uint8 G, Uint8 B, Uint8 A) {
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderFillRect(r, &rc);
}

void draw_border(SDL_Renderer *r, int x, int y, int w, int h,
                 Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderDrawRect(r, &rc);
}

void draw_text(SDL_Renderer *r, TTF_Font *f, const char *t,
               int x, int y, SDL_Color c) {
    if (!f || !t || !*t) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, t, c);
    if (!s) return;
    SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
    if (tx) {
        SDL_Rect dst = {x, y, s->w, s->h};
        SDL_RenderCopy(r, tx, NULL, &dst);
        SDL_DestroyTexture(tx);
    }
    SDL_FreeSurface(s);
}

void draw_bg(SDL_Renderer *r, SDL_Texture *bg) {
    if (!bg) {
        SDL_SetRenderDrawColor(r, 10, 10, 20, 255);
        SDL_RenderClear(r);
        return;
    }
    SDL_Rect dst = {0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderCopy(r, bg, NULL, &dst);
}

/* ═══════════════════════════════════════════════════════════════
   JOUEUR
═══════════════════════════════════════════════════════════════ */
void init_joueur(Joueur *j) {
    j->x = SCREEN_W / 2 - JOUEUR_W / 2;
    j->y = SCREEN_H / 2 - JOUEUR_H / 2;
    j->w = JOUEUR_W;    j->h = JOUEUR_H;
    j->hp = JOUEUR_HP_MAX;  j->hp_max = JOUEUR_HP_MAX;
    j->score = 0;
    j->dir = DIR_BAS;
    j->en_mouvement = 0;
    j->en_attaque   = 0;
    j->est_mort     = 0;
    j->attaque_timer = 0;
    j->anim_timer   = 0;
    j->anim_frame   = 0;
    j->anim_row     = SPR_ROW_IDLE;
}

void update_joueur(Joueur *j, const Uint8 *keys) {
    /* Si mort : jouer animation mort une fois et bloquer */
    if (j->est_mort) {
        if (SDL_GetTicks() - j->anim_timer > 120) {
            if (j->anim_frame < SPR_FRAMES_DEATH - 1)
                j->anim_frame++;
            j->anim_timer = SDL_GetTicks();
        }
        j->anim_row = SPR_ROW_DEATH;
        return;
    }

    int dx = 0, dy = 0;
    if (keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) { dy = -JOUEUR_VITESSE; j->dir = DIR_HAUT;   }
    if (keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) { dy =  JOUEUR_VITESSE; j->dir = DIR_BAS;    }
    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) { dx = -JOUEUR_VITESSE; j->dir = DIR_GAUCHE; }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) { dx =  JOUEUR_VITESSE; j->dir = DIR_DROITE; }

    j->x += dx;  j->y += dy;
    j->en_mouvement = (dx || dy);

    /* Limites ecran */
    if (j->x < 0)               j->x = 0;
    if (j->y < 42)               j->y = 42;
    if (j->x + j->w > SCREEN_W) j->x = SCREEN_W - j->w;
    if (j->y + j->h > SCREEN_H) j->y = SCREEN_H - j->h;

    /* Fin animation attaque apres 300ms */
    if (j->en_attaque && SDL_GetTicks() - j->attaque_timer > 300)
        j->en_attaque = 0;

    /* Choisir la ligne d'animation */
    if (j->en_attaque)       j->anim_row = SPR_ROW_ATTACK;
    else if (j->en_mouvement) j->anim_row = SPR_ROW_RUN;
    else                      j->anim_row = SPR_ROW_IDLE;

    /* Vitesse animation */
    int spd = (j->en_attaque) ? 80 : (j->en_mouvement ? 100 : 160);
    if (SDL_GetTicks() - j->anim_timer > (Uint32)spd) {
        int max_frames = (j->anim_row == SPR_ROW_ATTACK) ? SPR_FRAMES_ATK :
                         (j->anim_row == SPR_ROW_RUN)    ? SPR_FRAMES_RUN :
                                                            SPR_FRAMES_IDLE;
        j->anim_frame = (j->anim_frame + 1) % max_frames;
        j->anim_timer = SDL_GetTicks();
    }
}

/* Attaque du joueur — ESPACE — zone dans la direction du regard */
void joueur_attaque(Joueur *j, Ennemi ennemis[], int n) {
    Uint32 now = SDL_GetTicks();
    if (now - j->attaque_timer < 500) return;   /* cooldown 500ms */
    j->attaque_timer = now;
    j->en_attaque = 1;

    /* Zone d'attaque selon direction */
    int ax = j->x, ay = j->y, aw = j->w, ah = j->h;
    switch (j->dir) {
        case DIR_HAUT:   ay -= ATTAQUE_PORTEE; ah = ATTAQUE_PORTEE; break;
        case DIR_BAS:    ay += j->h;           ah = ATTAQUE_PORTEE; break;
        case DIR_GAUCHE: ax -= ATTAQUE_PORTEE; aw = ATTAQUE_PORTEE; break;
        case DIR_DROITE: ax += j->w;           aw = ATTAQUE_PORTEE; break;
    }

    for (int i = 0; i < n; i++) {
        if (!ennemis[i].actif) continue;
        if (rect_col(ax, ay, aw, ah, ennemis[i].x, ennemis[i].y, ennemis[i].w, ennemis[i].h)) {
            ennemis[i].hp -= 25;
            if (ennemis[i].hp <= 0) ennemis[i].actif = 0;
        }
    }
}

void draw_joueur(SDL_Renderer *r, Joueur *j) {
    /* Zone attaque — arc dore (rendu avant le sprite) */
    if (j->en_attaque) {
        int ax = j->x, ay = j->y, aw = j->w, ah = ATTAQUE_PORTEE;
        switch (j->dir) {
            case DIR_HAUT:   ay -= ATTAQUE_PORTEE; aw = j->w; break;
            case DIR_BAS:    ay  = j->y + j->h; aw = j->w; break;
            case DIR_GAUCHE: ax -= ATTAQUE_PORTEE; ah = j->h; aw = ATTAQUE_PORTEE; break;
            case DIR_DROITE: ax  = j->x + j->w;   ah = j->h; aw = ATTAQUE_PORTEE; break;
        }
        draw_filled(r, ax, ay, aw, ah, 255, 220, 50, 90);
        draw_border(r, ax, ay, aw, ah, 255, 200, 0);
    }

    if (g_joueur_sprite) {
        /* Source : frame dans le spritesheet (67x135 par frame) */
        SDL_Rect src = {
            j->anim_frame * SPR_FRAME_W,
            j->anim_row   * SPR_FRAME_H,
            SPR_FRAME_W,
            SPR_FRAME_H
        };

        /*
         * Le personnage dans le spritesheet fait ~45px large et ~115px haut
         * sur une cellule de 67x135.  On scale pour que sa hauteur visuelle
         * corresponde a la hitbox (JOUEUR_H = 60px) et on aligne les pieds
         * sur le bas de la hitbox.
         *
         * Ratio : JOUEUR_H / char_height_in_cell = 60 / (135 * 0.85) ≈ 0.52
         * On utilise directement les dimensions de la hitbox pour la dest.
         */
        float scale = (float)j->h / (SPR_FRAME_H * 0.87f);
        int dw = (int)(SPR_FRAME_W * scale);  /* largeur rendue */
        int dh = j->h;                         /* hauteur = hitbox */

        /* Centré horizontalement sur la hitbox, bas aligné sur le bas hitbox */
        int dx = j->x + (j->w - dw) / 2;
        int dy = j->y;   /* le sprite remplit exactement la hitbox en hauteur */

        SDL_Rect dst = { dx, dy, dw, dh };

        /* Flip horizontal si direction gauche */
        SDL_RendererFlip flip = (j->dir == DIR_GAUCHE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        /* Teinte selon état de santé */
        EtatSante es = get_etat_sante(j->hp, j->hp_max);
        if (es == SANTE_CRITIQUE) {
            Uint8 pulse = (Uint8)(160 + 95 * fabs(sin(SDL_GetTicks() / 200.0)));
            SDL_SetTextureColorMod(g_joueur_sprite, 255, pulse / 4, pulse / 4);
        } else if (es == SANTE_BLESSE) {
            SDL_SetTextureColorMod(g_joueur_sprite, 255, 160, 100);
        } else {
            SDL_SetTextureColorMod(g_joueur_sprite, 255, 255, 255);
        }

        SDL_RenderCopyEx(r, g_joueur_sprite, &src, &dst, 0.0, NULL, flip);
        SDL_SetTextureColorMod(g_joueur_sprite, 255, 255, 255);
    } else {
        /* Fallback rectangles si pas de sprite */
        EtatSante es = get_etat_sante(j->hp, j->hp_max);
        draw_filled(r, j->x + 5, j->y + 20, j->w - 10, j->h - 20, 35, 45, 80, 255);
        draw_filled(r, j->x + 10, j->y, j->w - 20, 22, 195, 160, 120, 255);
        if (es == SANTE_BLESSE)
            draw_filled(r, j->x, j->y, j->w, j->h, 220, 80, 0, 50);
        else if (es == SANTE_CRITIQUE) {
            Uint8 a = (Uint8)(60 + 60 * fabs(sin(SDL_GetTicks() / 200.0)));
            draw_filled(r, j->x, j->y, j->w, j->h, 255, 0, 0, a);
        }
        draw_border(r, j->x, j->y, j->w, j->h, 120, 160, 255);
    }
}

/* ═══════════════════════════════════════════════════════════════
   ENNEMI
═══════════════════════════════════════════════════════════════ */
static float dist2(int ax, int ay, int bx, int by) {
    float dx = (float)(ax - bx), dy = (float)(ay - by);
    return sqrtf(dx * dx + dy * dy);
}

static void new_dest(Ennemi *e) {
    e->dest_x = 40  + rand() % (SCREEN_W - ENNEMI_W - 80);
    e->dest_y = 80  + rand() % (SCREEN_H - ENNEMI_H - 120);
}

static void move_to(Ennemi *e, int tx, int ty) {
    float dx = (float)(tx - e->x), dy = (float)(ty - e->y);
    float d  = sqrtf(dx * dx + dy * dy);
    if (d < 2.0f) return;
    e->x += (int)(dx / d * e->vitesse);
    e->y += (int)(dy / d * e->vitesse);
    if (fabsf(dx) > fabsf(dy)) e->dir = (dx > 0) ? DIR_DROITE : DIR_GAUCHE;
    else                        e->dir = (dy > 0) ? DIR_BAS    : DIR_HAUT;
}

void init_ennemis(Ennemi e[], int n, Level lv) {
    for (int i = 0; i < MAX_ENNEMIS; i++) e[i].actif = 0;

    for (int i = 0; i < n; i++) {
        Ennemi *en = &e[i];
        /* Spawn sur un bord aleatoire */
        switch (rand() % 4) {
            case 0: en->x = rand() % (SCREEN_W - ENNEMI_W); en->y = -ENNEMI_H;  break;
            case 1: en->x = rand() % (SCREEN_W - ENNEMI_W); en->y =  SCREEN_H;  break;
            case 2: en->x = -ENNEMI_W; en->y = 80 + rand() % (SCREEN_H - 180);  break;
            case 3: en->x =  SCREEN_W; en->y = 80 + rand() % (SCREEN_H - 180);  break;
        }
        en->w = ENNEMI_W;  en->h = ENNEMI_H;
        en->hp = ENNEMI_HP_MAX;  en->hp_max = ENNEMI_HP_MAX;
        en->actif    = 1;
        en->vitesse  = (lv == LEVEL_1) ? 1 : 2;
        en->en_aggro = 0;
        en->dir      = DIR_BAS;
        en->phase    = 0;
        en->attaque_timer = 0;
        en->anim_timer    = SDL_GetTicks() + (Uint32)(i * 90);
        en->anim_frame    = 0;

        /* Trajectoire Level 1 : 1 point aleatoire */
        new_dest(en);

        /* Trajectoire Level 2 : 2 points de patrouille */
        en->pt_x[0] = 50  + rand() % (SCREEN_W / 2 - 100);
        en->pt_y[0] = 80  + rand() % (SCREEN_H - 200);
        en->pt_x[1] = SCREEN_W / 2 + rand() % (SCREEN_W / 2 - 100);
        en->pt_y[1] = 80  + rand() % (SCREEN_H - 200);
    }
}

void update_ennemis(Ennemi e[], int n, Joueur *j, Level lv) {
    Uint32 now = SDL_GetTicks();
    for (int i = 0; i < n; i++) {
        Ennemi *en = &e[i];
        if (!en->actif) continue;

        /* IA : aggro si joueur proche */
        float d = dist2(en->x + en->w/2, en->y + en->h/2,
                        j->x  + j->w/2,  j->y  + j->h/2);
        en->en_aggro = (d <= AGGRO_RANGE) ? 1 : 0;

        if (en->en_aggro) {
            /* Fonce sur le joueur */
            move_to(en, j->x, j->y);
        } else if (lv == LEVEL_2) {
            /* Patrouille entre 2 points */
            int tx = en->pt_x[en->phase], ty = en->pt_y[en->phase];
            if (dist2(en->x, en->y, tx, ty) < 8) en->phase = 1 - en->phase;
            move_to(en, tx, ty);
        } else {
            /* Level 1 : point aleatoire -> nouveau si arrive */
            if (dist2(en->x, en->y, en->dest_x, en->dest_y) < 8) new_dest(en);
            move_to(en, en->dest_x, en->dest_y);
        }

        /* Limites */
        if (en->x < 0)               en->x = 0;
        if (en->y < 42)               en->y = 42;
        if (en->x + en->w > SCREEN_W) en->x = SCREEN_W - en->w;
        if (en->y + en->h > SCREEN_H) en->y = SCREEN_H - en->h;

        /* Animation */
        if (now - en->anim_timer > 140) {
            en->anim_frame = (en->anim_frame + 1) % 4;
            en->anim_timer = now;
        }
    }
}

void draw_ennemis(SDL_Renderer *r, Ennemi e[], int n) {
    for (int i = 0; i < n; i++) {
        Ennemi *en = &e[i];
        if (!en->actif) continue;

        EtatSante es = get_etat_sante(en->hp, en->hp_max);
        Uint8 cr = 130, cg = 20, cb = 20;
        if (es == SANTE_BLESSE)   { cr = 200; cg = 100; cb = 20; }
        if (es == SANTE_CRITIQUE) { cr = 230; cg =  20; cb = 20; }

        /* Corps */
        draw_filled(r, en->x + 4, en->y + 18, en->w - 8, en->h - 18, cr, cg, cb, 255);
        /* Tete */
        draw_filled(r, en->x + 8, en->y, en->w - 16, 20, 185, 145, 105, 255);
        /* Yeux rouges */
        draw_filled(r, en->x + 10, en->y + 6, 5, 4, 255, 30, 30, 255);
        draw_filled(r, en->x + 19, en->y + 6, 5, 4, 255, 30, 30, 255);
        /* Jambes animees */
        int off = (en->anim_frame % 2 == 0) ? 2 : -2;
        draw_filled(r, en->x + 4,          en->y + en->h - 12 + off, 9, 12, cr-15, cg, cb, 255);
        draw_filled(r, en->x + en->w - 13, en->y + en->h - 12 - off, 9, 12, cr-15, cg, cb, 255);

        /* Aura aggro */
        if (en->en_aggro) {
            Uint8 a = (Uint8)(80 + 60 * fabs(sin(SDL_GetTicks() / 250.0)));
            draw_filled(r, en->x-3, en->y-3, en->w+6, en->h+6, 255, 50, 50, a/5);
            draw_border(r, en->x-3, en->y-3, en->w+6, en->h+6, 255, 80, 80);
        }
        draw_border(r, en->x, en->y, en->w, en->h, 180, 50, 50);

        /* Barre sante au-dessus */
        draw_barre_sante(r, en->x, en->y - 8, en->w, 5, en->hp, en->hp_max);
    }
}

/* ═══════════════════════════════════════════════════════════════
   ES — COLLECTIBLES
═══════════════════════════════════════════════════════════════ */
void init_es(ES es[], int n) {
    for (int i = 0; i < MAX_ES; i++) es[i].actif = 0;
    for (int i = 0; i < n; i++) {
        es[i].x     = 50 + rand() % (SCREEN_W - ES_W - 100);
        es[i].y     = 80 + rand() % (SCREEN_H - ES_H - 100);
        es[i].w     = ES_W;  es[i].h = ES_H;
        es[i].actif = 1;
        es[i].est_vie = (rand() % 10 < 3) ? 1 : 0;
    }
}

void draw_es(SDL_Renderer *r, ES es[], int n) {
    Uint32 t = SDL_GetTicks();
    for (int i = 0; i < n; i++) {
        if (!es[i].actif) continue;
        float p  = (float)(0.82 + 0.18 * sin((t + i * 350) / 420.0));
        int pw = (int)(es[i].w * p), ph = (int)(es[i].h * p);
        int px = es[i].x + (es[i].w - pw) / 2;
        int py = es[i].y + (es[i].h - ph) / 2;

        if (es[i].est_vie) {
            /* Rouge = vie */
            draw_filled(r, px, py, pw, ph, 220, 30, 60, 220);
            draw_border(r, px-1, py-1, pw+2, ph+2, 255, 100, 120);
            draw_filled(r, px+pw/2-1, py+2, 3, ph-4, 255, 255, 255, 200);
            draw_filled(r, px+2, py+ph/2-1, pw-4, 3, 255, 255, 255, 200);
        } else {
            /* Or = score */
            draw_filled(r, px, py, pw, ph, 220, 180, 20, 220);
            draw_border(r, px-1, py-1, pw+2, ph+2, 255, 230, 80);
            draw_filled(r, px+pw/2-1, py+2, 3, ph-4, 255, 255, 180, 200);
            draw_filled(r, px+2, py+ph/2-1, pw-4, 3, 255, 255, 180, 200);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
   COLLISION  —  1 seule fonction rect_col, appelee 2 fois
═══════════════════════════════════════════════════════════════ */
int rect_col(int ax, int ay, int aw, int ah,
             int bx, int by, int bw, int bh) {
    return (ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by);
}

void gerer_collisions(Joueur *j, Ennemi e[], int ne,
                      ES es[], int nes, ScoreInfo *sc) {
    Uint32 now = SDL_GetTicks();

    /* Appel 1 : Joueur + Ennemi */
    for (int i = 0; i < ne; i++) {
        if (!e[i].actif) continue;
        if (rect_col(j->x, j->y, j->w, j->h,
                     e[i].x, e[i].y, e[i].w, e[i].h)) {
            if (now - e[i].attaque_timer > ATTAQUE_COOLDOWN) {
                j->hp -= ENNEMI_DEGATS;
                if (j->hp < 0) j->hp = 0;
                e[i].attaque_timer = now;
            }
        }
    }

    /* Appel 2 : Joueur + ES */
    for (int i = 0; i < nes; i++) {
        if (!es[i].actif) continue;
        if (rect_col(j->x, j->y, j->w, j->h,
                     es[i].x, es[i].y, es[i].w, es[i].h)) {
            es[i].actif = 0;
            if (es[i].est_vie) {
                j->hp += ES_VIE_VAL;
                if (j->hp > j->hp_max) j->hp = j->hp_max;
            } else {
                j->score        += ES_SCORE_VAL;
                sc->score_total += ES_SCORE_VAL;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
   SANTE
═══════════════════════════════════════════════════════════════ */
EtatSante get_etat_sante(int hp, int hp_max) {
    if (hp_max <= 0) return SANTE_CRITIQUE;
    float ratio = (float)hp / hp_max;
    if (ratio > 0.50f) return SANTE_VIVANT;
    if (ratio > 0.25f) return SANTE_BLESSE;
    return SANTE_CRITIQUE;
}

void draw_barre_sante(SDL_Renderer *r, int x, int y, int w, int h,
                      int hp, int hp_max) {
    if (hp_max <= 0) return;
    float ratio = (float)hp / hp_max;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    draw_filled(r, x, y, w, h, 40, 40, 40, 200);
    int filled = (int)(w * ratio);
    EtatSante es = get_etat_sante(hp, hp_max);
    Uint8 R = 50, G = 200, B = 50;
    if (es == SANTE_BLESSE)   { R = 230; G = 140; B =  20; }
    if (es == SANTE_CRITIQUE) {
        R = 220; G = 30; B = 30;
        Uint8 a = (Uint8)(180 + 70 * fabs(sin(SDL_GetTicks() / 250.0)));
        draw_filled(r, x, y, filled, h, R, G, B, a);
        draw_border(r, x, y, w, h, 255, 60, 60);
        return;
    }
    draw_filled(r, x, y, filled, h, R, G, B, 220);
    draw_border(r, x, y, w, h, 80, 80, 80);
}

void draw_hud(SDL_Renderer *r, TTF_Font *f, Joueur *j, ScoreInfo *sc) {
    char buf[128];
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color or_   = {255, 215,   0, 255};
    SDL_Color rouge = {255,  80,  80, 255};

    /* Fond HUD */
    draw_filled(r, 0, 0, SCREEN_W, 42, 5, 5, 15, 210);

    /* Barre vie */
    draw_text(r, f, "VIE", 8, 12, blanc);
    draw_barre_sante(r, 40, 13, 160, 16, j->hp, j->hp_max);
    snprintf(buf, sizeof(buf), "%d/%d", j->hp, j->hp_max);
    draw_text(r, f, buf, 207, 12,
              get_etat_sante(j->hp, j->hp_max) == SANTE_CRITIQUE ? rouge : blanc);

    /* Score */
    snprintf(buf, sizeof(buf), "SCORE: %d", sc->score_total);
    draw_text(r, f, buf, 320, 12, or_);

    /* Niveau */
    snprintf(buf, sizeof(buf), "LEVEL %d", sc->level_actuel);
    draw_text(r, f, buf, 530, 12, blanc);

    /* Ennemis */
    snprintf(buf, sizeof(buf), "ENNEMIS: %d", sc->ennemis_restants);
    draw_text(r, f, buf, 640, 12, rouge);

    /* Vignette sang critique */
    if (get_etat_sante(j->hp, j->hp_max) == SANTE_CRITIQUE) {
        Uint8 a = (Uint8)(25 + 25 * fabs(sin(SDL_GetTicks() / 350.0)));
        draw_filled(r, 0, 0, 18, SCREEN_H, 180, 0, 0, a);
        draw_filled(r, SCREEN_W-18, 0, 18, SCREEN_H, 180, 0, 0, a);
        draw_filled(r, 0, 0, SCREEN_W, 18, 180, 0, 0, a);
        draw_filled(r, 0, SCREEN_H-18, SCREEN_W, 18, 180, 0, 0, a);
    }

    /* Aide */
    draw_text(r, f, "[ESPACE]=Attaque  [P]=Pause", SCREEN_W - 230, SCREEN_H - 22,
              (SDL_Color){150, 150, 180, 200});
}

/* ═══════════════════════════════════════════════════════════════
   SCORES
═══════════════════════════════════════════════════════════════ */
#define MAX_REC 10
typedef struct { char nom[32]; int score; } Record;

void sauvegarder_score(const char *nom, int score) {
    Record rec[MAX_REC]; int n = 0;
    FILE *f = fopen(SCORES_FILE, "r");
    if (f) { while (n < MAX_REC && fscanf(f, "%31s %d", rec[n].nom, &rec[n].score) == 2) n++; fclose(f); }

    if (n < MAX_REC) { strncpy(rec[n].nom, nom, 31); rec[n].nom[31]='\0'; rec[n].score = score; n++; }
    else {
        int mi = 0;
        for (int i = 1; i < n; i++) if (rec[i].score < rec[mi].score) mi = i;
        if (score > rec[mi].score) { strncpy(rec[mi].nom, nom, 31); rec[mi].score = score; }
    }
    /* Tri decroissant */
    for (int i = 0; i < n-1; i++)
        for (int k = 0; k < n-i-1; k++)
            if (rec[k].score < rec[k+1].score) { Record tmp = rec[k]; rec[k] = rec[k+1]; rec[k+1] = tmp; }

    f = fopen(SCORES_FILE, "w");
    if (!f) return;
    for (int i = 0; i < n; i++) fprintf(f, "%s %d\n", rec[i].nom, rec[i].score);
    fclose(f);
}

void afficher_top_scores(SDL_Renderer *r, TTF_Font *f, int x, int y) {
    Record rec[MAX_REC]; int n = 0;
    FILE *fp = fopen(SCORES_FILE, "r");
    if (fp) { while (n < MAX_REC && fscanf(fp, "%31s %d", rec[n].nom, &rec[n].score) == 2) n++; fclose(fp); }

    SDL_Color or_   = {255, 215, 0, 255};
    SDL_Color blanc = {220, 220, 220, 255};
    draw_text(r, f, "TOP SCORES", x, y, or_);
    if (!n) { draw_text(r, f, "Aucun score", x, y+26, blanc); return; }
    for (int i = 0; i < n && i < 5; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d. %-12s %d", i+1, rec[i].nom, rec[i].score);
        draw_text(r, f, buf, x, y + 26 + i * 24, (i == 0) ? or_ : blanc);
    }
}

/* ═══════════════════════════════════════════════════════════════
   NIVEAU
═══════════════════════════════════════════════════════════════ */
void init_niveau(Joueur *j, Ennemi e[], ES es[], ScoreInfo *sc, Level lv) {
    int nb_ennemis = (lv == LEVEL_1) ? 3 : MAX_ENNEMIS;
    int nb_es      = (lv == LEVEL_1) ? 5 : MAX_ES;

    init_joueur(j);
    j->x = (lv == LEVEL_1) ? 80 : SCREEN_W - 120;
    j->y = SCREEN_H / 2;

    init_ennemis(e, nb_ennemis, lv);
    init_es(es, nb_es);

    sc->level_actuel     = (int)lv;
    sc->ennemis_restants = nb_ennemis;
}

int niveau_termine(Ennemi e[], int n) {
    for (int i = 0; i < n; i++) if (e[i].actif) return 0;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════
   MENUS
═══════════════════════════════════════════════════════════════ */
static void overlay(SDL_Renderer *r, Uint8 a) {
    draw_filled(r, 0, 0, SCREEN_W, SCREEN_H, 0, 0, 0, a);
}

static void panneau(SDL_Renderer *r, int x, int y, int w, int h) {
    draw_filled(r, x, y, w, h, 10, 10, 25, 215);
    draw_border(r, x, y, w, h, 255, 215, 0);
    draw_border(r, x+2, y+2, w-4, h-4, 80, 60, 0);
}

/* ── Dessine le bouton image avec texte centré dessus ────────── */
void draw_btn(SDL_Renderer *r, SDL_Texture *btn_tex, TTF_Font *f,
              const char *label, int x, int y, int w, int h, int selected) {
    /* Image du bouton */
    SDL_Rect dst = {x, y, w, h};
    if (btn_tex) {
        /* Luminosité réduite si non sélectionné */
        if (!selected) SDL_SetTextureColorMod(btn_tex, 130, 130, 130);
        else           SDL_SetTextureColorMod(btn_tex, 255, 255, 255);
        SDL_RenderCopy(r, btn_tex, NULL, &dst);
        SDL_SetTextureColorMod(btn_tex, 255, 255, 255); /* reset */
    } else {
        /* Fallback si pas de texture */
        draw_filled(r, x, y, w, h, 20, 20, 50, 220);
        draw_border(r, x, y, w, h, selected?255:90, selected?215:90, 0);
    }

    /* Halo doré si sélectionné */
    if (selected) {
        Uint8 a = (Uint8)(60 + 40 * fabs(sin(SDL_GetTicks() / 400.0)));
        draw_filled(r, x-3, y-3, w+6, h+6, 255, 215, 0, a/4);
        draw_border(r, x-3, y-3, w+6, h+6, 255, 215, 0);
    }

    /* Texte centré sur le bouton — décalé à droite pour éviter la toupie */
    if (f && label) {
        SDL_Surface *s = TTF_RenderUTF8_Blended(f, label,
            selected ? (SDL_Color){255,255,255,255} : (SDL_Color){180,180,180,255});
        if (s) {
            SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
            if (tx) {
                /* Centrer dans la zone texte (après la toupie ~25%) */
                int text_zone_x = x + w/4;
                int text_zone_w = w - w/4 - 10;
                int tx_x = text_zone_x + (text_zone_w - s->w) / 2;
                int tx_y = y + (h - s->h) / 2;
                SDL_Rect tdst = {tx_x, tx_y, s->w, s->h};
                SDL_RenderCopy(r, tx, NULL, &tdst);
                SDL_DestroyTexture(tx);
            }
            SDL_FreeSurface(s);
        }
    }
}

/* Menu principal — choix Level 1 / Level 2 avec image boutons */
void draw_menu(SDL_Renderer *r, TTF_Font *f, TTF_Font *fs,
               SDL_Texture *bg, SDL_Texture *btn, int sel) {
    draw_bg(r, bg);
    overlay(r, 140);

    SDL_Color gris = {160, 160, 180, 255};

    /* Titre */
    Uint8 ta = (Uint8)(200 + 55 * fabs(sin(SDL_GetTicks() / 700.0)));
    draw_text(r, f, "I N C E P T I O N", SCREEN_W/2 - 155, 45, (SDL_Color){255,215,0,ta});
    draw_text(r, fs, "Incarnez  S A I T O  dans le reve", SCREEN_W/2 - 128, 90, gris);
    draw_filled(r, SCREEN_W/2-140, 116, 280, 2, 255, 215, 0, 180);

    /* Boutons image — LEVEL 1 et LEVEL 2 côte à côte */
    int bw = 280, bh = 70, gap = 20;
    int bx1 = SCREEN_W/2 - bw - gap/2;
    int bx2 = SCREEN_W/2 + gap/2;
    int by  = 135;

    draw_btn(r, btn, f, "LEVEL 1  —  3 ennemis", bx1, by, bw, bh, sel == 1);
    draw_btn(r, btn, f, "LEVEL 2  —  5 ennemis", bx2, by, bw, bh, sel == 2);

    /* Sous-titres */
    draw_text(r, fs, "lent",   bx1 + bw/2 - 15, by + bh + 4,
              sel==1?(SDL_Color){80,220,100,255}:gris);
    draw_text(r, fs, "rapide", bx2 + bw/2 - 20, by + bh + 4,
              sel==2?(SDL_Color){80,220,100,255}:gris);

    /* Instructions */
    draw_filled(r, SCREEN_W/2-200, 232, 400, 1, 100, 100, 140, 120);
    draw_text(r, fs, "1 / 2 ou Fleches : choisir     ENTREE : jouer", SCREEN_W/2-172, 240, gris);
    draw_text(r, fs, "Deplacement : Fleches / ZQSD       ESPACE : Attaquer", SCREEN_W/2-185, 262, gris);

    /* Top scores */
    afficher_top_scores(r, fs, SCREEN_W - 210, 135);

    /* ECHAP reste fonctionnel — discret en bas */
    draw_text(r, fs, "ECHAP : Quitter", SCREEN_W/2 - 55, SCREEN_H - 26,
              (SDL_Color){100, 100, 120, 180});
}

/* Saisie du nom — bouton JOUER en image */
void draw_saisie_nom(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg,
                     SDL_Texture *btn, const char *nom, Level lv) {
    draw_bg(r, bg);
    overlay(r, 150);

    int pw = 440, ph = 230;
    int px = SCREEN_W/2 - pw/2, py = SCREEN_H/2 - ph/2;
    panneau(r, px, py, pw, ph);

    SDL_Color or_   = {255, 215, 0, 255};
    SDL_Color blanc = {230, 230, 230, 255};
    SDL_Color gris  = {160, 160, 180, 255};

    char buf[64];
    snprintf(buf, sizeof(buf), "LEVEL %d  —  Entrez votre nom :", lv);
    draw_text(r, f, buf, px+20, py+18, or_);

    /* Champ saisie */
    draw_filled(r, px+20, py+58, pw-40, 42, 5, 5, 20, 240);
    draw_border(r, px+20, py+58, pw-40, 42, 255, 215, 0);
    char affiche[64];
    snprintf(affiche, sizeof(affiche), "%s%c", nom,
             (SDL_GetTicks()/500)%2 ? '|' : ' ');
    draw_text(r, f, affiche, px+30, py+67, blanc);

    /* Bouton JOUER en image */
    int bw = 200, bh = 58;
    int bx = px + pw/2 - bw/2;
    int bby = py + 120;
    draw_btn(r, btn, f, "JOUER", bx, bby, bw, bh, 1);

    draw_text(r, f, "ECHAP : Retour", px+20, py+ph-28, gris);
}

/* Pause — boutons image Reprendre */
void draw_pause(SDL_Renderer *r, TTF_Font *f, SDL_Texture *btn) {
    overlay(r, 150);
    int pw=380, ph=180, px=SCREEN_W/2-pw/2, py=SCREEN_H/2-ph/2;
    panneau(r, px, py, pw, ph);
    draw_text(r, f, "—  PAUSE  —", px+110, py+18, (SDL_Color){255,215,0,255});

    /* Bouton Reprendre */
    draw_btn(r, btn, f, "Reprendre  [ P ]", px+40, py+65, 300, 62, 1);

    draw_text(r, f, "ECHAP : Menu", px+115, py+142, (SDL_Color){150,150,170,200});
}

/* Game Over — boutons Rejouer */
void draw_game_over(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg,
                    SDL_Texture *btn, ScoreInfo *sc) {
    draw_bg(r, bg);
    overlay(r, 170);
    int pw=420, ph=250, px=SCREEN_W/2-pw/2, py=SCREEN_H/2-ph/2;
    panneau(r, px, py, pw, ph);

    draw_text(r, f, "GAME  OVER", px+120, py+15, (SDL_Color){220,40,40,255});
    char buf[64];
    snprintf(buf, sizeof(buf), "Score final  : %d", sc->score_total);
    draw_text(r, f, buf, px+60, py+68, (SDL_Color){255,215,0,255});
    snprintf(buf, sizeof(buf), "Niveau       : %d", sc->level_actuel);
    draw_text(r, f, buf, px+60, py+98, (SDL_Color){220,220,220,255});

    /* Bouton Rejouer */
    draw_btn(r, btn, f, "Rejouer  [ ENTREE ]", px+60, py+138, 300, 62, 1);

    draw_text(r, f, "ECHAP : Menu", px+130, py+215, (SDL_Color){150,150,170,200});
}

/* Victoire — bouton Menu */
void draw_win(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg,
              SDL_Texture *btn, ScoreInfo *sc) {
    draw_bg(r, bg);
    overlay(r, 160);
    int pw=460, ph=250, px=SCREEN_W/2-pw/2, py=SCREEN_H/2-ph/2;
    panneau(r, px, py, pw, ph);

    Uint8 ta = (Uint8)(200 + 55 * fabs(sin(SDL_GetTicks() / 600.0)));
    draw_text(r, f, "VICTOIRE !", px+155, py+15, (SDL_Color){255,215,0,ta});
    draw_text(r, f, "Le reve est accompli.", px+80, py+55, (SDL_Color){220,220,220,255});
    char buf[64];
    snprintf(buf, sizeof(buf), "Score Total : %d", sc->score_total);
    draw_text(r, f, buf, px+100, py+98, (SDL_Color){80,220,100,255});

    /* Bouton Menu */
    draw_btn(r, btn, f, "Menu  [ ENTREE ]", px+80, py+142, 300, 62, 1);

    draw_text(r, f, "ECHAP : Quitter", px+140, py+218, (SDL_Color){150,150,170,200});
}
