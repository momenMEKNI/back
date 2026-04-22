#include "menu.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

void initMenu(Menu *m, SDL_Renderer *renderer) {
    SDL_Surface *surf;
    
    printf("Loading menu images from images folder...\n");
    
    // Load background
    surf = IMG_Load("images/background.png");
    if (surf) {
        m->bg = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        printf("Loaded background.png\n");
    } else {
        printf("Failed to load images/background.png\n");
        m->bg = NULL;
    }
    
    // Load play button
    surf = IMG_Load("images/play.png");
    if (surf) {
        m->play = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        printf("Loaded play.png\n");
    } else {
        printf("Failed to load images/play.png\n");
        m->play = NULL;
    }
    
    // Load quit button
    surf = IMG_Load("images/quit.png");
    if (surf) {
        m->quit = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        printf("Loaded quit.png\n");
    } else {
        printf("Failed to load images/quit.png\n");
        m->quit = NULL;
    }
    
    // Set button positions
    if (m->play) {
        int w, h;
        SDL_QueryTexture(m->play, NULL, NULL, &w, &h);
        m->posPlay = (SDL_Rect){400 - w/2, 250, w, h};
        printf("Play button at: %d, %d size: %dx%d\n", m->posPlay.x, m->posPlay.y, w, h);
    } else {
        m->posPlay = (SDL_Rect){300, 250, 200, 80};
    }
    
    if (m->quit) {
        int w, h;
        SDL_QueryTexture(m->quit, NULL, NULL, &w, &h);
        m->posQuit = (SDL_Rect){400 - w/2, 350, w, h};
        printf("Quit button at: %d, %d size: %dx%d\n", m->posQuit.x, m->posQuit.y, w, h);
    } else {
        m->posQuit = (SDL_Rect){300, 350, 200, 80};
    }
    
    printf("Menu initialized\n");
}

void afficherMenu(Menu m, SDL_Renderer *renderer) {
    // Draw background
    if (m.bg) {
        SDL_RenderCopy(renderer, m.bg, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255);
        SDL_RenderClear(renderer);
    }
    
    // Draw play button
    if (m.play) {
        SDL_RenderCopy(renderer, m.play, NULL, &m.posPlay);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &m.posPlay);
    }
    
    // Draw quit button
    if (m.quit) {
        SDL_RenderCopy(renderer, m.quit, NULL, &m.posQuit);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &m.posQuit);
    }
}

int handleMenu(Menu m, SDL_Event e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int x = e.button.x;
        int y = e.button.y;
        
        // Check play button
        if (x >= m.posPlay.x && x <= m.posPlay.x + m.posPlay.w &&
            y >= m.posPlay.y && y <= m.posPlay.y + m.posPlay.h) {
            printf("Play button clicked!\n");
            return 1;
        }
        
        // Check quit button
        if (x >= m.posQuit.x && x <= m.posQuit.x + m.posQuit.w &&
            y >= m.posQuit.y && y <= m.posQuit.y + m.posQuit.h) {
            printf("Quit button clicked!\n");
            return -1;
        }
    }
    return 0;
}
