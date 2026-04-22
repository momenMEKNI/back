#include "enigme.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Fonction pour charger les questions
int chargerQuestions(const char* fichier, Question questions[], int max) {
    FILE* file = fopen(fichier, "r");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir %s\n", fichier);
        return 0;
    }
    
    int count = 0;
    char ligne[MAX_TEXTE];
    
    while (count < max && fgets(ligne, sizeof(ligne), file)) {
        // Ignorer les lignes vides ou commentaires
        if (ligne[0] == '\n' || ligne[0] == '#') continue;
        
        // Enlever le saut de ligne
        ligne[strcspn(ligne, "\n")] = 0;
        strcpy(questions[count].question, ligne);
        
        // Lire les 4 réponses
        for (int i = 0; i < MAX_REPONSES; i++) {
            if (fgets(ligne, sizeof(ligne), file)) {
                ligne[strcspn(ligne, "\n")] = 0;
                // Enlever l'éventuel numéro au début de la ligne
                char *ptr = ligne;
                while (*ptr >= '0' && *ptr <= '9') ptr++;
                if (*ptr == ' ' || *ptr == '.') ptr++;
                while (*ptr == ' ') ptr++;
                strcpy(questions[count].reponses[i], ptr);
            }
        }
        
        // Lire l'index de la bonne réponse
        if (fgets(ligne, sizeof(ligne), file)) {
            questions[count].bonne_reponse = atoi(ligne);
        }
        
        questions[count].deja_vu = 0;
        count++;
    }
    
    fclose(file);
    return count;
}

// Mélanger les questions
void melangerQuestions(Question questions[], int nb) {
    srand(time(NULL));
    for (int i = nb - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Question temp = questions[i];
        questions[i] = questions[j];
        questions[j] = temp;
    }
}

// Trouver la prochaine question non vue
int prochaineQuestion(Question questions[], int nb) {
    for (int i = 0; i < nb; i++) {
        if (!questions[i].deja_vu) {
            return i;
        }
    }
    return -1;
}

