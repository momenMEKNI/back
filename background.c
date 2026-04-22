#include "background.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void drawText(SDL_Renderer *renderer, TTF_Font *font,
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

void initBackground(GameState *gs, SDL_Renderer *renderer, const char *path, int niveau)
{
    (void)niveau;

    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
        gs->texture = NULL;
    else
    {
        gs->texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    gs->pos     = (SDL_Rect){0, 0, 800, 600};
    gs->camera  = (SDL_Rect){0, 0, 800, 600};
    gs->camera2 = (SDL_Rect){0, 0, 800, 600};
    gs->startTime = SDL_GetTicks();

    gs->nbScores = 0;
    gs->guide    = NULL;
}

void afficherBackground(GameState *gs, SDL_Renderer *renderer, int mode)
{
    if (gs->texture)
    {
        if (mode == 0)
        {
            SDL_RenderCopy(renderer, gs->texture, &gs->camera, &gs->pos);
        }
        else
        {
            SDL_Rect left = {0,   0, 400, 600};
            SDL_Rect cam1 = gs->camera;
            cam1.w = 400; cam1.h = 600;
            SDL_RenderCopy(renderer, gs->texture, &cam1, &left);

            SDL_Rect right = {400, 0, 400, 600};
            SDL_Rect cam2  = gs->camera2;
            cam2.w = 400; cam2.h = 600;
            SDL_RenderCopy(renderer, gs->texture, &cam2, &right);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, 400, 0, 400, 600);
        }
    }
}

void scrolling(GameState *gs, int direction)
{
    int speed = 10;

    if (direction == 0) gs->camera.x += speed;
    if (direction == 1) gs->camera.x -= speed;
    if (direction == 2) gs->camera.y += speed;
    if (direction == 3) gs->camera.y -= speed;

    if (gs->camera.x < 0) gs->camera.x = 0;
    if (gs->camera.y < 0) gs->camera.y = 0;
    if (gs->camera.x > 1600 - gs->camera.w) gs->camera.x = 1600 - gs->camera.w;
    if (gs->camera.y > 1200 - gs->camera.h) gs->camera.y = 1200 - gs->camera.h;
}

void afficherTemps(GameState *gs, SDL_Renderer *renderer, TTF_Font *font)
{
    Uint32 totalSec = (SDL_GetTicks() - gs->startTime) / 1000;
    Uint32 minutes  = totalSec / 60;
    Uint32 secondes = totalSec % 60;

    char texte[32];
    sprintf(texte, "Temps: %02d:%02d", minutes, secondes);

    SDL_Color jaune = {255, 220, 0, 255};
    drawText(renderer, font, texte, jaune, 10, 10);
}

