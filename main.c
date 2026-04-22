#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include "joueur.h"
#include "menu.h"

SDL_Texture *gameBackground = NULL;
SDL_Texture *gameBackgroundFlipped = NULL;
SDL_Texture *characterSelectionBg = NULL;
SDL_Texture *modeSelectionBg = NULL;
SDL_Texture *characterPreview1 = NULL;
SDL_Texture *characterPreview2 = NULL;
SDL_Texture *button1 = NULL;
SDL_Texture *button2 = NULL;
SDL_Texture *singleplayerButton = NULL;
SDL_Texture *multiplayerButton = NULL;

TTF_Font *font = NULL;

int isMultiplayer = 0;
Uint32 gameStartTime = 0;
int bgOffset1 = 0;
int bgOffset2 = 0;

int bgWidth = 0;
int bgHeight = 0;

// Function to remove background from selection images
SDL_Texture* loadCleanTexture(SDL_Renderer *renderer, const char *path) {
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
    Uint8 bgR, bgG, bgB;
    Uint32 bgPixel = pixels[0];
    SDL_GetRGB(bgPixel, withAlpha->format, &bgR, &bgG, &bgB);
    
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

void renderText(SDL_Renderer *renderer, const char *text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderModeSelection(SDL_Renderer *renderer) {
    if (modeSelectionBg) {
        SDL_RenderCopy(renderer, modeSelectionBg, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 30, 30, 50, 255);
        SDL_RenderClear(renderer);
    }
    SDL_Rect singleRect = {200, 250, 400, 80};
    SDL_Rect multiRect = {200, 380, 400, 80};
    if (singleplayerButton) SDL_RenderCopy(renderer, singleplayerButton, NULL, &singleRect);
    if (multiplayerButton) SDL_RenderCopy(renderer, multiplayerButton, NULL, &multiRect);
}

int handleModeSelection(SDL_Event e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int x = e.button.x, y = e.button.y;
        if (x >= 200 && x <= 600 && y >= 250 && y <= 330) return 1;
        if (x >= 200 && x <= 600 && y >= 380 && y <= 460) return 2;
    }
    return 0;
}

void renderCharacterSelection(SDL_Renderer *renderer) {
    if (characterSelectionBg) {
        SDL_RenderCopy(renderer, characterSelectionBg, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);
    }
    
    SDL_Rect p1Rect = {150, 150, 200, 200};
    SDL_Rect p1ButtonRect = {175, 380, 150, 60};
    SDL_Rect p2Rect = {450, 150, 200, 200};
    SDL_Rect p2ButtonRect = {475, 380, 150, 60};
    
    if (characterPreview1) {
        SDL_RenderCopy(renderer, characterPreview1, NULL, &p1Rect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        SDL_RenderFillRect(renderer, &p1Rect);
    }
    
    if (characterPreview2) {
        SDL_RenderCopy(renderer, characterPreview2, NULL, &p2Rect);
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
        SDL_RenderFillRect(renderer, &p2Rect);
    }
    
    if (button1) SDL_RenderCopy(renderer, button1, NULL, &p1ButtonRect);
    if (button2) SDL_RenderCopy(renderer, button2, NULL, &p2ButtonRect);
}

int handleCharacterSelection(SDL_Event e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int x = e.button.x, y = e.button.y;
        if (x >= 175 && x <= 325 && y >= 380 && y <= 440) return 1;
        if (x >= 475 && x <= 625 && y >= 380 && y <= 440) return 2;
    }
    return 0;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) return 1;
    if (TTF_Init() < 0) printf("TTF initialization failed\n");

    SDL_Window *win = SDL_CreateWindow("Dreamcore - Lucid Dreams",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    if (!win) return 1;

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return 1;
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf", 24);
    if (!font) printf("Font not found\n");

    // Load background
    SDL_Surface *surf;
    surf = IMG_Load("images/game_background.png");
    if (surf) {
        bgWidth = surf->w;
        bgHeight = surf->h;
        gameBackground = SDL_CreateTextureFromSurface(renderer, surf);
        
        // Create flipped version
        gameBackgroundFlipped = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bgWidth, bgHeight);
        SDL_SetRenderTarget(renderer, gameBackgroundFlipped);
        SDL_RenderCopyEx(renderer, gameBackground, NULL, NULL, 0, NULL, SDL_FLIP_HORIZONTAL);
        SDL_SetRenderTarget(renderer, NULL);
        
        SDL_FreeSurface(surf);
        printf("Background loaded: %dx%d\n", bgWidth, bgHeight);
    }
    
    // Load selection screen images with background removal
    surf = IMG_Load("images/mode_selection.png");
    if (surf) { modeSelectionBg = SDL_CreateTextureFromSurface(renderer, surf); SDL_FreeSurface(surf); }
    
    surf = IMG_Load("images/background_playerselection.png");
    if (surf) { characterSelectionBg = SDL_CreateTextureFromSurface(renderer, surf); SDL_FreeSurface(surf); }
    
    // Load character images with background removal
    characterPreview1 = loadCleanTexture(renderer, "images/player1.png");
    characterPreview2 = loadCleanTexture(renderer, "images/player2.png");
    
    surf = IMG_Load("images/player1_button.png");
    if (surf) { button1 = SDL_CreateTextureFromSurface(renderer, surf); SDL_FreeSurface(surf); }
    
    surf = IMG_Load("images/player2_button.png");
    if (surf) { button2 = SDL_CreateTextureFromSurface(renderer, surf); SDL_FreeSurface(surf); }
    
    surf = IMG_Load("images/singleplayer_button.png");
    if (surf) { singleplayerButton = SDL_CreateTextureFromSurface(renderer, surf); SDL_FreeSurface(surf); }
    
    surf = IMG_Load("images/multiplayer_button.png");
    if (surf) { multiplayerButton = SDL_CreateTextureFromSurface(renderer, surf); SDL_FreeSurface(surf); }

    SDL_Event e;
    int running = 1, inMenu = 1, inModeSelect = 0, inCharacterSelect = 0, gameRunning = 0;
    int lastTime = SDL_GetTicks();
    
    Menu menu;
    initMenu(&menu, renderer);
    
    Joueur player1, player2;
    
    printf("\n========================================\n");
    printf("   DREAMCORE - LUCID DREAMS\n");
    printf("========================================\n");
    printf("Player 1: Arrow Keys, Right Shift (run), Up (jump), . (attack)\n");
    printf("Player 2: WASD, Left Shift (run), W (jump), , (attack)\n");
    printf("Press ESC to return to menu\n");

    while (running) {
        int currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 16.0f;
        if (deltaTime > 1.0f) deltaTime = 1.0f;
        lastTime = currentTime;
        
        if (gameRunning) {
            if (isMultiplayer) {
                if (player1.isMoving) {
                    int speed = player1.isRunning ? 5 : 3;
                    if (player1.isFacingRight) {
                        bgOffset1 += speed;
                    } else {
                        bgOffset1 -= speed;
                    }
                }
                while (bgOffset1 >= bgWidth * 2) bgOffset1 -= bgWidth * 2;
                while (bgOffset1 < 0) bgOffset1 += bgWidth * 2;
                
                if (player2.isMoving) {
                    int speed = player2.isRunning ? 5 : 3;
                    if (player2.isFacingRight) {
                        bgOffset2 += speed;
                    } else {
                        bgOffset2 -= speed;
                    }
                }
                while (bgOffset2 >= bgWidth * 2) bgOffset2 -= bgWidth * 2;
                while (bgOffset2 < 0) bgOffset2 += bgWidth * 2;
            } else {
                if (player1.isMoving) {
                    int speed = player1.isRunning ? 5 : 3;
                    if (player1.isFacingRight) {
                        bgOffset1 += speed;
                    } else {
                        bgOffset1 -= speed;
                    }
                }
                while (bgOffset1 >= bgWidth * 2) bgOffset1 -= bgWidth * 2;
                while (bgOffset1 < 0) bgOffset1 += bgWidth * 2;
            }
        }
        
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            
            if (inMenu) {
                int r = handleMenu(menu, e);
                if (r == 1) { inMenu = 0; inModeSelect = 1; }
                if (r == -1) running = 0;
            } 
            else if (inModeSelect) {
                int mode = handleModeSelection(e);
                if (mode == 1) {
                    inModeSelect = 0;
                    inCharacterSelect = 1;
                    isMultiplayer = 0;
                }
                else if (mode == 2) {
                    inModeSelect = 0;
                    gameRunning = 1;
                    isMultiplayer = 1;
                    gameStartTime = SDL_GetTicks();
                    bgOffset1 = 0;
                    bgOffset2 = 0;
                    initJoueur(&player1, renderer, 1, 150, 500);
                    initJoueur(&player2, renderer, 2, 600, 500);
                }
            }
            else if (inCharacterSelect) {
                int selected = handleCharacterSelection(e);
                if (selected == 1 || selected == 2) {
                    inCharacterSelect = 0;
                    gameRunning = 1;
                    gameStartTime = SDL_GetTicks();
                    bgOffset1 = 0;
                    initJoueur(&player1, renderer, selected, 400, 500);
                }
            }
            else if (gameRunning) {
                const Uint8* ks = SDL_GetKeyboardState(NULL);
                if (isMultiplayer) {
                    deplacerJoueur(&player1, e, ks, deltaTime);
                    deplacerJoueur(&player2, e, ks, deltaTime);
                    
                    if (player1.pos.x + player1.w > 390) player1.pos.x = 390 - player1.w;
                    if (player1.pos.x < 0) player1.pos.x = 0;
                    if (player2.pos.x < 410) player2.pos.x = 410;
                    if (player2.pos.x + player2.w > 800) player2.pos.x = 800 - player2.w;
                } else {
                    deplacerJoueur(&player1, e, ks, deltaTime);
                    if (player1.pos.x < 0) player1.pos.x = 0;
                    if (player1.pos.x > 800 - player1.w) player1.pos.x = 800 - player1.w;
                }
                
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    inMenu = 1; gameRunning = 0; inModeSelect = 0;
                    inCharacterSelect = 0; isMultiplayer = 0; 
                    bgOffset1 = 0; bgOffset2 = 0;
                }
            }
        }

        SDL_RenderClear(renderer);

        if (inMenu) {
            afficherMenu(menu, renderer);
        } 
        else if (inModeSelect) {
            renderModeSelection(renderer);
        }
        else if (inCharacterSelect) {
            renderCharacterSelection(renderer);
        }
        else if (gameRunning) {
            Uint32 elapsed = (SDL_GetTicks() - gameStartTime) / 1000;
            int minutes = elapsed / 60;
            int seconds = elapsed % 60;
            char timeStr[20];
            sprintf(timeStr, "%02d:%02d", minutes, seconds);
            
            SDL_Color white = {255, 255, 255, 255};
            SDL_Color red = {255, 100, 100, 255};
            SDL_Color green = {100, 255, 100, 255};
            
            if (isMultiplayer) {
                // LEFT SIDE - Player 1 Background
                if (gameBackground) {
                    int offset = bgOffset1;
                    int segment = (offset / bgWidth) % 2;
                    int localOffset = offset % bgWidth;
                    
                    SDL_Texture *currentTex = (segment == 0) ? gameBackground : gameBackgroundFlipped;
                    
                    int drawWidth = bgWidth - localOffset;
                    if (drawWidth > 400) drawWidth = 400;
                    
                    SDL_Rect src1 = {localOffset, 0, drawWidth, bgHeight};
                    SDL_Rect dst1 = {0, 0, drawWidth, 600};
                    SDL_RenderCopy(renderer, currentTex, &src1, &dst1);
                    
                    if (drawWidth < 400) {
                        int remaining = 400 - drawWidth;
                        int nextSegment = (segment + 1) % 2;
                        SDL_Texture *nextTex = (nextSegment == 0) ? gameBackground : gameBackgroundFlipped;
                        SDL_Rect src2 = {0, 0, remaining, bgHeight};
                        SDL_Rect dst2 = {drawWidth, 0, remaining, 600};
                        SDL_RenderCopy(renderer, nextTex, &src2, &dst2);
                    }
                }
                
                // RIGHT SIDE - Player 2 Background
                if (gameBackground) {
                    int offset = bgOffset2;
                    int segment = (offset / bgWidth) % 2;
                    int localOffset = offset % bgWidth;
                    
                    SDL_Texture *currentTex = (segment == 0) ? gameBackground : gameBackgroundFlipped;
                    
                    int drawWidth = bgWidth - localOffset;
                    if (drawWidth > 400) drawWidth = 400;
                    
                    SDL_Rect src3 = {localOffset, 0, drawWidth, bgHeight};
                    SDL_Rect dst3 = {400, 0, drawWidth, 600};
                    SDL_RenderCopy(renderer, currentTex, &src3, &dst3);
                    
                    if (drawWidth < 400) {
                        int remaining = 400 - drawWidth;
                        int nextSegment = (segment + 1) % 2;
                        SDL_Texture *nextTex = (nextSegment == 0) ? gameBackground : gameBackgroundFlipped;
                        SDL_Rect src4 = {0, 0, remaining, bgHeight};
                        SDL_Rect dst4 = {400 + drawWidth, 0, remaining, 600};
                        SDL_RenderCopy(renderer, nextTex, &src4, &dst4);
                    }
                }
                
                // Draw dividing line
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                for (int i = 0; i < 5; i++) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50 - i * 10);
                    SDL_RenderDrawLine(renderer, 400 - i, 0, 400 - i, 600);
                    SDL_RenderDrawLine(renderer, 400 + i, 0, 400 + i, 600);
                }
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(renderer, 400, 0, 400, 600);
                
                afficherJoueur(player1, renderer);
                afficherJoueur(player2, renderer);
                
                char p1Score[50];
                sprintf(p1Score, "Score: %d", player1.score);
                
                SDL_Rect p1Bg = {10, 10, 180, 100};
                SDL_Rect p2Bg = {610, 10, 180, 100};
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
                SDL_RenderFillRect(renderer, &p1Bg);
                SDL_RenderFillRect(renderer, &p2Bg);
                
                if (font) {
                    renderText(renderer, "PLAYER 1", 20, 15, red);
                    renderText(renderer, p1Score, 20, 45, white);
                    renderText(renderer, timeStr, 20, 75, white);
                    
                    renderText(renderer, "PLAYER 2", 620, 15, green);
                    sprintf(p1Score, "Score: %d", player2.score);
                    renderText(renderer, p1Score, 620, 45, white);
                    renderText(renderer, timeStr, 620, 75, white);
                }
                
                float p1Life = (float)player1.lives / 3.0f;
                float p2Life = (float)player2.lives / 3.0f;
                SDL_Rect p1Health = {10, 95, (int)(180 * p1Life), 10};
                SDL_Rect p2Health = {610, 95, (int)(180 * p2Life), 10};
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderFillRect(renderer, &p1Health);
                SDL_RenderFillRect(renderer, &p2Health);
                
                printf("\rP1: %3d pts %d lives | P2: %3d pts %d lives | Time: %02d:%02d", 
                       player1.score, player1.lives, player2.score, player2.lives, minutes, seconds);
                fflush(stdout);
            } else {
                // SINGLEPLAYER
                if (gameBackground) {
                    int offset = bgOffset1;
                    int segment = (offset / bgWidth) % 2;
                    int localOffset = offset % bgWidth;
                    
                    SDL_Texture *currentTex = (segment == 0) ? gameBackground : gameBackgroundFlipped;
                    
                    int drawWidth = bgWidth - localOffset;
                    if (drawWidth > 800) drawWidth = 800;
                    
                    SDL_Rect src1 = {localOffset, 0, drawWidth, bgHeight};
                    SDL_Rect dst1 = {0, 0, drawWidth, 600};
                    SDL_RenderCopy(renderer, currentTex, &src1, &dst1);
                    
                    if (drawWidth < 800) {
                        int remaining = 800 - drawWidth;
                        int nextSegment = (segment + 1) % 2;
                        SDL_Texture *nextTex = (nextSegment == 0) ? gameBackground : gameBackgroundFlipped;
                        SDL_Rect src2 = {0, 0, remaining, bgHeight};
                        SDL_Rect dst2 = {drawWidth, 0, remaining, 600};
                        SDL_RenderCopy(renderer, nextTex, &src2, &dst2);
                    }
                }
                
                afficherJoueur(player1, renderer);
                
                char scoreText[50], livesText[50];
                sprintf(scoreText, "Score: %d", player1.score);
                sprintf(livesText, "Lives: %d", player1.lives);
                
                SDL_Rect uiBg = {10, 10, 180, 100};
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
                SDL_RenderFillRect(renderer, &uiBg);
                
                if (font) {
                    renderText(renderer, scoreText, 20, 20, white);
                    renderText(renderer, livesText, 20, 50, white);
                    renderText(renderer, timeStr, 20, 75, white);
                }
                
                float lifePercent = (float)player1.lives / 3.0f;
                SDL_Rect healthBar = {10, 95, (int)(180 * lifePercent), 10};
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderFillRect(renderer, &healthBar);
                
                printf("\rScore: %3d | Lives: %d | Time: %02d:%02d", 
                       player1.score, player1.lives, minutes, seconds);
                fflush(stdout);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (font) TTF_CloseFont(font);
    TTF_Quit();
    if (gameBackground) SDL_DestroyTexture(gameBackground);
    if (gameBackgroundFlipped) SDL_DestroyTexture(gameBackgroundFlipped);
    if (modeSelectionBg) SDL_DestroyTexture(modeSelectionBg);
    if (characterSelectionBg) SDL_DestroyTexture(characterSelectionBg);
    if (characterPreview1) SDL_DestroyTexture(characterPreview1);
    if (characterPreview2) SDL_DestroyTexture(characterPreview2);
    if (button1) SDL_DestroyTexture(button1);
    if (button2) SDL_DestroyTexture(button2);
    if (singleplayerButton) SDL_DestroyTexture(singleplayerButton);
    if (multiplayerButton) SDL_DestroyTexture(multiplayerButton);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