int runEnigme(SDL_Renderer *renderer) {
    // Charger les questions
    Question questions[MAX_QUESTIONS];
    int nb_questions = chargerQuestions("questions.txt", questions, MAX_QUESTIONS);
    
    if (nb_questions == 0) {
        printf("Aucune question chargee!\n");
        return 0;
    }
    
    melangerQuestions(questions, nb_questions);
    
    // Initialiser le jeu
    JeuData jeu = {
        .score = 0,
        .vies = 3,
        .questions_posees = 0,
        .temps_restant = 30,
        .debut_temps = time(NULL),
        .question_actuelle = prochaineQuestion(questions, nb_questions)
    };
    
    // CHARGER LE FOND (AJOUTÉ)
    SDL_Texture *bg = NULL;
    bg = IMG_LoadTexture(renderer, "images/background.png");
    
    // Polices
    TTF_Font *font_titre = NULL;
    TTF_Font *font_question = NULL;
    TTF_Font *font_reponse = NULL;
    
    // Essayer différentes polices
    const char* fonts[] = {
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-Regular.ttf",
        "/usr/share/fonts/truetype/arial/arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    for (int i = 0; i < 6; i++) {
        font_titre = TTF_OpenFont(fonts[i], 28);
        if (font_titre) break;
    }
    
    for (int i = 0; i < 6; i++) {
        font_question = TTF_OpenFont(fonts[i], 24);
        if (font_question) break;
    }
    
    for (int i = 0; i < 6; i++) {
        font_reponse = TTF_OpenFont(fonts[i], 22);
        if (font_reponse) break;
    }
    
    // Si aucune police n'est trouvée, en créer une par défaut
    if (!font_titre || !font_question || !font_reponse) {
        printf("Erreur chargement polices, utilisation de la police par defaut\n");
        if (!font_titre) font_titre = TTF_OpenFont(NULL, 28);
        if (!font_question) font_question = TTF_OpenFont(NULL, 24);
        if (!font_reponse) font_reponse = TTF_OpenFont(NULL, 22);
        
        if (!font_titre || !font_question || !font_reponse) {
            printf("Erreur: Impossible de charger les polices\n");
            if (bg) SDL_DestroyTexture(bg);
            return 0;
        }
    }
    
    int reponse_choisie = -1;
    int afficher_message = 0;
    int message_correct = 0;
    Uint32 temps_message = 0;
    int game_over = 0;
    int quit = 0;
    
    SDL_Event e;
    
    // Définir les zones de clic pour les réponses
    SDL_Rect zones_reponses[4];
    for (int i = 0; i < MAX_REPONSES; i++) {
        zones_reponses[i].x = 150;
        zones_reponses[i].y = 210 + i * 75;
        zones_reponses[i].w = 700;
        zones_reponses[i].h = 65;
    }
    
    while (!quit) {
        // Gestion du temps
        time_t maintenant = time(NULL);
        jeu.temps_restant = 30 - (int)(maintenant - jeu.debut_temps);
        
        if (jeu.temps_restant <= 0 && !afficher_message && !game_over && jeu.question_actuelle != -1) {
            jeu.vies--;
            afficher_message = 1;
            message_correct = 0;
            temps_message = SDL_GetTicks();
            
            if (jeu.vies <= 0) {
                game_over = 1;
            } else {
                questions[jeu.question_actuelle].deja_vu = 1;
                jeu.question_actuelle = prochaineQuestion(questions, nb_questions);
                jeu.debut_temps = time(NULL);
            }
        }
        
        // Événements
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
                break;
            }
            
            if (!game_over && !afficher_message && jeu.question_actuelle != -1) {
                if (e.type == SDL_MOUSEMOTION) {
                    SDL_Point p = {e.motion.x, e.motion.y};
                    reponse_choisie = -1;
                    for (int i = 0; i < MAX_REPONSES; i++) {
                        if (SDL_PointInRect(&p, &zones_reponses[i])) {
                            reponse_choisie = i;
                            break;
                        }
                    }
                }
                
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point p = {e.button.x, e.button.y};
                    for (int i = 0; i < MAX_REPONSES; i++) {
                        if (SDL_PointInRect(&p, &zones_reponses[i])) {
                            if (i == questions[jeu.question_actuelle].bonne_reponse) {
                                jeu.score += 100;
                                message_correct = 1;
                            } else {
                                jeu.vies--;
                                message_correct = 0;
                                if (jeu.vies <= 0) game_over = 1;
                            }
                            
                            afficher_message = 1;
                            temps_message = SDL_GetTicks();
                            questions[jeu.question_actuelle].deja_vu = 1;
                            jeu.question_actuelle = prochaineQuestion(questions, nb_questions);
                            jeu.debut_temps = time(NULL);
                            break;
                        }
                    }
                }
            }
            
            // CORRECTION : Appuyer sur ESPACE pour QUITTER
            if (e.type == SDL_KEYDOWN && (game_over || jeu.question_actuelle == -1)) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    quit = 1;  // Quitter le quiz
                }
            }
        }
        
        // Affichage
        SDL_RenderClear(renderer);
        
        // Fond
        if (bg) {
            SDL_Rect rect_fond = {0, 0, 1000, 700};
            SDL_RenderCopy(renderer, bg, NULL, &rect_fond);
        } else {
            SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
            SDL_RenderClear(renderer);
        }
        
        // Bandeau supérieur
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect bandeau_haut = {0, 0, 1000, 70};
        SDL_RenderFillRect(renderer, &bandeau_haut);
        
        // Affichage du score
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Color or = {255, 215, 0, 255};
        SDL_Color rouge = {255, 80, 80, 255};
        SDL_Color vert = {80, 255, 80, 255};
        
        char texte_score[50];
        sprintf(texte_score, "SCORE: %d", jeu.score);
        SDL_Surface *surf_score = TTF_RenderText_Blended(font_titre, texte_score, or);
        if (surf_score) {
            SDL_Texture *tex_score = SDL_CreateTextureFromSurface(renderer, surf_score);
            SDL_Rect rect_score = {30, 18, 160, 40};
            SDL_RenderCopy(renderer, tex_score, NULL, &rect_score);
            SDL_FreeSurface(surf_score);
            SDL_DestroyTexture(tex_score);
        }
        
        // Affichage des vies
        char texte_vies[50];
        sprintf(texte_vies, "VIES: %d", jeu.vies);
        SDL_Surface *surf_vies = TTF_RenderText_Blended(font_titre, texte_vies, rouge);
        if (surf_vies) {
            SDL_Texture *tex_vies = SDL_CreateTextureFromSurface(renderer, surf_vies);
            SDL_Rect rect_vies = {220, 18, 130, 40};
            SDL_RenderCopy(renderer, tex_vies, NULL, &rect_vies);
            SDL_FreeSurface(surf_vies);
            SDL_DestroyTexture(tex_vies);
        }
        
        // Barre de temps
        SDL_Rect barre_fond = {800, 15, 170, 40};
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderFillRect(renderer, &barre_fond);
        
        int pourcentage = (jeu.temps_restant * 170) / 30;
        if (pourcentage < 0) pourcentage = 0;
        if (pourcentage > 170) pourcentage = 170;
        SDL_Rect barre_temps = {800, 15, pourcentage, 40};
        SDL_Color couleur_temps = jeu.temps_restant <= 5 ? rouge : (jeu.temps_restant <= 10 ? or : vert);
        SDL_SetRenderDrawColor(renderer, couleur_temps.r, couleur_temps.g, couleur_temps.b, 255);
        SDL_RenderFillRect(renderer, &barre_temps);
        
        char texte_temps[20];
        sprintf(texte_temps, "%ds", jeu.temps_restant);
        SDL_Surface *surf_temps = TTF_RenderText_Blended(font_titre, texte_temps, blanc);
        if (surf_temps) {
            SDL_Texture *tex_temps = SDL_CreateTextureFromSurface(renderer, surf_temps);
            SDL_Rect rect_temps = {855, 22, 60, 30};
            SDL_RenderCopy(renderer, tex_temps, NULL, &rect_temps);
            SDL_FreeSurface(surf_temps);
            SDL_DestroyTexture(tex_temps);
        }
        
        // Game Over
        if (game_over) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
            SDL_Rect overlay = {0, 0, 1000, 700};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_Surface *surf_go = TTF_RenderText_Blended(font_question, "GAME OVER", rouge);
            if (surf_go) {
                SDL_Texture *tex_go = SDL_CreateTextureFromSurface(renderer, surf_go);
                SDL_Rect rect_go = {350, 280, 300, 70};
                SDL_RenderCopy(renderer, tex_go, NULL, &rect_go);
                SDL_FreeSurface(surf_go);
                SDL_DestroyTexture(tex_go);
            }
            
            char score_final[100];
            sprintf(score_final, "Score final: %d", jeu.score);
            SDL_Surface *surf_score_final = TTF_RenderText_Blended(font_reponse, score_final, blanc);
            if (surf_score_final) {
                SDL_Texture *tex_score_final = SDL_CreateTextureFromSurface(renderer, surf_score_final);
                SDL_Rect rect_score_final = {370, 370, 260, 40};
                SDL_RenderCopy(renderer, tex_score_final, NULL, &rect_score_final);
                SDL_FreeSurface(surf_score_final);
                SDL_DestroyTexture(tex_score_final);
            }
            
            // CORRECTION : Message "quitter"
            SDL_Surface *surf_msg = TTF_RenderText_Blended(font_reponse, "Appuyez sur ESPACE pour quitter", blanc);
            if (surf_msg) {
                SDL_Texture *tex_msg = SDL_CreateTextureFromSurface(renderer, surf_msg);
                SDL_Rect rect_msg = {300, 450, 400, 40};
                SDL_RenderCopy(renderer, tex_msg, NULL, &rect_msg);
                SDL_FreeSurface(surf_msg);
                SDL_DestroyTexture(tex_msg);
            }
        }
        // Victoire
        else if (jeu.question_actuelle == -1) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
            SDL_Rect overlay = {0, 0, 1000, 700};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_Surface *surf_victoire = TTF_RenderText_Blended(font_question, "VICTOIRE !", or);
            if (surf_victoire) {
                SDL_Texture *tex_victoire = SDL_CreateTextureFromSurface(renderer, surf_victoire);
                SDL_Rect rect_victoire = {350, 280, 300, 70};
                SDL_RenderCopy(renderer, tex_victoire, NULL, &rect_victoire);
                SDL_FreeSurface(surf_victoire);
                SDL_DestroyTexture(tex_victoire);
            }
            
            char score_final[100];
            sprintf(score_final, "Score final: %d", jeu.score);
            SDL_Surface *surf_score_final = TTF_RenderText_Blended(font_reponse, score_final, blanc);
            if (surf_score_final) {
                SDL_Texture *tex_score_final = SDL_CreateTextureFromSurface(renderer, surf_score_final);
                SDL_Rect rect_score_final = {370, 370, 260, 40};
                SDL_RenderCopy(renderer, tex_score_final, NULL, &rect_score_final);
                SDL_FreeSurface(surf_score_final);
                SDL_DestroyTexture(tex_score_final);
            }
            
            // CORRECTION : Message "quitter"
            SDL_Surface *surf_msg = TTF_RenderText_Blended(font_reponse, "Appuyez sur ESPACE pour quitter", blanc);
            if (surf_msg) {
                SDL_Texture *tex_msg = SDL_CreateTextureFromSurface(renderer, surf_msg);
                SDL_Rect rect_msg = {300, 450, 400, 40};
                SDL_RenderCopy(renderer, tex_msg, NULL, &rect_msg);
                SDL_FreeSurface(surf_msg);
                SDL_DestroyTexture(tex_msg);
            }
        }
        else if (afficher_message) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect msg_bg = {300, 300, 400, 80};
            SDL_RenderFillRect(renderer, &msg_bg);
            
            const char* msg = message_correct ? "BRAVO ! +100" : "MAUVAISE ! -1 vie";
            SDL_Color couleur_msg = message_correct ? vert : rouge;
            SDL_Surface *surf_msg = TTF_RenderText_Blended(font_question, msg, couleur_msg);
            if (surf_msg) {
                SDL_Texture *tex_msg = SDL_CreateTextureFromSurface(renderer, surf_msg);
                SDL_Rect rect_msg = {320, 320, 360, 50};
                SDL_RenderCopy(renderer, tex_msg, NULL, &rect_msg);
                SDL_FreeSurface(surf_msg);
                SDL_DestroyTexture(tex_msg);
            }
            
            if (SDL_GetTicks() - temps_message > 1500) afficher_message = 0;
        }
        else {
            // Fond pour la question
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect fond_question = {50, 90, 900, 90};
            SDL_RenderFillRect(renderer, &fond_question);
            
            // Afficher la question
            SDL_Color jaune = {255, 255, 200, 255};
            SDL_Surface *surf_q = TTF_RenderText_Blended(font_question, questions[jeu.question_actuelle].question, jaune);
            if (surf_q) {
                SDL_Texture *tex_q = SDL_CreateTextureFromSurface(renderer, surf_q);
                int w, h;
                SDL_QueryTexture(tex_q, NULL, NULL, &w, &h);
                int x = (1000 - w) / 2;
                if (w > 900) {
                    w = 900;
                    x = 50;
                }
                SDL_Rect rect_q = {x, 105, w, h};
                SDL_RenderCopy(renderer, tex_q, NULL, &rect_q);
                SDL_FreeSurface(surf_q);
                SDL_DestroyTexture(tex_q);
            }
            
            // Afficher les réponses
            for (int i = 0; i < MAX_REPONSES; i++) {
                // Fond de la réponse
                if (reponse_choisie == i) {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 200, 220);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
                }
                SDL_RenderFillRect(renderer, &zones_reponses[i]);
                
                // Bordure
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 220);
                SDL_RenderDrawRect(renderer, &zones_reponses[i]);
                
                // Lettre de la réponse (A, B, C, D)
                char lettre[3];
                sprintf(lettre, "%c", 'A' + i);
                SDL_Surface *surf_lettre = TTF_RenderText_Blended(font_reponse, lettre, or);
                if (surf_lettre) {
                    SDL_Texture *tex_lettre = SDL_CreateTextureFromSurface(renderer, surf_lettre);
                    SDL_Rect rect_lettre = {zones_reponses[i].x + 20, zones_reponses[i].y + 18, 30, 30};
                    SDL_RenderCopy(renderer, tex_lettre, NULL, &rect_lettre);
                    SDL_FreeSurface(surf_lettre);
                    SDL_DestroyTexture(tex_lettre);
                }
                
                // Texte de la réponse
                SDL_Surface *surf_r = TTF_RenderText_Blended(font_reponse, questions[jeu.question_actuelle].reponses[i], blanc);
                if (surf_r) {
                    SDL_Texture *tex_r = SDL_CreateTextureFromSurface(renderer, surf_r);
                    int w, h;
                    SDL_QueryTexture(tex_r, NULL, NULL, &w, &h);
                    int y = zones_reponses[i].y + (zones_reponses[i].h - h) / 2;
                    int x = zones_reponses[i].x + 65;
                    if (w > 600) w = 600;
                    SDL_Rect rect_texte = {x, y, w, h};
                    SDL_RenderCopy(renderer, tex_r, NULL, &rect_texte);
                    SDL_FreeSurface(surf_r);
                    SDL_DestroyTexture(tex_r);
                }
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    // Nettoyage
    if (bg) SDL_DestroyTexture(bg);
    if (font_titre) TTF_CloseFont(font_titre);
    if (font_question) TTF_CloseFont(font_question);
    if (font_reponse) TTF_CloseFont(font_reponse);
    
    return 1;
}