void initGuide(GameState *gs, SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_Surface *s = IMG_Load("guide.png");
    if (s)
    {
        gs->guide = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
        return;
    }

    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET, 440, 320);
    if (!tex) { gs->guide = NULL; return; }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, tex);

    SDL_SetRenderDrawColor(renderer, 15, 15, 50, 230);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 200);
    SDL_Rect bord = {0, 0, 440, 320};
    SDL_RenderDrawRect(renderer, &bord);

    SDL_Color jaune = {255, 220,   0, 255};
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color gris  = {160, 160, 160, 255};
    SDL_Color vert  = {  0, 220,  80, 255};

    int ox = 0;
    int oy = 14;

    drawText(renderer, font, "=== GUIDE DU JEU ===", jaune, ox + 110, oy); oy += 30;

    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    drawText(renderer, font, "DEPLACEMENT", jaune, ox + 10, oy); oy += 26;
    drawText(renderer, font, "  Fleches  ->  Scrolling camera P1", blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  Z/Q/S/D  ->  Scrolling camera P2", blanc, ox + 10, oy); oy += 28;

    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    drawText(renderer, font, "GAMEPLAY", jaune, ox + 10, oy); oy += 26;
    drawText(renderer, font, "  1        ->  Charger Level 1", blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  2        ->  Charger Level 2", blanc, ox + 10, oy); oy += 28;

    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    drawText(renderer, font, "AFFICHAGE", jaune, ox + 10, oy); oy += 26;
    drawText(renderer, font, "  M        ->  Mono / Split-screen",     blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  H        ->  Afficher / Cacher guide", blanc, ox + 10, oy); oy += 22;
    drawText(renderer, font, "  F2       ->  Meilleurs scores",         blanc, ox + 10, oy); oy += 28;

    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 100);
    SDL_RenderDrawLine(renderer, ox, oy, ox + 410, oy);
    oy += 10;

    drawText(renderer, font, "Appuyez sur H pour fermer", gris, ox + 110, oy);
    drawText(renderer, font, "  ECHAP -> Quitter le jeu", vert, ox + 10, oy);

    SDL_SetRenderTarget(renderer, NULL);
    gs->guide = tex;
}

void gererScores(GameState *gs, SDL_Renderer *renderer, TTF_Font *font, char nom[], int score, int action)
{
    int i;

    if (action == 0)
    {
        memset(nom, 0, 50);
        int len    = 0;
        int saisie = 1;
        int valide;
        SDL_Event e;

        SDL_StartTextInput();

        while (saisie)
        {
            SDL_SetRenderDrawColor(renderer, 10, 10, 40, 255);
            SDL_RenderClear(renderer);

            SDL_Color jaune = {255, 220,   0, 255};
            SDL_Color blanc = {255, 255, 255, 255};
            SDL_Color gris  = {160, 160, 160, 255};

            drawText(renderer, font, "Partie terminee !", jaune, 290, 170);
            drawText(renderer, font, "Entrez votre nom :", blanc, 286, 230);

            SDL_Rect boite = {240, 290, 320, 46};
            SDL_SetRenderDrawColor(renderer, 35, 35, 80, 255);
            SDL_RenderFillRect(renderer, &boite);
            SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
            SDL_RenderDrawRect(renderer, &boite);

            char affiche[64];
            if ((SDL_GetTicks() / 500) % 2 == 0)
                sprintf(affiche, "%s_", nom);
            else
                sprintf(affiche, "%s ", nom);

            const char *aff = (affiche[0]) ? affiche : "_";
            drawText(renderer, font, aff, jaune, 260, 302);
            drawText(renderer, font, "ENTREE pour valider", gris, 300, 370);

            SDL_RenderPresent(renderer);

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
                    if (len < 49)
                    {
                        char c = e.text.text[0];
                        valide = ((c >= 'a' && c <= 'z') ||
                                  (c >= 'A' && c <= 'Z') ||
                                  (c >= '0' && c <= '9'));
                        if (valide)
                        {
                            nom[len] = c;
                            len++;
                            nom[len] = '\0';
                        }
                    }
                }
            }
            SDL_Delay(16);
        }

        SDL_StopTextInput();

        FILE *f = fopen("scores.txt", "a");
        if (f)
        {
            fprintf(f, "%s %d\n", nom, score);
            fclose(f);
        }
    }
    else if (action == 1)
    {
        FILE *f = fopen("scores.txt", "r");
        int n = 0;
        if (f)
        {
            while (n < 20 && fscanf(f, "%49s %d", gs->scores[n].nom, &gs->scores[n].score) == 2)
            {
                int j;
                int ok = 1;
                for (j = 0; gs->scores[n].nom[j] != '\0'; j++)
                {
                    char c = gs->scores[n].nom[j];
                    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
                    {
                        ok = 0;
                        break;
                    }
                }
                if (ok && gs->scores[n].score >= 0)
                    n++;
            }
            fclose(f);
        }
        gs->nbScores = n;

        int a, b;
        for (a = 0; a < n - 1; a++)
            for (b = 0; b < n - 1 - a; b++)
                if (gs->scores[b].score < gs->scores[b+1].score)
                {
                    char tmpNom[50];
                    int  tmpScore = gs->scores[b].score;
                    strncpy(tmpNom, gs->scores[b].nom, 50);
                    gs->scores[b].score = gs->scores[b+1].score;
                    strncpy(gs->scores[b].nom, gs->scores[b+1].nom, 50);
                    gs->scores[b+1].score = tmpScore;
                    strncpy(gs->scores[b+1].nom, tmpNom, 50);
                }

        int running = 1;
        SDL_Event e;

        while (running)
        {
            SDL_SetRenderDrawColor(renderer, 10, 10, 30, 255);
            SDL_RenderClear(renderer);

            SDL_Color jaune = {255, 220,   0, 255};
            SDL_Color blanc = {255, 255, 255, 255};
            SDL_Color gris  = {160, 160, 160, 255};
            SDL_Color bleu  = { 80, 180, 255, 255};

            drawText(renderer, font, "=== MEILLEURS SCORES ===", jaune, 220, 50);

            SDL_SetRenderDrawColor(renderer, 80, 160, 255, 200);
            SDL_RenderDrawLine(renderer, 100, 90, 700, 90);

            if (n == 0)
            {
                drawText(renderer, font, "Aucun score enregistre.", gris, 280, 260);
            }
            else
            {
                drawText(renderer, font, "  #   Joueur                Score", bleu, 130, 110);
                SDL_RenderDrawLine(renderer, 100, 135, 700, 135);

                for (i = 0; i < n && i < 10; i++)
                {
                    char ligne[80];
                    sprintf(ligne, " %2d.  %-20s  %d pts",
                            i + 1, gs->scores[i].nom, gs->scores[i].score);
                    SDL_Color c = (i == 0) ? jaune : blanc;
                    drawText(renderer, font, ligne, c, 130, 150 + i * 36);
                }
            }

            SDL_SetRenderDrawColor(renderer, 80, 160, 255, 200);
            SDL_RenderDrawLine(renderer, 100, 540, 700, 540);
            drawText(renderer, font, "Appuyez sur ENTREE ou ECHAP pour fermer", gris, 185, 555);

            SDL_RenderPresent(renderer);

            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) running = 0;
                if (e.type == SDL_KEYDOWN &&
                   (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_ESCAPE))
                    running = 0;
            }
            SDL_Delay(16);
        }
    }
    else if (action == 2)
    {
        if (gs->texture) SDL_DestroyTexture(gs->texture);
        if (gs->guide)   SDL_DestroyTexture(gs->guide);
        gs->texture = NULL;
        gs->guide   = NULL;
    }
    else if (action == 3)
    {
        if (gs->guide)
        {
            SDL_Rect zone = {180, 110, 440, 320};
            SDL_RenderCopy(renderer, gs->guide, NULL, &zone);
        }
    }
}
