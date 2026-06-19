/*
 * main.c — Boucle principale INCEPTION Game (Saito)
 *
 * Controles :
 *   Fleches / ZQSD : deplacer Saito
 *   ESPACE         : attaquer
 *   1 / 2          : choisir niveau au menu
 *   ENTREE         : confirmer
 *   P              : pause
 *   ECHAP          : quitter
 */

#include "header.h"

/* Cherche une police TTF disponible sur le systeme */
static TTF_Font *charger_police(int taille) {
    const char *chemins[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        NULL
    };
    for (int i = 0; chemins[i]; i++) {
        TTF_Font *f = TTF_OpenFont(chemins[i], taille);
        if (f) return f;
    }
    return NULL;
}

static int compter_ennemis(Ennemi e[], int n) {
    int cnt = 0;
    for (int i = 0; i < n; i++) if (e[i].actif) cnt++;
    return cnt;
}

int main(void) {
    SDL_Window   *win = NULL;
    SDL_Renderer *ren = NULL;
    if (!init_sdl(&win, &ren)) return 1;

    TTF_Font *font  = charger_police(22);
    TTF_Font *fontS = charger_police(16);
    if (!font) {
        fprintf(stderr,
            "Police TTF introuvable.\n"
            "Installez : sudo apt install fonts-dejavu\n");
        quit_sdl(win, ren); return 1;
    }
    if (!fontS) fontS = font;

    /* Chargement du background */
    SDL_Texture *bg = IMG_LoadTexture(ren, "assets/background.png");
    if (!bg) fprintf(stderr, "Background: %s\n", IMG_GetError());

    /* Bouton image */
    SDL_Texture *btn = IMG_LoadTexture(ren, "assets/btn.png");
    if (!btn) fprintf(stderr, "Bouton: %s\n", IMG_GetError());

    /* Donnees jeu */
    Joueur    joueur;
    Ennemi    ennemis[MAX_ENNEMIS];
    ES        es_list[MAX_ES];
    ScoreInfo sc;
    memset(&sc, 0, sizeof(sc));

    EtatJeu etat        = ETAT_MENU;
    Level   level_sel   = LEVEL_1;
    Level   level_actif = LEVEL_1;

    char nom[32] = "";
    int  nom_len = 0;

    int running = 1;
    SDL_Event ev;

    /* Activer la saisie texte */
    SDL_StartTextInput();

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        /* ── EVENEMENTS ─────────────────────────────────────── */
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = 0; break; }

            if (ev.type == SDL_KEYDOWN) {
                SDL_Keycode k = ev.key.keysym.sym;

                if (etat == ETAT_MENU) {
                    if (k == SDLK_1 || k == SDLK_LEFT)  level_sel = LEVEL_1;
                    if (k == SDLK_2 || k == SDLK_RIGHT) level_sel = LEVEL_2;
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                        nom[0] = '\0'; nom_len = 0;
                        etat = ETAT_NOM;
                    }
                    if (k == SDLK_ESCAPE) running = 0;
                }
                else if (etat == ETAT_NOM) {
                    if (k == SDLK_ESCAPE) etat = ETAT_MENU;
                    else if (k == SDLK_BACKSPACE && nom_len > 0)
                        nom[--nom_len] = '\0';
                    else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                        if (nom_len == 0) { strncpy(nom, "Saito", 31); nom_len = 5; }
                        level_actif = level_sel;
                        memset(&sc, 0, sizeof(sc));
                        init_niveau(&joueur, ennemis, es_list, &sc, level_actif);
                        etat = ETAT_JEU;
                    }
                }
                else if (etat == ETAT_JEU) {
                    if (k == SDLK_p)      etat = ETAT_PAUSE;
                    if (k == SDLK_ESCAPE) etat = ETAT_MENU;
                    if (k == SDLK_SPACE)
                        joueur_attaque(&joueur, ennemis, MAX_ENNEMIS);
                }
                else if (etat == ETAT_PAUSE) {
                    if (k == SDLK_p)      etat = ETAT_JEU;
                    if (k == SDLK_ESCAPE) etat = ETAT_MENU;
                }
                else if (etat == ETAT_GAME_OVER) {
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                        memset(&sc, 0, sizeof(sc));
                        init_niveau(&joueur, ennemis, es_list, &sc, level_actif);
                        etat = ETAT_JEU;
                    }
                    if (k == SDLK_ESCAPE) etat = ETAT_MENU;
                }
                else if (etat == ETAT_WIN) {
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_ESCAPE)
                        etat = ETAT_MENU;
                }
            }

            /* Saisie texte nom */
            if (etat == ETAT_NOM && ev.type == SDL_TEXTINPUT && nom_len < 20) {
                char c = ev.text.text[0];
                if (c >= 32 && c < 127) { nom[nom_len++] = c; nom[nom_len] = '\0'; }
            }
        }

        /* ── LOGIQUE JEU ────────────────────────────────────── */
        if (etat == ETAT_JEU) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            update_joueur(&joueur, keys);
            update_ennemis(ennemis, MAX_ENNEMIS, &joueur, level_actif);
            gerer_collisions(&joueur, ennemis, MAX_ENNEMIS, es_list, MAX_ES, &sc);
            sc.ennemis_restants = compter_ennemis(ennemis, MAX_ENNEMIS);

            /* Mort du joueur */
            if (joueur.hp <= 0) {
                if (!joueur.est_mort) {
                    joueur.est_mort = 1;
                    joueur.anim_frame = 0;
                    joueur.anim_timer = SDL_GetTicks();
                } else if (joueur.anim_frame >= SPR_FRAMES_DEATH - 1) {
                    sc.score_total += joueur.score;
                    sauvegarder_score(nom, sc.score_total);
                    etat = ETAT_GAME_OVER;
                }
            }

            /* Niveau termine */
            if (niveau_termine(ennemis, MAX_ENNEMIS)) {
                sc.score_total += joueur.score;
                if (level_actif == LEVEL_1) {
                    level_actif = LEVEL_2;
                    init_niveau(&joueur, ennemis, es_list, &sc, LEVEL_2);
                } else {
                    sauvegarder_score(nom, sc.score_total);
                    etat = ETAT_WIN;
                }
            }
        }

        /* ── RENDU ──────────────────────────────────────────── */
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        switch (etat) {
        case ETAT_MENU:
            draw_menu(ren, font, fontS, bg, btn, (int)level_sel);
            break;
        case ETAT_NOM:
            draw_saisie_nom(ren, font, bg, btn, nom, level_sel);
            break;
        case ETAT_JEU:
            draw_bg(ren, bg);
            draw_es(ren, es_list, MAX_ES);
            draw_ennemis(ren, ennemis, MAX_ENNEMIS);
            draw_joueur(ren, &joueur);
            draw_hud(ren, fontS, &joueur, &sc);
            break;
        case ETAT_PAUSE:
            draw_bg(ren, bg);
            draw_es(ren, es_list, MAX_ES);
            draw_ennemis(ren, ennemis, MAX_ENNEMIS);
            draw_joueur(ren, &joueur);
            draw_hud(ren, fontS, &joueur, &sc);
            draw_pause(ren, font, btn);
            break;
        case ETAT_GAME_OVER:
            draw_game_over(ren, font, bg, btn, &sc);
            break;
        case ETAT_WIN:
            draw_win(ren, font, bg, btn, &sc);
            break;
        }

        SDL_RenderPresent(ren);

        /* Cap FPS */
        Uint32 ft = SDL_GetTicks() - frame_start;
        if (ft < FRAME_DELAY) SDL_Delay(FRAME_DELAY - ft);
    }

    SDL_StopTextInput();
    if (btn)             SDL_DestroyTexture(btn);
    if (bg)              SDL_DestroyTexture(bg);
    if (fontS != font)   TTF_CloseFont(fontS);
    if (font)            TTF_CloseFont(font);
    quit_sdl(win, ren);
    return 0;
}
