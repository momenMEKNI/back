#include "enigme.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NB_QUESTIONS_QUIZ 10

int chargerQuestions(const char* fichier, Question questions[], int max) {
    FILE* file = fopen(fichier, "r");
    if (!file) return 0;
    
    int count = 0;
    char ligne[MAX_TEXTE];
    
    while (count < max && fgets(ligne, sizeof(ligne), file)) {
        if (ligne[0] == '\n' || ligne[0] == '#') continue;
        ligne[strcspn(ligne, "\n")] = 0;
        strcpy(questions[count].question, ligne);
        
        for (int i = 0; i < MAX_REPONSES; i++) {
            if (fgets(ligne, sizeof(ligne), file)) {
                ligne[strcspn(ligne, "\n")] = 0;
                char *ptr = ligne;
                while (*ptr >= '0' && *ptr <= '9') ptr++;
                while (*ptr == ' ' || *ptr == '.') ptr++;
                strcpy(questions[count].reponses[i], ptr);
            }
        }
        
        if (fgets(ligne, sizeof(ligne), file))
            questions[count].bonne_reponse = atoi(ligne);
        
        questions[count].deja_vu = 0;
        count++;
    }
    fclose(file);
    return count;
}

void melangerQuestions(Question questions[], int nb) {
    for (int i = nb - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Question temp = questions[i];
        questions[i] = questions[j];
        questions[j] = temp;
    }
}

int selectionnerQuestions(Question toutes[], int nb_total, Question selection[], int nb_select) {
    if (nb_total < nb_select) nb_select = nb_total;
    for (int i = 0; i < nb_total; i++) selection[i] = toutes[i];
    melangerQuestions(selection, nb_total);
    for (int i = nb_select; i < nb_total; i++) memset(&selection[i], 0, sizeof(Question));
    return nb_select;
}

int prochaineQuestion(Question questions[], int nb) {
    for (int i = 0; i < nb; i++)
        if (!questions[i].deja_vu) return i;
    return -1;
}

void dessinerHorloge(SDL_Renderer *renderer, int temps_restant, int temps_max, int x, int y, int rayon) {
    float angle = (float)temps_restant / temps_max * 2 * M_PI;
    
    SDL_Color couleur_arc = temps_restant <= 5 ? (SDL_Color){255, 50, 50, 255} :
                           (temps_restant <= 10 ? (SDL_Color){255, 200, 50, 255} :
                                                  (SDL_Color){80, 200, 255, 255});
    
    // Fond gris
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
    for (int w = 0; w < rayon; w++)
        for (int i = -w; i <= w; i++)
            for (int j = -w; j <= w; j++)
                if (i*i + j*j <= rayon*rayon) SDL_RenderDrawPoint(renderer, x + i, y + j);
    
    // Intérieur noir
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    for (int w = 0; w < rayon-5; w++)
        for (int i = -w; i <= w; i++)
            for (int j = -w; j <= w; j++)
                if (i*i + j*j <= (rayon-5)*(rayon-5)) SDL_RenderDrawPoint(renderer, x + i, y + j);
    
    // Secteur coloré
    if (angle > 0) {
        int r = rayon - 5;
        SDL_SetRenderDrawColor(renderer, couleur_arc.r, couleur_arc.g, couleur_arc.b, 200);
        for (float a = 0; a <= angle; a += angle/30.0f) {
            int px = x + (int)(r * sin(a));
            int py = y - (int)(r * cos(a));
            SDL_RenderDrawLine(renderer, x, y, px, py);
        }
    }
}

