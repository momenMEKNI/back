#include "background.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================
//  UTILITAIRE INTERNE : afficher une ligne de texte SDL
// ============================================================
static void drawText(SDL_Renderer *renderer, TTF_Font *font,
                     const char *texte, SDL_Color couleur, int x, int y)
{
    if (!font || !texte) return;
    SDL_Surface *sf = TTF_RenderUTF8_Blended(font, texte, couleur);
    if (!sf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, sf);
    SDL_Rect pos = {x, y, sf->w, sf->h};
    SDL_RenderCopy(renderer, tex, NULL, &pos);
    SDL_FreeSurface(sf);
    SDL_DestroyTexture(tex);
}

// ============================================================
//  BACKGROUND
// ============================================================

void initBackground(Background *bg, SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
    {
        printf("Erreur chargement image: %s\n", IMG_GetError());
        bg->texture = NULL;
    }
    else
    {
        bg->texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    bg->pos     = (SDL_Rect){0, 0, 800, 600};
    bg->camera  = (SDL_Rect){0, 0, 800, 600};
    bg->camera2 = (SDL_Rect){0, 0, 800, 600};
}

/*
 * afficherBackground
 *   mode 0 : mono  -> plein ecran avec camera principale
 *   mode 1 : split -> moitie gauche (camera1) | moitie droite (camera2)
 */
void afficherBackground(Background bg, SDL_Renderer *renderer, int mode)
{
    if (!bg.texture) return;

    if (mode == 0)
    {
        SDL_RenderCopy(renderer, bg.texture, &bg.camera, &bg.pos);
    }
    else
    {
        // Vue joueur 1 (gauche)
        SDL_Rect left  = {0,   0, 400, 600};
        SDL_Rect cam1  = bg.camera;
        cam1.w = 400; cam1.h = 600;
        SDL_RenderCopy(renderer, bg.texture, &cam1, &left);

        // Vue joueur 2 (droite)
        SDL_Rect right = {400, 0, 400, 600};
        SDL_Rect cam2  = bg.camera2;
        cam2.w = 400; cam2.h = 600;
        SDL_RenderCopy(renderer, bg.texture, &cam2, &right);

        // Ligne de separation blanche
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 400, 0, 400, 600);
    }
}

void scrolling(Background *bg, int direction)
{
    const int speed = 10;

    if (direction == 0) bg->camera.x += speed;  // droite
    if (direction == 1) bg->camera.x -= speed;  // gauche
    if (direction == 2) bg->camera.y += speed;  // bas
    if (direction == 3) bg->camera.y -= speed;  // haut

    // Clampage camera principale
    if (bg->camera.x < 0) bg->camera.x = 0;
    if (bg->camera.y < 0) bg->camera.y = 0;
    if (bg->camera.x > 1600 - bg->camera.w) bg->camera.x = 1600 - bg->camera.w;
    if (bg->camera.y > 1200 - bg->camera.h) bg->camera.y = 1200 - bg->camera.h;
}

void libererBackground(Background *bg)
{
    if (bg->texture)
        SDL_DestroyTexture(bg->texture);
    bg->texture = NULL;
}

// ============================================================
//  TEMPS
// ============================================================

void initTemps(Background *bg)
{
    bg->startTime = SDL_GetTicks();
}

/*
 * Affiche le temps ecoule depuis le lancement en format MM:SS
 * Position : coin superieur gauche
 */
void afficherTemps(Background *bg, SDL_Renderer *renderer, TTF_Font *font)
{
    Uint32 totalSec = (SDL_GetTicks() - bg->startTime) / 1000;
    Uint32 minutes  = totalSec / 60;
    Uint32 secondes = totalSec % 60;

    char texte[32];
    sprintf(texte, "Temps: %02d:%02d", minutes, secondes);

    SDL_Color jaune = {255, 220, 0, 255};
    drawText(renderer, font, texte, jaune, 10, 10);
}

// ============================================================
//  PLATEFORMES
// ============================================================

/*
 * initPlateformes
 *   LEVEL1 : alternance fixe / mobile
 *   LEVEL2 : fixe / mobile / destructible (cycle sur type 0,1,2)
 */
