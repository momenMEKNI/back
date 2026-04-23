#include "puzzle.h"

/* ===== POSITIONS CIBLES ===== */
PositionCible obtenirPositionCible(int numero_dossier) {
    PositionCible pos;
    switch (numero_dossier) {
        case 1: pos.x = 293; pos.y = 363; break;
        case 2: pos.x = 440; pos.y = 343; break;
        case 3: pos.x = 301; pos.y = 15;  break;
        case 4: pos.x = 480; pos.y = 153; break;
        case 5: pos.x = 298; pos.y = 17;  break;
        default: pos.x = 300; pos.y = 20; break;
    }
    return pos;
}

/* ===== FOND ===== */
void initialiserFond(Fond *fond, SDL_Renderer *renderer) {
    SDL_Surface *surface = IMG_Load("background.jpg");
    
    fond->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    fond->rect = (SDL_Rect){0, 0, LARGEUR_FENETRE, HAUTEUR_FENETRE};
}

/* ===== PIÈCES ===== */
void initialiserPuzzle(PiecePuzzle pieces[], int numero_dossier,
                       SDL_Renderer *renderer) {
    char chemin[128];   
    for (int i = 0; i < NB_PIECES; i++) {
        snprintf(chemin, sizeof(chemin),
                 "puzzlerand/%d/puzzle%d.png", numero_dossier, i + 1);
        SDL_Surface *surface = IMG_Load(chemin);
        
        pieces[i].texture = SDL_CreateTextureFromSurface(renderer, surface);
        pieces[i].rect.w  = surface->w;
        pieces[i].rect.h  = surface->h;
        pieces[i].rect.x  = 0;   
        pieces[i].rect.y  = 0;
        SDL_FreeSurface(surface);

        pieces[i].est_glisse        = 0;
        pieces[i].decalage_glisse_x = 0;
        pieces[i].decalage_glisse_y = 0;
        pieces[i].visible           = 1;
        pieces[i].secoue            = 0;
        pieces[i].debut_secousse    = 0;
        pieces[i].rect_originale    = pieces[i].rect;
    }
}

void libererPieces(PiecePuzzle pieces[], int nombre) {
    for (int i = 0; i < nombre; i++) {
        if (pieces[i].texture)
            SDL_DestroyTexture(pieces[i].texture);
    }
}

int sourisSurPiece(PiecePuzzle *piece, int souris_x, int souris_y) {
    return piece->visible &&
           souris_x >= piece->rect.x &&
           souris_x <= piece->rect.x + piece->rect.w &&
           souris_y >= piece->rect.y &&
           souris_y <= piece->rect.y + piece->rect.h;
}

int estSurPositionCible(PiecePuzzle *piece, int numero_dossier) {
    PositionCible cible = obtenirPositionCible(numero_dossier);
    return (abs(piece->rect.x - cible.x) < SEUIL_CIBLE) &&
           (abs(piece->rect.y - cible.y) < SEUIL_CIBLE);
}

void ajusterPositionCible(PiecePuzzle *piece, int numero_dossier) {
    PositionCible cible = obtenirPositionCible(numero_dossier);
    piece->rect.x = cible.x;
    piece->rect.y = cible.y;
}

void demarrerSecousse(PiecePuzzle *piece) {
    piece->secoue         = 1;
    piece->debut_secousse = SDL_GetTicks();
    piece->rect_originale = piece->rect;
}

void mettreAJourSecousse(PiecePuzzle *piece) {
    if (!piece->secoue) return;

    Uint32 actuel = SDL_GetTicks();
    if (actuel - piece->debut_secousse > DUREE_SECOUSSE) {
        piece->secoue = 0;
        piece->rect   = piece->rect_originale;
        return;
    }

    int decalage  = (rand() % 10) - 5;
    piece->rect.x = piece->rect_originale.x + decalage;
    decalage      = (rand() % 10) - 5;
    piece->rect.y = piece->rect_originale.y + decalage;
}