int runEnigme(SDL_Renderer *renderer) {
    srand(time(NULL));
    
    Question toutes[MAX_QUESTIONS];
    int nb_total = chargerQuestions("questions.txt", toutes, MAX_QUESTIONS);
    if (nb_total == 0) return 0;
    
    Question questions[NB_QUESTIONS_QUIZ];
    int nb_questions = selectionnerQuestions(toutes, nb_total, questions, NB_QUESTIONS_QUIZ);
    
    JeuData jeu = {0, 3, 0, 30, time(NULL), prochaineQuestion(questions, nb_questions)};
    
    SDL_Texture *bg = IMG_LoadTexture(renderer, "images/background.png");
    
    // Chargement polices
    const char* font_paths[] = {"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                                 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                                 "C:\\Windows\\Fonts\\arial.ttf"};
    TTF_Font *font_titre = NULL, *font_question = NULL, *font_reponse = NULL;
    for (int i = 0; i < 3 && (!font_titre || !font_question || !font_reponse); i++) {
        if (!font_titre) font_titre = TTF_OpenFont(font_paths[i], 28);
        if (!font_question) font_question = TTF_OpenFont(font_paths[i], 24);
        if (!font_reponse) font_reponse = TTF_OpenFont(font_paths[i], 22);
    }
    if (!font_titre || !font_question || !font_reponse) return 0;
    
    SDL_Rect zones[4] = {{150, 210, 700, 65}, {150, 285, 700, 65}, {150, 360, 700, 65}, {150, 435, 700, 65}};
    
    int reponse_choisie = -1, afficher_msg = 0, msg_correct = 0, game_over = 0, quit = 0;
    Uint32 temps_msg = 0;
    SDL_Event e;
    
    while (!quit) {
        time_t maintenant = time(NULL);
        jeu.temps_restant = 30 - (int)(maintenant - jeu.debut_temps);
        
        if (jeu.temps_restant <= 0 && !afficher_msg && !game_over && jeu.question_actuelle != -1) {
            jeu.vies--;
            if (jeu.vies <= 0) game_over = 1;
            else {
                questions[jeu.question_actuelle].deja_vu = 1;
                jeu.question_actuelle = prochaineQuestion(questions, nb_questions);
                jeu.debut_temps = time(NULL);
            }
            if (jeu.vies > 0) { afficher_msg = 1; msg_correct = 0; temps_msg = SDL_GetTicks(); }
        }
        
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
            
            if (!game_over && !afficher_msg && jeu.question_actuelle != -1) {
                if (e.type == SDL_MOUSEMOTION) {
                    SDL_Point p = {e.motion.x, e.motion.y};
                    reponse_choisie = -1;
                    for (int i = 0; i < 4; i++)
                        if (SDL_PointInRect(&p, &zones[i])) reponse_choisie = i;
                }
                
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point p = {e.button.x, e.button.y};
                    for (int i = 0; i < 4; i++) {
                        if (SDL_PointInRect(&p, &zones[i])) {
                            if (i == questions[jeu.question_actuelle].bonne_reponse) jeu.score += 100, msg_correct = 1;
                            else { jeu.vies--; if (jeu.vies <= 0) game_over = 1; msg_correct = 0; }
                            
                            afficher_msg = 1;
                            temps_msg = SDL_GetTicks();
                            questions[jeu.question_actuelle].deja_vu = 1;
                            jeu.question_actuelle = prochaineQuestion(questions, nb_questions);
                            jeu.debut_temps = time(NULL);
                            break;
                        }
                    }
                }
            }
            
            if ((game_over || jeu.question_actuelle == -1) && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
                quit = 1;
        }
        
        // Rendu
        SDL_RenderClear(renderer);
        if (bg) { SDL_Rect r = {0, 0, 1000, 700}; SDL_RenderCopy(renderer, bg, NULL, &r); }
        else SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255), SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, 1000, 70});
        
        SDL_Color blanc = {255,255,255}, or = {255,215,0}, rouge = {255,80,80};
        
        // Affichages texte
        char buf[50];
        sprintf(buf, "SCORE: %d", jeu.score);
        SDL_Surface *s = TTF_RenderText_Blended(font_titre, buf, or);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){30,18,160,40}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
        
        sprintf(buf, "VIES: %d", jeu.vies);
        s = TTF_RenderText_Blended(font_titre, buf, rouge);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){220,18,130,40}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
        
        int q_vues = 0;
        for (int i = 0; i < nb_questions; i++) if (questions[i].deja_vu) q_vues++;
        if (jeu.question_actuelle != -1) q_vues++;
        sprintf(buf, "Q %d/%d", q_vues, nb_questions);
        s = TTF_RenderText_Blended(font_titre, buf, blanc);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){430,18,120,40}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
        
        dessinerHorloge(renderer, jeu.temps_restant, 30, 920, 35, 25);
        
        if (game_over || jeu.question_actuelle == -1) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
            SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, 1000, 700});
            
            const char* titre = game_over ? "GAME OVER" : "VICTOIRE !";
            SDL_Color couleur = game_over ? rouge : or;
            s = TTF_RenderText_Blended(font_question, titre, couleur);
            if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){350,280,300,70}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
            
            sprintf(buf, "Score final: %d", jeu.score);
            s = TTF_RenderText_Blended(font_reponse, buf, blanc);
            if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){360,370,280,40}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
            
            s = TTF_RenderText_Blended(font_reponse, "Appuyez sur ESPACE pour quitter", blanc);
            if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){300,450,400,40}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
        }
        else if (afficher_msg) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &(SDL_Rect){300,300,400,80});
            const char* msg = msg_correct ? "BRAVO ! +100" : "MAUVAISE ! -1 vie";
            s = TTF_RenderText_Blended(font_question, msg, msg_correct ? (SDL_Color){80,255,80} : rouge);
            if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){320,320,360,50}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
            if (SDL_GetTicks() - temps_msg > 1500) afficher_msg = 0;
        }
        else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_RenderFillRect(renderer, &(SDL_Rect){50,90,900,90});
            
            s = TTF_RenderText_Blended(font_question, questions[jeu.question_actuelle].question, (SDL_Color){255,255,200});
            if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); int w,h; SDL_QueryTexture(t,NULL,NULL,&w,&h); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){(1000-w)/2,105,w,h}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
            
            for (int i = 0; i < 4; i++) {
                SDL_SetRenderDrawColor(renderer, reponse_choisie == i ? 0 : 0, reponse_choisie == i ? 100 : 0, reponse_choisie == i ? 200 : 0, 220);
                SDL_RenderFillRect(renderer, &zones[i]);
                SDL_SetRenderDrawColor(renderer, 255,200,0,220);
                SDL_RenderDrawRect(renderer, &zones[i]);
                
                char lettre[2] = {'A' + i, 0};
                s = TTF_RenderText_Blended(font_reponse, lettre, or);
                if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){zones[i].x+20, zones[i].y+18,30,30}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
                
                s = TTF_RenderText_Blended(font_reponse, questions[jeu.question_actuelle].reponses[i], blanc);
                if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s); int w,h; SDL_QueryTexture(t,NULL,NULL,&w,&h); SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){zones[i].x+65, zones[i].y+(65-h)/2, w>600?600:w, h}); SDL_FreeSurface(s); SDL_DestroyTexture(t); }
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    if (bg) SDL_DestroyTexture(bg);
    TTF_CloseFont(font_titre); TTF_CloseFont(font_question); TTF_CloseFont(font_reponse);
    return jeu.score;
}
