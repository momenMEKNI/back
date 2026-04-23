#include "joueur.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

#define TARGET_WIDTH 80
#define TARGET_HEIGHT 80
#define MOVE_STEP 8

SDL_Texture* removeWhiteBackground(SDL_Renderer *renderer, const char *path, int targetW, int targetH) {
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        printf("Failed to load: %s\n", path);
        return NULL;
    }
    
    SDL_Surface *withAlpha = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);
    if (!withAlpha) return NULL;
    
    SDL_LockSurface(withAlpha);
    Uint32 *pixels = (Uint32*)withAlpha->pixels;
    
    for (int y = 0; y < withAlpha->h; y++) {
        for (int x = 0; x < withAlpha->w; x++) {
            Uint32 pixel = pixels[y * withAlpha->w + x];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, withAlpha->format, &r, &g, &b, &a);
            if (r > 200 && g > 200 && b > 200) {
                pixels[y * withAlpha->w + x] = 0;
            }
        }
    }
    
    SDL_UnlockSurface(withAlpha);
    
    SDL_Surface *resized = SDL_CreateRGBSurfaceWithFormat(0, targetW, targetH, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_BlitScaled(withAlpha, NULL, resized, NULL);
    SDL_FreeSurface(withAlpha);
    
    SDL_SetSurfaceBlendMode(resized, SDL_BLENDMODE_BLEND);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, resized);
    SDL_FreeSurface(resized);
    
    return texture;
}

void loadAnimations(Joueur *j, SDL_Renderer *renderer, int playerNum) {
    char path[256];
    
    j->w = TARGET_WIDTH;
    j->h = TARGET_HEIGHT;
    
    if (playerNum == 1) {
        sprintf(path, "images/player1_clean.png");
        j->idleTexture = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
        
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player1_anim/walk%d_p1.png", i);
            j->walkFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->walkFrames[i-1]) j->walkFrames[i-1] = j->idleTexture;
            
            sprintf(path, "player1_anim/run%d_p1.png", i);
            j->runFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->runFrames[i-1]) j->runFrames[i-1] = j->idleTexture;
            
            sprintf(path, "player1_anim/jump%d_p1.png", i);
            j->jumpFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->jumpFrames[i-1]) j->jumpFrames[i-1] = j->idleTexture;
            
            sprintf(path, "player1_anim/attack%d_p1.png", i);
            j->attackFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->attackFrames[i-1]) j->attackFrames[i-1] = j->idleTexture;
            
            sprintf(path, "player1_anim/dead%d_p1.png", i);
            j->deadFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->deadFrames[i-1]) j->deadFrames[i-1] = j->idleTexture;
        }
        j->numFrames = 5;
    } else {
        sprintf(path, "images/player2_clean.png");
        j->idleTexture = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
        
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player2_anim/walkp2_%d.jpg", i);
            j->walkFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->walkFrames[i-1]) j->walkFrames[i-1] = j->idleTexture;
        }
        for (int i = 1; i <= 9; i++) {
            sprintf(path, "player2_anim/runp2_%d.jpg", i);
            j->runFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->runFrames[i-1]) j->runFrames[i-1] = j->idleTexture;
        }
        for (int i = 1; i <= 8; i++) {
            sprintf(path, "player2_anim/jumpp2_%d.jpg", i);
            j->jumpFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->jumpFrames[i-1]) j->jumpFrames[i-1] = j->idleTexture;
        }
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player2_anim/attackp2_%d.jpg", i);
            j->attackFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->attackFrames[i-1]) j->attackFrames[i-1] = j->idleTexture;
            
            sprintf(path, "player2_anim/deadp2_%d.jpg", i);
            j->deadFrames[i-1] = removeWhiteBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->deadFrames[i-1]) j->deadFrames[i-1] = j->idleTexture;
        }
        j->numFrames = 5;
    }
}

