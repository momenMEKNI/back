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
    ETAT_ACCUEIL,
    ETAT_CHARGEMENT,
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

void afficherEcranAccueil(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Ici vous pouvez afficher un écran d'accueil
    // Pour l'instant, juste un message dans la console
    printf("=== ECRAN D'ACCUEIL ===\n");
    printf("Appuyez sur une touche pour continuer...\n");
    
    SDL_RenderPresent(renderer);
    
    // Attendre une touche
    SDL_Event e;
    int attente = 1;
    while (attente) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                attente = 0;
            }
            if (e.type == SDL_KEYDOWN) {
                attente = 0;
            }
        }
        SDL_Delay(100);
    }
}

void afficherResultats(SDL_Renderer *renderer, int score) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    printf("=== RESULTATS FINAUX ===\n");
    printf("Score final: %d\n", score);
    printf("Appuyez sur une touche pour quitter...\n");
    
    SDL_RenderPresent(renderer);
    
    // Attendre une touche
    SDL_Event e;
    int attente = 1;
    while (attente) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                attente = 0;
            }
            if (e.type == SDL_KEYDOWN) {
                attente = 0;
            }
        }
        SDL_Delay(100);
    }
}

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    
    // Initialisation
    if (!initSDL(&window, &renderer)) {
        printf("Erreur d'initialisation\n");
        return 1;
    }
    
    // Variable pour contrôler la boucle principale
    int programme_actif = 1;
    
    // État initial du jeu
    EtatJeu etat_actuel = ETAT_ACCUEIL;
    int resultat_quiz = 0;
    int score_final = 0;
    
    // Boucle principale avec switch-case
    while (programme_actif) {
        
        switch (etat_actuel) {
            
            case ETAT_ACCUEIL:
                printf("\n--- État: ACCUEIL ---\n");
                afficherEcranAccueil(renderer);
                etat_actuel = ETAT_CHARGEMENT;
                break;
                
            case ETAT_CHARGEMENT:
                printf("\n--- État: CHARGEMENT ---\n");
                printf("Chargement des ressources...\n");
                // Ici on pourrait charger des sons, des images, etc.
                SDL_Delay(1000); // Simule un chargement
                etat_actuel = ETAT_QUIZ;
                break;
                
            case ETAT_QUIZ:
                printf("\n--- État: QUIZ ---\n");
                printf("Lancement du quiz...\n");
                resultat_quiz = runEnigme(renderer);
                
                // Simuler un score (à remplacer par le vrai score retourné par runEnigme)
                score_final = resultat_quiz * 100;
                
                etat_actuel = ETAT_RESULTATS;
                break;
                
            case ETAT_RESULTATS:
                printf("\n--- État: RESULTATS ---\n");
                afficherResultats(renderer, score_final);
                etat_actuel = ETAT_QUITTER;
                break;
                
            case ETAT_QUITTER:
                printf("\n--- État: QUITTER ---\n");
                printf("Fermeture du jeu...\n");
                programme_actif = 0;
                break;
                
            case ETAT_ERREUR:
                printf("\n--- État: ERREUR ---\n");
                printf("Une erreur est survenue !\n");
                programme_actif = 0;
                break;
                
            default:
                printf("État inconnu !\n");
                programme_actif = 0;
                break;
        }
    }
    
    cleanup(window, renderer);
    
    if (resultat_quiz) {
        printf("\nQuiz terminé avec succès\n");
        printf("Score final: %d\n", score_final);
    }
    
    return 0;
}

