#ifndef JOUEUR_H
#define JOUEUR_H

#include <SDL2/SDL.h>

#define MAX_FRAMES 15

typedef struct {
    SDL_Rect pos;
    SDL_Texture *walkFrames[MAX_FRAMES];
    SDL_Texture *runFrames[MAX_FRAMES];
    SDL_Texture *jumpFrames[MAX_FRAMES];
    SDL_Texture *attackFrames[MAX_FRAMES];
    SDL_Texture *deadFrames[MAX_FRAMES];
    SDL_Texture *idleTexture;
    
    int w, h, score, lives, playerNum;
    int isFacingRight, isAttacking, attackTimer, isDead;
    int currentFrame, frameDelay, frameTimer, numFrames;
    char currentAnim[20];
    float velocityY;
    int isGrounded, isMoving, isRunning;
    
    int rightPressed;
    int leftPressed;
    
    char costume[50];
    SDL_Color color;
} Joueur;

void initJoueur(Joueur *j, SDL_Renderer *renderer, int playerNum, int x, int y);
void afficherJoueur(Joueur j, SDL_Renderer *renderer);
void deplacerJoueur(Joueur *j, SDL_Event e, const Uint8* keyboardState, float deltaTime);
void setAnimation(Joueur *j, const char* animName);
void addScore(Joueur *j, int points);
void removeLife(Joueur *j);
void resetPlayer(Joueur *j, int x, int y);
void customizePlayer(Joueur *j, char* costume, SDL_Color color);

#endif