void initPlateformes(Plateforme p[], int n, Niveau niveau)
{
   if (niveau == LEVEL1)
{
    for (int i = 0; i < n; i++)
    {
        p[i].pos       = (SDL_Rect){80 + 130*i, 400 - 60*i, 110, 18};

        // ✅ Ajout destructible
        p[i].type      = i % 3; // 0 fixe, 1 mobile, 2 destructible

        p[i].active    = 1;
        p[i].direction = 1;

        if (p[i].type == 2)
            p[i].hits = 2;
        else
            p[i].hits = 0;
    }
    }
    else // LEVEL2
    {
        for (int i = 0; i < n; i++)
        {
            p[i].pos       = (SDL_Rect){60 + 120*i, 350 - 40*(i % 3), 100, 18};
            p[i].type      = i % 3;           // 0 fixe, 1 mobile, 2 destructible
            p[i].active    = 1;
            p[i].direction = (i % 2 == 0) ? 1 : -1;
            p[i].hits      = (p[i].type == 2) ? 2 : 0;
        }
    }
}

void afficherPlateformes(Plateforme p[], int n, SDL_Renderer *renderer)
{
    for (int i = 0; i < n; i++)
    {
        if (!p[i].active) continue;

        switch (p[i].type)
        {
            case 0: // Fixe -> vert
                SDL_SetRenderDrawColor(renderer, 0, 200, 50, 255);
                break;
            case 1: // Mobile -> bleu
                SDL_SetRenderDrawColor(renderer, 50, 100, 255, 255);
                break;
            case 2: // Destructible -> rouge ou orange selon etat
                if (p[i].hits == 2)
                    SDL_SetRenderDrawColor(renderer, 255, 60,  60,  255);
                else
                    SDL_SetRenderDrawColor(renderer, 255, 160, 60,  255);
                break;
        }

        SDL_RenderFillRect(renderer, &p[i].pos);

        // Contour blanc semi-transparent
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
        SDL_RenderDrawRect(renderer, &p[i].pos);
    }
}

void updatePlateformes(Plateforme p[], int n)
{
    for (int i = 0; i < n; i++)
    {
        if (!p[i].active) continue;

        if (p[i].type == 1) // mobile
        {
            p[i].pos.x += p[i].direction * 2;
            if (p[i].pos.x > 650 || p[i].pos.x < 0)
                p[i].direction *= -1;
        }
    }
}

/* Frappe une plateforme destructible ; la desactive quand hits <= 0 */
void detruirePlateforme(Plateforme p[], int index)
{
    if (p[index].type == 2 && p[index].active)
    {
        p[index].hits--;
        if (p[index].hits <= 0)
            p[index].active = 0;
    }
}

// ============================================================
//  GUIDE
// ============================================================

/*
 * initGuide : pre-construit la texture du guide (fond + titre)
 * Le texte des controles est rendu dans afficherGuide (besoin du font).
 */