void initJoueur(Joueur *j, SDL_Renderer *renderer, int playerNum, int x, int y) {
    j->playerNum = playerNum;
    j->score = 0;
    j->lives = 3;
    j->isFacingRight = 1;
    j->isAttacking = 0;
    j->attackTimer = 0;
    j->isDead = 0;
    j->velocityY = 0;
    j->isGrounded = 1;
    j->isMoving = 0;
    j->isRunning = 0;
    j->currentFrame = 0;
    j->frameDelay = 5;
    j->frameTimer = 0;
    
    // Track key states
    j->rightPressed = 0;
    j->leftPressed = 0;
    
    loadAnimations(j, renderer, playerNum);
    
    j->pos.x = x;
    j->pos.y = y - j->h;
    j->pos.w = j->w;
    j->pos.h = j->h;
}

void setAnimation(Joueur *j, const char* animName) {
    if (strcmp(j->currentAnim, animName) != 0) {
        strcpy(j->currentAnim, animName);
        j->currentFrame = 0;
        j->frameTimer = 0;
        
        if (strcmp(animName, "dead") == 0) { j->numFrames = 5; j->frameDelay = 8; }
        else if (strcmp(animName, "attack") == 0) { j->numFrames = 5; j->frameDelay = 3; }
        else if (strcmp(animName, "jump") == 0) { j->numFrames = (j->playerNum == 2) ? 8 : 5; j->frameDelay = 4; }
        else if (strcmp(animName, "run") == 0) { j->numFrames = (j->playerNum == 2) ? 9 : 5; j->frameDelay = 3; }
        else if (strcmp(animName, "walk") == 0) { j->numFrames = 5; j->frameDelay = 5; }
        else { j->numFrames = 5; j->frameDelay = 5; }
    }
}

void updateAnimation(Joueur *j) {
    if (j->isDead) return;
    
    j->frameTimer++;
    if (j->frameTimer >= j->frameDelay) {
        j->frameTimer = 0;
        j->currentFrame++;
        if (j->currentFrame >= j->numFrames) {
            j->currentFrame = 0;
            if (strcmp(j->currentAnim, "attack") == 0) {
                j->isAttacking = 0;
                setAnimation(j, "idle");
            }
            if (strcmp(j->currentAnim, "dead") == 0) j->isDead = 1;
        }
    }
}

void afficherJoueur(Joueur j, SDL_Renderer *renderer) {
    SDL_Rect renderRect = j.pos;
    SDL_Texture *currentTex = NULL;
    
    if (j.isDead) currentTex = j.deadFrames[j.currentFrame];
    else if (strcmp(j.currentAnim, "walk") == 0) currentTex = j.walkFrames[j.currentFrame];
    else if (strcmp(j.currentAnim, "run") == 0) currentTex = j.runFrames[j.currentFrame];
    else if (strcmp(j.currentAnim, "jump") == 0) currentTex = j.jumpFrames[j.currentFrame];
    else if (strcmp(j.currentAnim, "attack") == 0) currentTex = j.attackFrames[j.currentFrame];
    else currentTex = j.idleTexture;
    
    if (currentTex) {
        SDL_SetTextureBlendMode(currentTex, SDL_BLENDMODE_BLEND);
        if (j.isFacingRight) SDL_RenderCopy(renderer, currentTex, NULL, &renderRect);
        else SDL_RenderCopyEx(renderer, currentTex, NULL, &renderRect, 0, NULL, SDL_FLIP_HORIZONTAL);
    }
}

