#include "joueur.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

#define TARGET_WIDTH 80
#define TARGET_HEIGHT 80

// Better background removal function
SDL_Texture* removeBackground(SDL_Renderer *renderer, const char *path, int targetW, int targetH) {
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        printf("Failed to load: %s\n", path);
        return NULL;
    }
    
    printf("Processing: %s (%dx%d)\n", path, surf->w, surf->h);
    
    // Convert to 32-bit format with alpha channel
    SDL_Surface *withAlpha = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);
    
    if (!withAlpha) return NULL;
    
    // Lock surface for pixel access
    SDL_LockSurface(withAlpha);
    
    Uint32 *pixels = (Uint32*)withAlpha->pixels;
    
    // Get background color from top-left pixel (assume it's the background color)
    Uint8 bgR, bgG, bgB;
    Uint32 bgPixel = pixels[0];
    SDL_GetRGB(bgPixel, withAlpha->format, &bgR, &bgG, &bgB);
    
    printf("Background color: R=%d G=%d B=%d\n", bgR, bgG, bgB);
    
    int transparentCount = 0;
    
    // Make all background pixels transparent
    for (int y = 0; y < withAlpha->h; y++) {
        for (int x = 0; x < withAlpha->w; x++) {
            Uint32 pixel = pixels[y * withAlpha->w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, withAlpha->format, &r, &g, &b);
            
            // If pixel color is close to background color, make it transparent
            int diff = abs(r - bgR) + abs(g - bgG) + abs(b - bgB);
            if (diff < 50) {  // Threshold for background
                pixels[y * withAlpha->w + x] = 0;  // Make transparent
                transparentCount++;
            }
        }
    }
    
    printf("Made %d pixels transparent\n", transparentCount);
    
    SDL_UnlockSurface(withAlpha);
    
    // Resize to target dimensions
    SDL_Surface *resized = SDL_CreateRGBSurfaceWithFormat(0, targetW, targetH, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_BlitScaled(withAlpha, NULL, resized, NULL);
    SDL_FreeSurface(withAlpha);
    
    SDL_SetSurfaceBlendMode(resized, SDL_BLENDMODE_BLEND);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, resized);
    SDL_FreeSurface(resized);
    
    return texture;
}

// Special function for character selection (keep original size, just remove background)
SDL_Texture* removeBackgroundKeepSize(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        printf("Failed to load: %s\n", path);
        return NULL;
    }
    
    printf("Processing selection image: %s (%dx%d)\n", path, surf->w, surf->h);
    
    // Convert to 32-bit format with alpha channel
    SDL_Surface *withAlpha = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);
    
    if (!withAlpha) return NULL;
    
    SDL_LockSurface(withAlpha);
    
    Uint32 *pixels = (Uint32*)withAlpha->pixels;
    
    // Get background color from top-left pixel
    Uint8 bgR, bgG, bgB;
    Uint32 bgPixel = pixels[0];
    SDL_GetRGB(bgPixel, withAlpha->format, &bgR, &bgG, &bgB);
    
    // Make all background pixels transparent
    for (int y = 0; y < withAlpha->h; y++) {
        for (int x = 0; x < withAlpha->w; x++) {
            Uint32 pixel = pixels[y * withAlpha->w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, withAlpha->format, &r, &g, &b);
            
            int diff = abs(r - bgR) + abs(g - bgG) + abs(b - bgB);
            if (diff < 50) {
                pixels[y * withAlpha->w + x] = 0;
            }
        }
    }
    
    SDL_UnlockSurface(withAlpha);
    
    SDL_SetSurfaceBlendMode(withAlpha, SDL_BLENDMODE_BLEND);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, withAlpha);
    SDL_FreeSurface(withAlpha);
    
    return texture;
}

void loadAnimations(Joueur *j, SDL_Renderer *renderer, int playerNum) {
    char path[256];
    
    printf("\n========================================\n");
    printf("Loading Player %d animations...\n", playerNum);
    printf("========================================\n");
    
    j->w = TARGET_WIDTH;
    j->h = TARGET_HEIGHT;
    
    if (playerNum == 1) {
        // Load idle texture
        sprintf(path, "images/player1_clean.png");
        j->idleTexture = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
        
        // Load walk frames
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player1_anim/walk%d_p1.png", i);
            j->walkFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->walkFrames[i-1]) j->walkFrames[i-1] = j->idleTexture;
        }
        
        // Load run frames
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player1_anim/run%d_p1.png", i);
            j->runFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->runFrames[i-1]) j->runFrames[i-1] = j->idleTexture;
        }
        
        // Load jump frames
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player1_anim/jump%d_p1.png", i);
            j->jumpFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->jumpFrames[i-1]) j->jumpFrames[i-1] = j->idleTexture;
        }
        
        // Load attack frames
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player1_anim/attack%d_p1.png", i);
            j->attackFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->attackFrames[i-1]) j->attackFrames[i-1] = j->idleTexture;
        }
        
        // Load dead frames
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player1_anim/dead%d_p1.png", i);
            j->deadFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->deadFrames[i-1]) j->deadFrames[i-1] = j->idleTexture;
        }
        j->numFrames = 5;
        
    } else {
        // Player 2
        sprintf(path, "images/player2_clean.png");
        j->idleTexture = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
        
        // Walk frames (5)
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player2_anim/walkp2_%d.jpg", i);
            j->walkFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->walkFrames[i-1]) j->walkFrames[i-1] = j->idleTexture;
        }
        
        // Run frames (9)
        for (int i = 1; i <= 9; i++) {
            sprintf(path, "player2_anim/runp2_%d.jpg", i);
            j->runFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->runFrames[i-1]) j->runFrames[i-1] = j->idleTexture;
        }
        
        // Jump frames (8)
        for (int i = 1; i <= 8; i++) {
            sprintf(path, "player2_anim/jumpp2_%d.jpg", i);
            j->jumpFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->jumpFrames[i-1]) j->jumpFrames[i-1] = j->idleTexture;
        }
        
        // Attack frames (5)
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player2_anim/attackp2_%d.jpg", i);
            j->attackFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->attackFrames[i-1]) j->attackFrames[i-1] = j->idleTexture;
        }
        
        // Dead frames (5)
        for (int i = 1; i <= 5; i++) {
            sprintf(path, "player2_anim/deadp2_%d.jpg", i);
            j->deadFrames[i-1] = removeBackground(renderer, path, TARGET_WIDTH, TARGET_HEIGHT);
            if (!j->deadFrames[i-1]) j->deadFrames[i-1] = j->idleTexture;
        }
        j->numFrames = 5;
    }
    
    printf("✓ Player %d animations loaded! Size: %dx%d\n", playerNum, j->w, j->h);
}