void initGuide(SDL_Texture **guide, SDL_Renderer *renderer, TTF_Font *font)
{
    // Essayer de charger guide.png
    SDL_Surface *s = IMG_Load("guide.png");
    if (s)
    {
        *guide = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
        return;
    }

    // Fallback : texture de fond vide (le contenu sera dessinee par afficherGuide)
    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET, 440, 380);
    if (!tex) { *guide = NULL; return; }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, tex);

    // Fond sombre
    SDL_SetRenderDrawColor(renderer, 15, 15, 50, 230);
    SDL_RenderClear(renderer);

    // Cadre bleu
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
    SDL_Rect cadre = {0, 0, 440, 380};
    SDL_RenderDrawRect(renderer, &cadre);

    // Ligne sous le titre
    SDL_RenderDrawLine(renderer, 10, 42, 430, 42);

    // Titre
    if (font)
    {
        SDL_Color titreCol = {80, 200, 255, 255};
        SDL_Surface *ts = TTF_RenderUTF8_Blended(font, "=== GUIDE DU JEU ===", titreCol);
        if (ts)
        {
            SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
            SDL_Rect pr = {220 - ts->w/2, 10, ts->w, ts->h};
            SDL_RenderCopy(renderer, tt, NULL, &pr);
            SDL_FreeSurface(ts);
            SDL_DestroyTexture(tt);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    *guide = tex;
    (void)font;
}

/*
 * afficherGuide
 *   Affiche un overlay sombre + fenetre de guide centree avec
 *   la liste complete des controles du jeu.
 */
void afficherGuide(SDL_Texture *guide, SDL_Renderer *renderer, TTF_Font *font)
{
    // Overlay sombre sur tout l'ecran
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_Rect ecran = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &ecran);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Fenetre guide centree 440x380
    SDL_Rect fenetre = {180, 110, 440, 380};

    // Fond de la fenetre
    SDL_SetRenderDrawColor(renderer, 15, 15, 50, 255);
    SDL_RenderFillRect(renderer, &fenetre);

    // Texture de fond pre-construite (si disponible)
    if (guide)
        SDL_RenderCopy(renderer, guide, NULL, &fenetre);

    // Bordure bleue
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
    SDL_RenderDrawRect(renderer, &fenetre);

    if (!font) return;

    SDL_Color titre  = {80,  200, 255, 255};
    SDL_Color blanc  = {255, 255, 255, 255};
    SDL_Color jaune  = {255, 220,   0, 255};
    SDL_Color vert   = {100, 220,  80, 255};
    SDL_Color gris   = {180, 180, 180, 255};

    int ox = 195;   // decalage x dans la fenetre
    int oy = 116;   // decalage y de depart

    // Titre
    drawText(renderer, font, "=== GUIDE DU JEU ===", titre, ox + 60, oy);
    oy += 36;

    // Ligne de separation
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 200);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    // Section Deplacement
    drawText(renderer, font, "DEPLACEMENT", jaune, ox + 10, oy); oy += 26;
    drawText(renderer, font, "  Fleches  ->  Scrolling camera P1",  blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  Z/Q/S/D  ->  Scrolling camera P2",  blanc, ox + 10, oy); oy += 28;

    // Ligne de separation
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    // Section Gameplay
    drawText(renderer, font, "GAMEPLAY", jaune, ox + 10, oy); oy += 26;
    drawText(renderer, font, "  ESPACE   ->  Detruire une plateforme", blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  1        ->  Charger Level 1",         blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  2        ->  Charger Level 2",         blanc, ox + 10, oy); oy += 28;

    // Ligne de separation
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    // Section Affichage
    drawText(renderer, font, "AFFICHAGE", jaune, ox + 10, oy); oy += 26;
    drawText(renderer, font, "  M        ->  Mono / Split-screen",     blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  H        ->  Afficher / Cacher guide", blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  F2       ->  Meilleurs scores",         blanc, ox + 10, oy); oy += 28;

    // Legende plateformes
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    drawText(renderer, font, "PLATEFORMES", jaune, ox + 10, oy); oy += 26;

    // Carre vert + label
    SDL_SetRenderDrawColor(renderer, 0, 200, 50, 255);
    SDL_Rect pv = {ox + 14, oy + 3, 18, 12}; SDL_RenderFillRect(renderer, &pv);
    drawText(renderer, font, "  Fixe", blanc, ox + 38, oy); oy += 22;

    // Carre bleu + label
    SDL_SetRenderDrawColor(renderer, 50, 100, 255, 255);
    SDL_Rect pb = {ox + 14, oy + 3, 18, 12}; SDL_RenderFillRect(renderer, &pb);
    drawText(renderer, font, "  Mobile", blanc, ox + 38, oy); oy += 22;

    // Carre rouge + label
    SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
    SDL_Rect pr = {ox + 14, oy + 3, 18, 12}; SDL_RenderFillRect(renderer, &pr);
    drawText(renderer, font, "  Destructible (ESPACE pour frapper)", blanc, ox + 38, oy); oy += 26;

    // Pied de fenetre
    drawText(renderer, font, "Appuyez sur H pour fermer", gris,
             ox + 110, oy);

    // Indicateur ECHAP
    drawText(renderer, font, "  ECHAP -> Quitter le jeu", vert, ox + 10, oy);
}

void libererGuide(SDL_Texture **guide)
{
    if (*guide)
        SDL_DestroyTexture(*guide);
    *guide = NULL;
}

// ============================================================
//  SCORES
// ============================================================

/* Sauvegarde un score dans scores.txt (append) */
void saveScore(char nom[], int score)
{
    FILE *f = fopen("scores.txt", "a");
    if (!f) return;
    fprintf(f, "%s %d\n", nom, score);
    fclose(f);
}

/* Charge tous les scores depuis scores.txt, les trie par ordre decroissant */
int chargerScores(Score scores[], int max)
{
    FILE *f = fopen("scores.txt", "r");
    if (!f) return 0;

    int n = 0;
    while (n < max && fscanf(f, "%49s %d", scores[n].nom, &scores[n].score) == 2)
        n++;
    fclose(f);

    // Tri decroissant (bubble sort)
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (scores[j].score < scores[j+1].score)
            {
                Score tmp    = scores[j];
                scores[j]   = scores[j+1];
                scores[j+1] = tmp;
            }
    return n;
}