void deplacerJoueur(Joueur *j, SDL_Event e, const Uint8* ks, float dt) {
    if (!ks || j->isDead) return;
    
    // Handle attack timer
    if (j->attackTimer > 0) {
        j->attackTimer--;
        if (j->attackTimer == 0) j->isAttacking = 0;
    }
    
    // Update key states
    if (j->playerNum == 1) {
        j->rightPressed = ks[SDL_SCANCODE_RIGHT];
        j->leftPressed = ks[SDL_SCANCODE_LEFT];
    } else {
        j->rightPressed = ks[SDL_SCANCODE_D];
        j->leftPressed = ks[SDL_SCANCODE_A];
    }
    
    j->isMoving = 0;
    j->isRunning = 0;
    
    if (j->playerNum == 1) {
        // DOWN ARROW (SDLK_DOWN) - Reset player to idle/standing position
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN) {
            // Stop movement
            j->rightPressed = 0;
            j->leftPressed = 0;
            j->isMoving = 0;
            j->isRunning = 0;
            // Reset to idle animation
            setAnimation(j, "idle");
            // Reset velocity
            j->velocityY = 0;
            // Ensure grounded
            int groundLevel = 500 - j->h;
            if (j->pos.y < groundLevel) {
                j->pos.y = groundLevel;
            }
            printf("Player 1 returned to idle status\n");
        }
        
        // Movement while key is held
        if (j->rightPressed) {
            j->pos.x += MOVE_STEP;
            j->isFacingRight = 1;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_RSHIFT];
        }
        if (j->leftPressed) {
            j->pos.x -= MOVE_STEP;
            j->isFacingRight = 0;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_RSHIFT];
        }
        // Jump on up key press
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP && j->isGrounded && !j->isAttacking) {
            j->velocityY = -12.0f;
            j->isGrounded = 0;
            setAnimation(j, "jump");
        }
        // Attack on period key press
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_PERIOD && !j->isAttacking && j->attackTimer == 0) {
            j->isAttacking = 1;
            j->attackTimer = 15;
            setAnimation(j, "attack");
            addScore(j, 10);
        }
    } else {
        // S KEY (SDLK_s) - Reset player to idle/standing position
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s) {
            // Stop movement
            j->rightPressed = 0;
            j->leftPressed = 0;
            j->isMoving = 0;
            j->isRunning = 0;
            // Reset to idle animation
            setAnimation(j, "idle");
            // Reset velocity
            j->velocityY = 0;
            // Ensure grounded
            int groundLevel = 500 - j->h;
            if (j->pos.y < groundLevel) {
                j->pos.y = groundLevel;
            }
            printf("Player 2 returned to idle status\n");
        }
        
        if (j->rightPressed) {
            j->pos.x += MOVE_STEP;
            j->isFacingRight = 1;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_LSHIFT];
        }
        if (j->leftPressed) {
            j->pos.x -= MOVE_STEP;
            j->isFacingRight = 0;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_LSHIFT];
        }
        // Jump on w key press
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w && j->isGrounded && !j->isAttacking) {
            j->velocityY = -12.0f;
            j->isGrounded = 0;
            setAnimation(j, "jump");
        }
        // Attack on comma key press
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_COMMA && !j->isAttacking && j->attackTimer == 0) {
            j->isAttacking = 1;
            j->attackTimer = 15;
            setAnimation(j, "attack");
            addScore(j, 10);
        }
    }
    
    // Set animation based on movement state
    if (!j->isAttacking && j->isGrounded) {
        if (j->isMoving && j->isRunning) {
            setAnimation(j, "run");
        } else if (j->isMoving) {
            setAnimation(j, "walk");
        } else {
            setAnimation(j, "idle");
        }
    }
    
    updateAnimation(j);
    
    // Apply gravity
    j->velocityY += 0.6f;
    j->pos.y += j->velocityY;
    
    int groundLevel = 500 - j->h;
    if (j->pos.y >= groundLevel) {
        j->pos.y = groundLevel;
        j->velocityY = 0;
        j->isGrounded = 1;
    }
    
    // Keep on screen boundaries
    if (j->pos.x < 0) j->pos.x = 0;
    if (j->pos.x > 800 - j->w) j->pos.x = 800 - j->w;
}

void addScore(Joueur *j, int points) { j->score += points; }
void removeLife(Joueur *j) { j->lives--; if (j->lives <= 0) setAnimation(j, "dead"); }
void resetPlayer(Joueur *j, int x, int y) { j->pos.x = x; j->pos.y = y - j->h; j->velocityY = 0; j->isGrounded = 1; j->score = 0; j->lives = 3; j->isDead = 0; j->isAttacking = 0; j->attackTimer = 0; setAnimation(j, "idle"); }
void customizePlayer(Joueur *j, char* costume, SDL_Color color) { strcpy(j->costume, costume); j->color = color; }
