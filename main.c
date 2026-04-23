#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "enigme.h"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700

// Définition des états du jeu
typedef enum {
    ETAT_QUIZ,
    ETAT_RESULTATS,
    ETAT_QUITTER,
    ETAT_ERREUR
} EtatJeu;

int initSDL(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    
    if (TTF_Init() < 0) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }
    
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("Erreur IMG_Init: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    
    *window = SDL_CreateWindow("Quiz Game - Inception",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               SCREEN_WIDTH, SCREEN_HEIGHT,
                               SDL_WINDOW_SHOWN);
    
    if (!*window) {
        printf("Erreur creation fenetre: %s\n", SDL_GetError());
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        printf("Erreur creation renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    
    return 1;
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    
    // Initialisation
    if (!initSDL(&window, &renderer)) {
        printf("Erreur d'initialisation\n");
        return 1;
    }
    
    int programme_actif = 1;

    // ✅ On démarre directement sur ETAT_QUIZ (plus d'accueil ni de chargement)
    EtatJeu etat_actuel = ETAT_QUIZ;
    int resultat_quiz = 0;
    int score_final = 0;
    
    while (programme_actif) {
        
        switch (etat_actuel) {
                
            case ETAT_QUIZ:
                printf("\n--- Lancement du quiz ---\n");
                resultat_quiz = runEnigme(renderer);
                score_final = resultat_quiz;
                etat_actuel = ETAT_QUITTER;
                break;
                
            case ETAT_QUITTER:
                printf("\nFermeture du jeu...\n");
                printf("Score final: %d\n", score_final);
                programme_actif = 0;
                break;
                
            case ETAT_ERREUR:
                printf("\nUne erreur est survenue !\n");
                programme_actif = 0;
                break;
                
            default:
                printf("Etat inconnu !\n");
                programme_actif = 0;
                break;
        }
    }
    
    cleanup(window, renderer);
    return 0;
}