/*
 * afficherScores
 *   Sous-menu meilleurs scores : fond sombre, classement trie,
 *   ENTREE ou ECHAP pour fermer.
 */
void afficherScores(SDL_Renderer *renderer, TTF_Font *font)
{
    Score scores[20];
    int   n = chargerScores(scores, 20);

    int running = 1;
    SDL_Event e;

    while (running)
    {
        // Fond
        SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
        SDL_RenderClear(renderer);

        SDL_Color jaune = {255, 220,   0, 255};
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Color gris  = {160, 160, 160, 255};
        SDL_Color bleu  = { 80, 180, 255, 255};

        // Titre
        drawText(renderer, font, "=== MEILLEURS SCORES ===", jaune, 220, 50);

        // Ligne decorative
        SDL_SetRenderDrawColor(renderer, 80, 160, 255, 200);
        SDL_RenderDrawLine(renderer, 100, 90, 700, 90);

        if (n == 0)
        {
            drawText(renderer, font, "Aucun score enregistre.", gris, 280, 260);
        }
        else
        {
            // En-tete colonnes
            drawText(renderer, font, "  #   Joueur                Score", bleu, 130, 110);
            SDL_RenderDrawLine(renderer, 100, 135, 700, 135);

            for (int i = 0; i < n && i < 10; i++)
            {
                char ligne[80];
                sprintf(ligne, " %2d.  %-20s  %d pts",
                        i + 1, scores[i].nom, scores[i].score);

                SDL_Color c = (i == 0) ? jaune : blanc;
                drawText(renderer, font, ligne, c, 130, 150 + i * 36);
            }
        }

        // Ligne decorative bas
        SDL_SetRenderDrawColor(renderer, 80, 160, 255, 200);
        SDL_RenderDrawLine(renderer, 100, 540, 700, 540);

        drawText(renderer, font, "Appuyez sur ENTREE ou ECHAP pour fermer", gris, 185, 555);

        SDL_RenderPresent(renderer);

        // Evenements
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
            if (e.type == SDL_KEYDOWN &&
               (e.key.keysym.sym == SDLK_RETURN ||
                e.key.keysym.sym == SDLK_ESCAPE))
                running = 0;
        }
        SDL_Delay(16);
    }
}

/*
 * saisirNom
 *   Ecran de saisie du nom du joueur directement dans SDL.
 *   ENTREE pour valider, BACKSPACE pour effacer, ECHAP = "Joueur".
 */
void saisirNom(char nom[], int maxLen, SDL_Renderer *renderer, TTF_Font *font)
{
    memset(nom, 0, maxLen);
    int len    = 0;
    int saisie = 1;
    SDL_Event e;

    SDL_StartTextInput();

    while (saisie)
    {
        // Fond
        SDL_SetRenderDrawColor(renderer, 10, 10, 40, 255);
        SDL_RenderClear(renderer);

        SDL_Color jaune = {255, 220,   0, 255};
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Color gris  = {160, 160, 160, 255};

        // Titre
        drawText(renderer, font, "Partie terminee !", jaune, 290, 170);
        drawText(renderer, font, "Entrez votre nom :", blanc, 286, 230);

        // Boite de saisie
        SDL_Rect boite = {240, 290, 320, 46};
        SDL_SetRenderDrawColor(renderer, 35, 35, 80, 255);
        SDL_RenderFillRect(renderer, &boite);
        SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
        SDL_RenderDrawRect(renderer, &boite);

        // Texte avec curseur clignotant
        char affiche[64];
        if ((SDL_GetTicks() / 500) % 2 == 0)
            sprintf(affiche, "%s_", nom);
        else
            sprintf(affiche, "%s ", nom);

        const char *aff = (affiche[0]) ? affiche : "_";
        drawText(renderer, font, aff, jaune, 260, 302);

        drawText(renderer, font, "ENTREE pour valider", gris, 300, 370);

        SDL_RenderPresent(renderer);

        // Evenements
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                if (len == 0) strcpy(nom, "Joueur");
                saisie = 0;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_RETURN)
                {
                    if (len == 0) strcpy(nom, "Joueur");
                    saisie = 0;
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE && len > 0)
                {
                    nom[--len] = '\0';
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    strcpy(nom, "Joueur");
                    saisie = 0;
                }
            }
            else if (e.type == SDL_TEXTINPUT)
            {
                if (len < maxLen - 1)
                {
                    strcat(nom, e.text.text);
                    len = strlen(nom);
                }
            }
        }

        SDL_Delay(16);
    }

    SDL_StopTextInput();
}
