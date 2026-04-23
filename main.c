#include "puzzle.h"

int main() {   

    /* ===== INITIALISATION SDL2 ===== */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Échec SDL_Init: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "Échec IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    if (TTF_Init() == -1) {
        fprintf(stderr, "Échec TTF_Init: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Window *fenetre = SDL_CreateWindow(
        "Jeu de Puzzle",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        LARGEUR_FENETRE, HAUTEUR_FENETRE,
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer *renderer = SDL_CreateRenderer(
        fenetre, -1, SDL_RENDERER_ACCELERATED);
    
    /* ===== POLICE ===== */
    TTF_Font *police = TTF_OpenFont("arial.ttf", 48);

    SDL_Color couleur_or    = {255, 215, 0,   255};
    SDL_Color couleur_rouge = {255, 0,   0,   255};

    /* ===== OBJETS DU JEU ===== */
    Fond        fond;
    PiecePuzzle pieces[NB_PIECES];
    Chronometre chrono;
    Message     message_victoire;
    Message     message_temps_ecoule;
    Message     message_erreur;

    srand((unsigned int)time(NULL));   
    int numero_dossier = rand() % 5 + 1;
    printf("Dossier puzzle: %d\n", numero_dossier);

    initialiserFond(&fond, renderer);
    initialiserPuzzle(pieces, numero_dossier, renderer);
    initialiserChronometre(&chrono, renderer);
    initialiserMessage(&message_victoire,
                       police, couleur_or,    "GOOD! Puzzle Completed!", renderer);
    initialiserMessage(&message_temps_ecoule,
                       police, couleur_or,    "TIME OVER!",              renderer);
    initialiserMessage(&message_erreur,
                       police, couleur_rouge, "Wrong piece! -2 secondes", renderer);

    /* Positions initiales des pièces */
    pieces[0].rect.x = 300; pieces[0].rect.y = 20;
    pieces[1].rect.x = 200; pieces[1].rect.y = 550;
    pieces[2].rect.x = 500; pieces[2].rect.y = 550;
    pieces[3].rect.x = 900; pieces[3].rect.y = 550;

    for (int i = 0; i < NB_PIECES; i++)
        pieces[i].rect_originale = pieces[i].rect;

    /* ===== VARIABLES D'ÉTAT ===== */
    int          en_cours             = 1;
    int          puzzle_termine       = 0;
    PiecePuzzle *piece_glisse         = NULL;
    Uint32       temps_message_erreur = 0;

    /* ===== BOUCLE PRINCIPALE ===== */
    while (en_cours) {

        if (!puzzle_termine)
            mettreAJourChronometre(&chrono);

        for (int i = 0; i < NB_PIECES; i++)
            if (pieces[i].secoue)
                mettreAJourSecousse(&pieces[i]);

        if (chrono.temps_ecoule && !puzzle_termine) {
            printf("=== TIME OVER ===\n");
            for (int i = 1; i < NB_PIECES; i++)
                pieces[i].visible = 0;
            message_temps_ecoule.visible         = 1;
            message_temps_ecoule.zoom            = 0.1f;
            message_temps_ecoule.etat_zoom       = 0;
            message_temps_ecoule.temps_affichage = SDL_GetTicks();
            chrono.chrono_arrete = 1;
            puzzle_termine       = 1;
        }

        if (message_victoire.visible &&
            message_victoire.etat_zoom == 2 &&
            SDL_GetTicks() - message_victoire.temps_affichage > 2000)
            en_cours = 0;

        if (message_temps_ecoule.visible &&
            message_temps_ecoule.etat_zoom == 2 &&
            SDL_GetTicks() - message_temps_ecoule.temps_affichage > 3000)
            en_cours = 0;

        if (message_erreur.visible &&
            SDL_GetTicks() - temps_message_erreur > 1000)
            message_erreur.visible = 0;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, fond.texture, NULL, &fond.rect);

        if (chrono.en_alerte && (SDL_GetTicks() % 200 < 100)) {
            appliquerEffetAlerte(chrono.textures[chrono.texture_actuelle], renderer);
            renderTexture(chrono.textures[chrono.texture_actuelle], renderer,
                          chrono.rect.x, chrono.rect.y,
                          chrono.rect.w, chrono.rect.h);
            SDL_SetTextureColorMod(
                chrono.textures[chrono.texture_actuelle], 255, 255, 255);
        } else {
            renderTexture(chrono.textures[chrono.texture_actuelle], renderer,
                          chrono.rect.x, chrono.rect.y,
                          chrono.rect.w, chrono.rect.h);
        }

        for (int i = 0; i < NB_PIECES; i++)
            if (pieces[i].visible)
                SDL_RenderCopy(renderer, pieces[i].texture, NULL, &pieces[i].rect);

        updateZoomMessage(&message_victoire);
        updateZoomMessage(&message_temps_ecoule);

        if (message_victoire.visible)
            renderTextureScaled(message_victoire.texture, renderer,
                                message_victoire.rect.x + message_victoire.rect.w / 2,
                                message_victoire.rect.y + message_victoire.rect.h / 2,
                                message_victoire.zoom);

        if (message_temps_ecoule.visible)
            renderTextureScaled(message_temps_ecoule.texture, renderer,
                                message_temps_ecoule.rect.x + message_temps_ecoule.rect.w / 2,
                                message_temps_ecoule.rect.y + message_temps_ecoule.rect.h / 2,
                                message_temps_ecoule.zoom);

        if (message_erreur.visible)
            SDL_RenderCopy(renderer, message_erreur.texture, NULL,
                           &message_erreur.rect);

        SDL_RenderPresent(renderer);

        SDL_Event evenement;
        while (SDL_PollEvent(&evenement)) {
            switch (evenement.type) {

                case SDL_QUIT:
                    en_cours = 0;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (evenement.button.button == SDL_BUTTON_LEFT &&
                        !chrono.temps_ecoule && !puzzle_termine) {
                        for (int i = 1; i < NB_PIECES; i++) {
                            if (pieces[i].visible &&
                                sourisSurPiece(&pieces[i],
                                               evenement.button.x,
                                               evenement.button.y)) {
                                piece_glisse = &pieces[i];
                                piece_glisse->decalage_glisse_x =
                                    evenement.button.x - piece_glisse->rect.x;
                                piece_glisse->decalage_glisse_y =
                                    evenement.button.y - piece_glisse->rect.y;
                                break;
                            }
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (evenement.button.button == SDL_BUTTON_LEFT &&
                        piece_glisse) {
                        if (piece_glisse == &pieces[1] &&
                            estSurPositionCible(piece_glisse, numero_dossier)) {
                            ajusterPositionCible(piece_glisse, numero_dossier);
                            puzzle_termine             = 1;
                            message_victoire.visible   = 1;
                            message_victoire.zoom      = 0.1f;
                            message_victoire.etat_zoom = 0;
                            chrono.chrono_arrete       = 1;
                            pieces[2].visible          = 0;
                            pieces[3].visible          = 0;
                            
                        } else if (piece_glisse != &pieces[1]) {
                            demarrerSecousse(piece_glisse);
                            chrono.secondes_restantes =
                                (chrono.secondes_restantes > PENALITE_TEMPS)
                                ? chrono.secondes_restantes - PENALITE_TEMPS
                                : 0;
                            chrono.texture_actuelle =
                                10 - chrono.secondes_restantes;
                            if (chrono.secondes_restantes <= 0) {
                                chrono.temps_ecoule     = 1;
                                chrono.texture_actuelle = 10;
                            }
                            message_erreur.visible = 1;
                            temps_message_erreur   = SDL_GetTicks();
                        }
                        piece_glisse = NULL;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (piece_glisse && !puzzle_termine) {
                        piece_glisse->rect.x =
                            evenement.motion.x - piece_glisse->decalage_glisse_x;
                        piece_glisse->rect.y =
                            evenement.motion.y - piece_glisse->decalage_glisse_y;
                    }
                    break;
            }
        }

        SDL_Delay(10);
    }

    /* ===== LIBÉRATION DES RESSOURCES ===== */
    libererMessage(&message_victoire);
    libererMessage(&message_temps_ecoule);
    libererMessage(&message_erreur);
    libererChronometre(&chrono);
    SDL_DestroyTexture(fond.texture);
    libererPieces(pieces, NB_PIECES);
    TTF_CloseFont(police);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(fenetre);
    SDL_Quit();

    return 0;
}