void initJoueur(Joueur *j, SDL_Renderer *renderer, int playerNum, int x, int y) {
    printf("\nInitializing Player %d...\n", playerNum);
    
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
    
    loadAnimations(j, renderer, playerNum);
    
    j->pos.x = x;
    j->pos.y = y - j->h;
    j->pos.w = j->w;
    j->pos.h = j->h;
    
    printf("Player %d ready at (%d, %d) size %dx%d\n", playerNum, j->pos.x, j->pos.y, j->w, j->h);
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
    float speed = 250.0f * dt, runSpeed = 400.0f * dt;
    if (!ks || j->isDead) return;
    
    if (j->attackTimer > 0) {
        j->attackTimer--;
        if (j->attackTimer == 0) j->isAttacking = 0;
    }
    
    j->isMoving = 0;
    j->isRunning = 0;
    
    if (j->playerNum == 1) {
        if (ks[SDL_SCANCODE_RIGHT]) {
            j->pos.x += (ks[SDL_SCANCODE_RSHIFT] ? runSpeed : speed);
            j->isFacingRight = 1;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_RSHIFT];
        }
        if (ks[SDL_SCANCODE_LEFT]) {
            j->pos.x -= (ks[SDL_SCANCODE_RSHIFT] ? runSpeed : speed);
            j->isFacingRight = 0;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_RSHIFT];
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP && j->isGrounded && !j->isAttacking) {
            j->velocityY = -9.0f; j->isGrounded = 0; setAnimation(j, "jump");
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_PERIOD && !j->isAttacking && j->attackTimer == 0) {
            j->isAttacking = 1; j->attackTimer = 15; setAnimation(j, "attack"); addScore(j, 10);
        }
    } else {
        if (ks[SDL_SCANCODE_D]) {
            j->pos.x += (ks[SDL_SCANCODE_LSHIFT] ? runSpeed : speed);
            j->isFacingRight = 1;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_LSHIFT];
        }
        if (ks[SDL_SCANCODE_A]) {
            j->pos.x -= (ks[SDL_SCANCODE_LSHIFT] ? runSpeed : speed);
            j->isFacingRight = 0;
            j->isMoving = 1;
            j->isRunning = ks[SDL_SCANCODE_LSHIFT];
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w && j->isGrounded && !j->isAttacking) {
            j->velocityY = -9.0f; j->isGrounded = 0; setAnimation(j, "jump");
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_COMMA && !j->isAttacking && j->attackTimer == 0) {
            j->isAttacking = 1; j->attackTimer = 15; setAnimation(j, "attack"); addScore(j, 10);
        }
    }
    
    if (!j->isAttacking && j->isGrounded) {
        if (j->isMoving && j->isRunning) setAnimation(j, "run");
        else if (j->isMoving) setAnimation(j, "walk");
        else setAnimation(j, "idle");
    }
    
    updateAnimation(j);
    j->velocityY += 0.6f;
    j->pos.y += j->velocityY;
    
    int groundLevel = 500 - j->h;
    if (j->pos.y >= groundLevel) { j->pos.y = groundLevel; j->velocityY = 0; j->isGrounded = 1; }
}

void addScore(Joueur *j, int points) { j->score += points; }
void removeLife(Joueur *j) { j->lives--; if (j->lives <= 0) setAnimation(j, "dead"); }
void resetPlayer(Joueur *j, int x, int y) { j->pos.x = x; j->pos.y = y - j->h; j->velocityY = 0; j->isGrounded = 1; j->score = 0; j->lives = 3; j->isDead = 0; j->isAttacking = 0; j->attackTimer = 0; setAnimation(j, "idle"); }
void customizePlayer(Joueur *j, char* costume, SDL_Color color) { strcpy(j->costume, costume); j->color = color; }
