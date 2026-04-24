#include "puzzle.h"

int main() {

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window *w = SDL_CreateWindow(
        "Puzzle",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        LARGEUR_FENETRE,
        HAUTEUR_FENETRE,
        0
    );

    SDL_Renderer *r = SDL_CreateRenderer(w,-1,0);

    TTF_Font *font = TTF_OpenFont("bg&police/arial.ttf",48);

    Jeu jeu;
    initJeu(&jeu,r,font);

    int run = 1;
    int compteur_fin = 0;

    SDL_Event e;

    while(run){

        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT)
                run = 0;

            handleEvents(&jeu,&e);
        }

        updateJeu(&jeu);
        renderJeu(&jeu,r);

        if(jeu.fini){
            compteur_fin++;

            if(compteur_fin > 120){
                run = 0;
            }
        }

        SDL_Delay(16);
    }

    cleanJeu(&jeu);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
