#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>

#define MAX_QUESTIONS 50
#define MAX_REPONSES 4
#define MAX_TEXTE 512

typedef struct {
    char question[MAX_TEXTE];
    char reponses[MAX_REPONSES][MAX_TEXTE];
    int bonne_reponse;
    int deja_vu;
} Question;

typedef struct {
    int score;
    int vies;
    int questions_posees;
    int temps_restant;
    time_t debut_temps;
    int question_actuelle;
} JeuData;

// Retourne 1 si le quiz est terminé normalement, 0 si quitté
int runEnigme(SDL_Renderer *renderer);

#endif