/* ===== CHRONOMÈTRE ===== */
void initialiserChronometre(Chronometre *chrono, SDL_Renderer *renderer) {
    char chemin[32];
    for (int i = 0; i < 11; i++) {
        snprintf(chemin, sizeof(chemin), "%d.png", 10 - i);
        SDL_Surface *surface = IMG_Load(chemin);
        
        chrono->textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    int w, h;
    SDL_QueryTexture(chrono->textures[0], NULL, NULL, &w, &h);
    chrono->rect                = (SDL_Rect){1000, 20, w, h};
    chrono->rect_originale      = chrono->rect;
    chrono->texture_actuelle    = 0;
    chrono->temps_debut         = SDL_GetTicks();
    chrono->derniere_mise_a_jour = chrono->temps_debut;
    chrono->temps_ecoule        = 0;
    chrono->chrono_arrete       = 0;
    chrono->secondes_restantes  = 10;
    chrono->en_alerte           = 0;
    chrono->dernier_clignotement = 0;
}

void mettreAJourChronometre(Chronometre *chrono) {
    if (chrono->chrono_arrete || chrono->temps_ecoule) return;

    Uint32 temps_actuel = SDL_GetTicks();

    if (temps_actuel - chrono->derniere_mise_a_jour >= 1000) {
        chrono->secondes_restantes--;
        chrono->texture_actuelle = 10 - chrono->secondes_restantes;

        if (chrono->secondes_restantes <= ALERTE_SECONDS &&
            chrono->secondes_restantes > 0) {
            chrono->en_alerte = 1;
        } else if (chrono->secondes_restantes > ALERTE_SECONDS) {
            chrono->en_alerte = 0;
            chrono->rect      = chrono->rect_originale;
        }

        if (chrono->secondes_restantes <= 0) {
            chrono->secondes_restantes = 0;        
            chrono->temps_ecoule       = 1;
            chrono->texture_actuelle   = 10;
            chrono->en_alerte          = 0;
            chrono->rect               = chrono->rect_originale;
        }

        chrono->derniere_mise_a_jour = temps_actuel;
    }

    if (chrono->en_alerte) {
        int decalage   = (rand() % 7) - 3;
        chrono->rect.x = chrono->rect_originale.x + decalage;
        decalage       = (rand() % 7) - 3;
        chrono->rect.y = chrono->rect_originale.y + decalage;

        if (temps_actuel - chrono->dernier_clignotement > 500) {
            chrono->dernier_clignotement = temps_actuel;
        }
    }
}

void libererChronometre(Chronometre *chrono) {
    for (int i = 0; i < 11; i++) {
        if (chrono->textures[i])
            SDL_DestroyTexture(chrono->textures[i]);
    }
}

/* ===== MESSAGES ===== */
void initialiserMessage(Message *msg, TTF_Font *police, SDL_Color couleur,
                        const char *texte, SDL_Renderer *renderer) {
    SDL_Surface *surface = TTF_RenderText_Solid(police, texte, couleur);
    
    msg->texture = SDL_CreateTextureFromSurface(renderer, surface);
    msg->rect    = (SDL_Rect){400, 550, surface->w, surface->h};
    SDL_FreeSurface(surface);

    msg->visible         = 0;
    msg->temps_affichage = 0;
    msg->zoom            = 0.1f;
    msg->etat_zoom       = 0;
}

void libererMessage(Message *msg) {
    if (msg->texture)
        SDL_DestroyTexture(msg->texture);
}

void updateZoomMessage(Message *msg) {
    if (!msg->visible) return;

    switch (msg->etat_zoom) {
        case 0:
            msg->etat_zoom       = 1;
            msg->temps_affichage = SDL_GetTicks();
            break;
        case 1:
            msg->zoom += 0.05f;
            if (msg->zoom >= 1.0f) {
                msg->zoom      = 1.0f;
                msg->etat_zoom = 2;
            }
            break;
        case 2:
        default:
            break;
    }
}

/* ===== RENDU UTILITAIRES ===== */

void appliquerEffetAlerte(SDL_Texture *texture, SDL_Renderer *renderer) {
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture, 255, 100, 100);
}

void renderTexture(SDL_Texture *texture, SDL_Renderer *renderer,
                   int x, int y, int w, int h) {
    SDL_Rect dest = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
}

void renderTextureScaled(SDL_Texture *texture, SDL_Renderer *renderer,
                         int x, int y, float scale) {
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    int new_w = (int)(w * scale);
    int new_h = (int)(h * scale);
    SDL_Rect dest = {
        x - new_w / 2,   
        y - new_h / 2,
        new_w,
        new_h
    };
    SDL_RenderCopy(renderer, texture, NULL, &dest);
}
