#include "header.h"

typedef struct { char nom[32]; int score; } Record;

SDL_Texture *g_joueur_sprite = NULL, *g_heart_full = NULL, *g_heart_empty = NULL;
SDL_Texture *g_ennemi_sheet = NULL, *g_ennemi_blesse = NULL, *g_ennemi_critique = NULL;

int init_sdl(SDL_Window **win, SDL_Renderer **ren) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return 0;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { SDL_Quit(); return 0; }
    if (TTF_Init() != 0) { IMG_Quit(); SDL_Quit(); return 0; }
    *win = SDL_CreateWindow("INCEPTION — Saito", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!*win) return 0;
    *ren = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*ren) return 0;
    SDL_SetRenderDrawBlendMode(*ren, SDL_BLENDMODE_BLEND);
    srand((unsigned)time(NULL));
    g_joueur_sprite = IMG_LoadTexture(*ren, "assets/spri_transparent.png");
    g_heart_full = IMG_LoadTexture(*ren, "assets/heart_full.png");
    g_heart_empty = IMG_LoadTexture(*ren, "assets/heart_empty.png");
    g_ennemi_sheet = IMG_LoadTexture(*ren, "assets/ennemi_sheet.png");
    g_ennemi_blesse = IMG_LoadTexture(*ren, "assets/ennemi_blesse.png");
    g_ennemi_critique = IMG_LoadTexture(*ren, "assets/ennemi_critique.png");
    return 1;
}
void quit_sdl(SDL_Window *win, SDL_Renderer *ren) {
    if (g_joueur_sprite) SDL_DestroyTexture(g_joueur_sprite);
    if (g_heart_full) SDL_DestroyTexture(g_heart_full);
    if (g_heart_empty) SDL_DestroyTexture(g_heart_empty);
    if (g_ennemi_sheet) SDL_DestroyTexture(g_ennemi_sheet);
    if (g_ennemi_blesse) SDL_DestroyTexture(g_ennemi_blesse);
    if (g_ennemi_critique) SDL_DestroyTexture(g_ennemi_critique);
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
}
void draw_filled(SDL_Renderer *r, int x, int y, int w, int h, Uint8 R, Uint8 G, Uint8 B, Uint8 A) {
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderFillRect(r, &rc);
}
void draw_border(SDL_Renderer *r, int x, int y, int w, int h, Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderDrawRect(r, &rc);
}
void draw_text(SDL_Renderer *r, TTF_Font *f, const char *t, int x, int y, SDL_Color c) {
    if (!f || !t || !*t) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, t, c);
    if (!s) return;
    SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
    if (tx) { SDL_Rect dst = {x, y, s->w, s->h}; SDL_RenderCopy(r, tx, NULL, &dst); SDL_DestroyTexture(tx); }
    SDL_FreeSurface(s);
}
void draw_bg(SDL_Renderer *r, SDL_Texture *bg) {
    if (!bg) { SDL_SetRenderDrawColor(r, 10, 10, 20, 255); SDL_RenderClear(r); return; }
    SDL_Rect dst = {0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderCopy(r, bg, NULL, &dst);
}
void init_joueur(Joueur *j) {
    j->x = SCREEN_W/2 - JOUEUR_W/2; j->y = SCREEN_H/2 - JOUEUR_H/2;
    j->w = JOUEUR_W; j->h = JOUEUR_H; j->hp = j->hp_max = JOUEUR_HP_MAX;
    j->score = j->en_mouvement = j->en_attaque = j->est_mort = j->attaque_timer = j->anim_timer = j->anim_frame = 0;
    j->dir = DIR_BAS; j->anim_row = SPR_ROW_IDLE;
}
void update_joueur(Joueur *j, const Uint8 *keys) {
    if (j->est_mort) {
        if (SDL_GetTicks() - j->anim_timer > 120 && j->anim_frame < SPR_FRAMES_DEATH-1) { j->anim_frame++; j->anim_timer = SDL_GetTicks(); }
        j->anim_row = SPR_ROW_DEATH; return;
    }
    int dx = 0, dy = 0;
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) { dy = -JOUEUR_VITESSE; j->dir = DIR_HAUT; }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) { dy = JOUEUR_VITESSE; j->dir = DIR_BAS; }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) { dx = -JOUEUR_VITESSE; j->dir = DIR_GAUCHE; }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) { dx = JOUEUR_VITESSE; j->dir = DIR_DROITE; }
    j->x += dx; j->y += dy; j->en_mouvement = (dx || dy);
    if (j->x < 0) j->x = 0; if (j->y < 42) j->y = 42;
    if (j->x+j->w > SCREEN_W) j->x = SCREEN_W-j->w; if (j->y+j->h > SCREEN_H) j->y = SCREEN_H-j->h;
    if (j->en_attaque && SDL_GetTicks() - j->attaque_timer > 300) j->en_attaque = 0;
    j->anim_row = j->en_attaque ? SPR_ROW_ATTACK : (j->en_mouvement ? SPR_ROW_RUN : SPR_ROW_IDLE);
    int spd = j->en_attaque ? 80 : (j->en_mouvement ? 100 : 160);
    if (SDL_GetTicks() - j->anim_timer > (Uint32)spd) {
        int max_f = (j->anim_row == SPR_ROW_ATTACK) ? SPR_FRAMES_ATK : (j->anim_row == SPR_ROW_RUN ? SPR_FRAMES_RUN : SPR_FRAMES_IDLE);
        j->anim_frame = (j->anim_frame + 1) % max_f; j->anim_timer = SDL_GetTicks();
    }
}
void joueur_attaque(Joueur *j, Ennemi ennemis[], int n) {
    if (SDL_GetTicks() - j->attaque_timer < 500) return;
    j->attaque_timer = SDL_GetTicks(); j->en_attaque = 1;
    int ax = j->x, ay = j->y, aw = j->w, ah = j->h;
    if (j->dir == DIR_HAUT) { ay -= ATTAQUE_PORTEE; ah = ATTAQUE_PORTEE; }
    else if (j->dir == DIR_BAS) { ay += j->h; ah = ATTAQUE_PORTEE; }
    else if (j->dir == DIR_GAUCHE) { ax -= ATTAQUE_PORTEE; aw = ATTAQUE_PORTEE; }
    else { ax += j->w; aw = ATTAQUE_PORTEE; }
    for (int i = 0; i < n; i++)
        if (ennemis[i].actif && rect_col(ax, ay, aw, ah, ennemis[i].x, ennemis[i].y, ennemis[i].w, ennemis[i].h))
            if ((ennemis[i].hp -= 25) <= 0) ennemis[i].actif = 0;
}
void draw_joueur(SDL_Renderer *r, Joueur *j) {
    if (j->en_attaque) {
        int ax = j->x, ay = j->y, aw = j->w, ah = ATTAQUE_PORTEE;
        if (j->dir == DIR_HAUT) ay -= ATTAQUE_PORTEE; else if (j->dir == DIR_BAS) ay = j->y + j->h;
        else if (j->dir == DIR_GAUCHE) { ax -= ATTAQUE_PORTEE; ah = j->h; aw = ATTAQUE_PORTEE; }
        else { ax = j->x + j->w; ah = j->h; aw = ATTAQUE_PORTEE; }
        draw_filled(r, ax, ay, aw, ah, 255, 220, 50, 90); draw_border(r, ax, ay, aw, ah, 255, 200, 0);
    }
    if (g_joueur_sprite) {
        SDL_Rect src = { j->anim_frame * SPR_FRAME_W, j->anim_row * SPR_FRAME_H, SPR_FRAME_W, SPR_FRAME_H };
        float sc = (float)j->h / (SPR_FRAME_H * 0.87f);
        SDL_Rect dst = { j->x + (j->w - (int)(SPR_FRAME_W * sc)) / 2, j->y, (int)(SPR_FRAME_W * sc), j->h };
        EtatSante es = get_etat_sante(j->hp, j->hp_max);
        if (es == SANTE_CRITIQUE) { Uint8 p = (Uint8)(160 + 95 * fabs(sin(SDL_GetTicks()/200.0))); SDL_SetTextureColorMod(g_joueur_sprite, 255, p/4, p/4); }
        else if (es == SANTE_BLESSE) SDL_SetTextureColorMod(g_joueur_sprite, 255, 160, 100);
        SDL_RenderCopyEx(r, g_joueur_sprite, &src, &dst, 0.0, NULL, (j->dir == DIR_GAUCHE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
        SDL_SetTextureColorMod(g_joueur_sprite, 255, 255, 255);
    } else { draw_filled(r, j->x+5, j->y+20, j->w-10, j->h-20, 35, 45, 80, 255); draw_border(r, j->x, j->y, j->w, j->h, 120, 160, 255); }
}
void init_ennemis(Ennemi e[], int n, Level lv) {
    for (int i = 0; i < MAX_ENNEMIS; i++) e[i].actif = 0;
    for (int i = 0; i < n; i++) {
        e[i].w = ENNEMI_W; e[i].h = ENNEMI_H; e[i].hp = e[i].hp_max = ENNEMI_HP_MAX; e[i].actif = 1;
        e[i].vitesse = (lv == LEVEL_1) ? 1 : 2; e[i].traj_type = i % 2; e[i].traj_sens = (rand()%2) ? 1 : -1;
        e[i].anim_timer = SDL_GetTicks() + (i*90);
        if (e[i].traj_type == 0) { e[i].traj_min = 10; e[i].traj_max = SCREEN_W/2-ENNEMI_W-10; e[i].x = e[i].traj_min + rand()%(e[i].traj_max-e[i].traj_min); e[i].y = 60 + rand()%(SCREEN_H-ENNEMI_H-80); e[i].dir = DIR_DROITE; }
        else { e[i].traj_min = 50; e[i].traj_max = SCREEN_H/2-ENNEMI_H-10; e[i].x = 40 + rand()%(SCREEN_W-ENNEMI_W-80); e[i].y = e[i].traj_min + rand()%(e[i].traj_max-e[i].traj_min); e[i].dir = DIR_BAS; }
    }
}
void update_ennemis(Ennemi e[], int n, Joueur *j, Level lv) {
    for (int i = 0; i < n; i++) {
        if (!e[i].actif) continue;
        float dx = (e[i].x+e[i].w/2) - (j->x+j->w/2), dy = (e[i].y+e[i].h/2) - (j->y+j->h/2);
        e[i].en_aggro = (sqrtf(dx*dx + dy*dy) <= AGGRO_RANGE);
        if (e[i].en_aggro) {
            if (e[i].traj_type == 0) { int d = j->x - e[i].x; e[i].x += (d>0?1:-1)*e[i].vitesse; e[i].dir = (d>0?DIR_DROITE:DIR_GAUCHE); }
            else { int d = j->y - e[i].y; e[i].y += (d>0?1:-1)*e[i].vitesse; e[i].dir = (d>0?DIR_BAS:DIR_HAUT); }
        } else {
            if (e[i].traj_type == 0) { e[i].x += e[i].vitesse * e[i].traj_sens; if (e[i].x <= e[i].traj_min || e[i].x >= e[i].traj_max) e[i].traj_sens *= -1; e[i].dir = (e[i].traj_sens>0?DIR_DROITE:DIR_GAUCHE); }
            else { e[i].y += e[i].vitesse * e[i].traj_sens; if (e[i].y <= e[i].traj_min || e[i].y >= e[i].traj_max) e[i].traj_sens *= -1; e[i].dir = (e[i].traj_sens>0?DIR_BAS:DIR_HAUT); }
        }
        if (SDL_GetTicks() - e[i].anim_timer > 140) { e[i].anim_frame = (e[i].anim_frame+1)%4; e[i].anim_timer = SDL_GetTicks(); }
    }
}
void draw_ennemis(SDL_Renderer *r, Ennemi e[], int n) {
    for (int i = 0; i < n; i++) {
        if (!e[i].actif) continue;
        SDL_Rect dst = { e[i].x, e[i].y, e[i].w, e[i].h };
        if (e[i].en_aggro) draw_border(r, e[i].x-4, e[i].y-4, e[i].w+8, e[i].h+8, 255, 80, 80);
        EtatSante es = get_etat_sante(e[i].hp, e[i].hp_max);
        if (es == SANTE_CRITIQUE && g_ennemi_critique) SDL_RenderCopy(r, g_ennemi_critique, NULL, &dst);
        else if (g_ennemi_sheet) { SDL_Rect src = { e[i].anim_frame * ENNEMI_W, 0, ENNEMI_W, ENNEMI_H }; SDL_RenderCopyEx(r, g_ennemi_sheet, &src, &dst, 0.0, NULL, (e[i].dir == DIR_GAUCHE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)); }
        else draw_filled(r, e[i].x, e[i].y, e[i].w, e[i].h, 200, 50, 50, 255);
        draw_barre_sante(r, e[i].x, e[i].y - 8, e[i].w, 5, e[i].hp, e[i].hp_max);
    }
}
void init_es(ES es[], int n) {
    for (int i = 0; i < MAX_ES; i++) es[i].actif = 0;
    for (int i = 0; i < n; i++) {
        es[i].x = 50 + rand()%(SCREEN_W-150); es[i].y = 80 + rand()%(SCREEN_H-150);
        es[i].w = ES_W; es[i].h = ES_H; es[i].actif = 1; es[i].est_vie = (rand()%10 < 3);
    }
}
void draw_es(SDL_Renderer *r, ES es[], int n) {
    for (int i = 0; i < n; i++) {
        if (!es[i].actif) continue;
        float p = 0.82f + 0.18f * sinf((SDL_GetTicks() + i*350)/420.0f);
        int pw = (int)(es[i].w*p), ph = (int)(es[i].h*p);
        draw_filled(r, es[i].x+(es[i].w-pw)/2, es[i].y+(es[i].h-ph)/2, pw, ph, es[i].est_vie?220:220, es[i].est_vie?30:180, es[i].est_vie?60:20, 220);
    }
}
int rect_col(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) { return (ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by); }
void gerer_collisions(Joueur *j, Ennemi e[], int ne, ES es[], int nes, ScoreInfo *sc) {
    for (int i = 0; i < ne; i++)
        if (e[i].actif && rect_col(j->x, j->y, j->w, j->h, e[i].x, e[i].y, e[i].w, e[i].h))
            if (SDL_GetTicks() - e[i].attaque_timer > ATTAQUE_COOLDOWN) { j->hp = fmax(0, j->hp-ENNEMI_DEGATS); e[i].attaque_timer = SDL_GetTicks(); }
    for (int i = 0; i < nes; i++)
        if (es[i].actif && rect_col(j->x, j->y, j->w, j->h, es[i].x, es[i].y, es[i].w, es[i].h)) {
            es[i].actif = 0; if (es[i].est_vie) j->hp = fmin(j->hp_max, j->hp+ES_VIE_VAL);
            else { j->score += ES_SCORE_VAL; sc->score_total += ES_SCORE_VAL; }
        }
}
EtatSante get_etat_sante(int hp, int hp_max) { float r = (float)hp/hp_max; return (r > 0.5f)?SANTE_VIVANT:(r > 0.25f?SANTE_BLESSE:SANTE_CRITIQUE); }
void draw_barre_sante(SDL_Renderer *r, int x, int y, int w, int h, int hp, int hp_max) {
    float ratio = fmaxf(0, fminf(1, (float)hp/hp_max)); draw_filled(r, x, y, w, h, 40, 40, 40, 200);
    EtatSante es = get_etat_sante(hp, hp_max);
    draw_filled(r, x, y, (int)(w*ratio), h, es==SANTE_VIVANT?50:(es==SANTE_BLESSE?230:220), es==SANTE_VIVANT?200:(es==SANTE_BLESSE?140:30), es==SANTE_VIVANT?50:(es==SANTE_BLESSE?20:30), 220);
    draw_border(r, x, y, w, h, 80, 80, 80);
}
void draw_hud(SDL_Renderer *r, TTF_Font *f, Joueur *j, ScoreInfo *sc) {
    char buf[64]; draw_filled(r, 0, 0, SCREEN_W, 42, 5, 5, 15, 210);
    draw_text(r, f, "VIE", 8, 12, (SDL_Color){255,255,255,255});
    for (int c = 0; c < 5; c++) {
        SDL_Rect dst = { 42 + c*25, 11, 22, 20 };
        SDL_Texture *tex = (j->hp >= (c+1)*20) ? g_heart_full : g_heart_empty;
        if (tex) SDL_RenderCopy(r, tex, NULL, &dst);
    }
    snprintf(buf, 64, "SCORE: %d  LVL: %d  ENEM: %d", sc->score_total, sc->level_actuel, sc->ennemis_restants);
    draw_text(r, f, buf, 320, 12, (SDL_Color){255,215,0,255});
}
void sauvegarder_score(const char *nom, int score) {
    Record rec[10]; int n = 0; FILE *f = fopen(SCORES_FILE, "r");
    if (f) { while (n < 10 && fscanf(f, "%s %d", rec[n].nom, &rec[n].score) == 2) n++; fclose(f); }
    if (n < 10) { strcpy(rec[n].nom, nom); rec[n].score = score; n++; }
    else if (score > rec[9].score) { strcpy(rec[9].nom, nom); rec[9].score = score; }
    for (int i=0; i<n-1; i++) for (int k=0; k<n-i-1; k++) if (rec[k].score < rec[k+1].score) { Record tmp=rec[k]; rec[k]=rec[k+1]; rec[k+1]=tmp; }
    f = fopen(SCORES_FILE, "w"); if (f) { for (int i=0; i<n; i++) fprintf(f, "%s %d\n", rec[i].nom, rec[i].score); fclose(f); }
}
void afficher_top_scores(SDL_Renderer *r, TTF_Font *f, int x, int y) {
    Record rec[5]; int n = 0; FILE *fp = fopen(SCORES_FILE, "r");
    if (fp) { while (n < 5 && fscanf(fp, "%s %d", rec[n].nom, &rec[n].score) == 2) n++; fclose(fp); }
    draw_text(r, f, "TOP SCORES", x, y, (SDL_Color){255,215,0,255});
    for (int i=0; i<n; i++) { char b[64]; snprintf(b, 64, "%d. %s %d", i+1, rec[i].nom, rec[i].score); draw_text(r, f, b, x, y+26+i*24, (SDL_Color){255,255,255,255}); }
}
void init_niveau(Joueur *j, Ennemi e[], ES es[], ScoreInfo *sc, Level lv) {
    int ne = (lv == LEVEL_1?3:MAX_ENNEMIS), nes = (lv == LEVEL_1?5:MAX_ES);
    init_joueur(j); j->x = (lv == LEVEL_1?80:SCREEN_W-120); init_ennemis(e, ne, lv); init_es(es, nes);
    sc->level_actuel = (int)lv; sc->ennemis_restants = ne;
}
int niveau_termine(Ennemi e[], int n) { for (int i=0; i<n; i++) if (e[i].actif) return 0; return 1; }
void draw_btn(SDL_Renderer *r, SDL_Texture *t, TTF_Font *f, const char *l, int x, int y, int w, int h, int sel) {
    SDL_Rect d = {x, y, w, h}; if (t) { SDL_SetTextureColorMod(t, sel?255:130, sel?255:130, sel?255:130); SDL_RenderCopy(r, t, NULL, &d); }
    if (sel) draw_border(r, x-3, y-3, w+6, h+6, 255, 215, 0);
    draw_text(r, f, l, x+w/4, y+h/4, (SDL_Color){255,255,255,255});
}
void draw_menu(SDL_Renderer *r, TTF_Font *f, TTF_Font *fs, SDL_Texture *bg, SDL_Texture *btn, int sel) {
    draw_bg(r, bg); draw_text(r, f, "I N C E P T I O N", SCREEN_W/2-155, 45, (SDL_Color){255,215,0,255});
    draw_btn(r, btn, f, "LVL 1", SCREEN_W/2-290, 135, 280, 70, sel==1);
    draw_btn(r, btn, f, "LVL 2", SCREEN_W/2+10, 135, 280, 70, sel==2);
    afficher_top_scores(r, fs, SCREEN_W-210, 135);
}
void draw_saisie_nom(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg, SDL_Texture *btn, const char *nom, Level lv) {
    draw_bg(r, bg); draw_text(r, f, "NOM:", SCREEN_W/2-200, SCREEN_H/2-80, (SDL_Color){255,215,0,255});
    draw_text(r, f, nom, SCREEN_W/2-100, SCREEN_H/2-80, (SDL_Color){255,255,255,255});
    draw_btn(r, btn, f, "JOUER", SCREEN_W/2-100, SCREEN_H/2+20, 200, 58, 1);
}
void draw_pause(SDL_Renderer *r, TTF_Font *f, SDL_Texture *btn) { draw_btn(r, btn, f, "REPRENDRE", SCREEN_W/2-150, SCREEN_H/2-30, 300, 62, 1); }
void draw_game_over(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg, SDL_Texture *btn, ScoreInfo *sc) {
    draw_bg(r, bg); draw_text(r, f, "GAME OVER", SCREEN_W/2-100, SCREEN_H/2-80, (SDL_Color){255,0,0,255});
    draw_btn(r, btn, f, "REJOUER", SCREEN_W/2-150, SCREEN_H/2+20, 300, 62, 1);
}
void draw_win(SDL_Renderer *r, TTF_Font *f, SDL_Texture *bg, SDL_Texture *btn, ScoreInfo *sc) {
    draw_bg(r, bg); draw_text(r, f, "VICTOIRE", SCREEN_W/2-100, SCREEN_H/2-80, (SDL_Color){0,255,0,255});
    draw_btn(r, btn, f, "MENU", SCREEN_W/2-150, SCREEN_H/2+20, 300, 62, 1);
}
