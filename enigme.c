#include "enigme.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NB_QUESTIONS_QUIZ 10  // ✅ Nombre de questions par partie

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
        if (ligne[0] == '\n' || ligne[0] == '#') continue;
        
        ligne[strcspn(ligne, "\n")] = 0;
        strcpy(questions[count].question, ligne);
        
        for (int i = 0; i < MAX_REPONSES; i++) {
            if (fgets(ligne, sizeof(ligne), file)) {
                ligne[strcspn(ligne, "\n")] = 0;
                char *ptr = ligne;
                while (*ptr >= '0' && *ptr <= '9') ptr++;
                if (*ptr == ' ' || *ptr == '.') ptr++;
                while (*ptr == ' ') ptr++;
                strcpy(questions[count].reponses[i], ptr);
            }
        }
        
        if (fgets(ligne, sizeof(ligne), file)) {
            questions[count].bonne_reponse = atoi(ligne);
        }
        
        questions[count].deja_vu = 0;
        count++;
    }
    
    fclose(file);
    return count;
}

// Mélanger les questions (Fisher-Yates)
void melangerQuestions(Question questions[], int nb) {
    for (int i = nb - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Question temp = questions[i];
        questions[i] = questions[j];
        questions[j] = temp;
    }
}

// ✅ Sélectionner NB_QUESTIONS_QUIZ questions aléatoires parmi toutes
int selectionnerQuestions(Question toutes[], int nb_total, Question selection[], int nb_select) {
    if (nb_total < nb_select) nb_select = nb_total;
    
    // Copier et mélanger
    Question copie[MAX_QUESTIONS];
    for (int i = 0; i < nb_total; i++) copie[i] = toutes[i];
    melangerQuestions(copie, nb_total);
    
    // Prendre les nb_select premières
    for (int i = 0; i < nb_select; i++) {
        selection[i] = copie[i];
        selection[i].deja_vu = 0;
    }
    return nb_select;
}

// Trouver la prochaine question non vue
int prochaineQuestion(Question questions[], int nb) {
    for (int i = 0; i < nb; i++) {
        if (!questions[i].deja_vu) return i;
    }
    return -1;
}

// ✅ Fonction pour dessiner une horloge qui diminue (sans texte numérique)
void dessinerHorloge(SDL_Renderer *renderer, int temps_restant, int temps_max, int x, int y, int rayon) {
    // Calculer l'angle en fonction du temps restant (angle en radians)
    float angle = (float)temps_restant / temps_max * 2 * M_PI;
    // L'horloge diminue de 360° à 0° : 0 secondes = 0°, temps_max = 360°
    float angle_final = angle; // 0° quand temps_restant = 0, 360° quand temps_restant = temps_max
    
    // Couleur de l'arc (blanc à rouge)
    SDL_Color couleur_arc;
    if (temps_restant <= 5) {
        couleur_arc = (SDL_Color){255, 50, 50, 255}; // Rouge clignotant
    } else if (temps_restant <= 10) {
        couleur_arc = (SDL_Color){255, 200, 50, 255}; // Orange
    } else {
        couleur_arc = (SDL_Color){80, 200, 255, 255}; // Bleu clair
    }
    
    // Dessiner l'anneau extérieur (fond gris)
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
    for (int w = 0; w < 8; w++) {
        for (int i = -w; i <= w; i++) {
            for (int j = -w; j <= w; j++) {
                if (i*i + j*j <= rayon*rayon) {
                    SDL_RenderDrawPoint(renderer, x + i, y + j);
                }
            }
        }
    }
    
    // Dessiner l'anneau intérieur (noir)
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    for (int w = 0; w < rayon-5; w++) {
        for (int i = -w; i <= w; i++) {
            for (int j = -w; j <= w; j++) {
                if (i*i + j*j <= (rayon-5)*(rayon-5)) {
                    SDL_RenderDrawPoint(renderer, x + i, y + j);
                }
            }
        }
    }
    
    // Dessiner l'arc de cercle (aiguille de l'horloge)
    int cx = x;
    int cy = y;
    int r = rayon - 5;
    
    // Dessiner l'arc de cercle (secteur)
    if (angle_final > 0) {
        int aiguille_x, aiguille_y;
        float angle_step = angle_final / 30.0f;
        
        // Tracer plusieurs lignes pour former un secteur
        SDL_SetRenderDrawColor(renderer, couleur_arc.r, couleur_arc.g, couleur_arc.b, 200);
        for (float a = 0; a <= angle_final; a += angle_step) {
            aiguille_x = cx + (int)(r * sin(a));
            aiguille_y = cy - (int)(r * cos(a));
            SDL_RenderDrawLine(renderer, cx, cy, aiguille_x, aiguille_y);
        }
        
        // Tracer l'arc externe
        for (float a = 0; a <= angle_final; a += 0.05f) {
            aiguille_x = cx + (int)(r * sin(a));
            aiguille_y = cy - (int)(r * cos(a));
            for (int offset = -2; offset <= 2; offset++) {
                SDL_RenderDrawPoint(renderer, aiguille_x + offset, aiguille_y);
                SDL_RenderDrawPoint(renderer, aiguille_x, aiguille_y + offset);
            }
        }
    }
    
    // Effet de pulsation quand le temps est critique (<= 3 secondes)
    if (temps_restant <= 3) {
        static int pulse = 0;
        pulse = (pulse + 1) % 20;
        if (pulse < 10) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
            for (int w = 0; w < rayon + 5; w++) {
                for (int i = -w; i <= w; i++) {
                    for (int j = -w; j <= w; j++) {
                        if (i*i + j*j <= (rayon+5)*(rayon+5) && i*i + j*j >= (rayon-5)*(rayon-5)) {
                            SDL_RenderDrawPoint(renderer, x + i, y + j);
                        }
                    }
                }
            }
        }
    }
}

// ✅ Retourne le score final
int runEnigme(SDL_Renderer *renderer) {
    srand(time(NULL));

    // Charger toutes les questions
    Question toutes_questions[MAX_QUESTIONS];
    int nb_total = chargerQuestions("questions.txt", toutes_questions, MAX_QUESTIONS);
    
    if (nb_total == 0) {
        printf("Aucune question chargee!\n");
        return 0;
    }
    
    // ✅ Sélectionner 10 questions aléatoires
    Question questions[NB_QUESTIONS_QUIZ];
    int nb_questions = selectionnerQuestions(toutes_questions, nb_total, questions, NB_QUESTIONS_QUIZ);
    
    printf("Quiz: %d questions selectionnees aleatoirement\n", nb_questions);

    // Initialiser le jeu
    JeuData jeu = {
        .score = 0,
        .vies = 3,
        .questions_posees = 0,
        .temps_restant = 30,
        .debut_temps = time(NULL),
        .question_actuelle = prochaineQuestion(questions, nb_questions)
    };
    
    // Charger le fond
    SDL_Texture *bg = NULL;
    bg = IMG_LoadTexture(renderer, "images/background.png");
    
    // Polices
    TTF_Font *font_titre = NULL;
    TTF_Font *font_question = NULL;
    TTF_Font *font_reponse = NULL;
    
    const char* fonts[] = {
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-Regular.ttf",
        "/usr/share/fonts/truetype/arial/arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    for (int i = 0; i < 6; i++) { font_titre   = TTF_OpenFont(fonts[i], 28); if (font_titre)   break; }
    for (int i = 0; i < 6; i++) { font_question = TTF_OpenFont(fonts[i], 24); if (font_question) break; }
    for (int i = 0; i < 6; i++) { font_reponse  = TTF_OpenFont(fonts[i], 22); if (font_reponse)  break; }
    
    if (!font_titre || !font_question || !font_reponse) {
        printf("Erreur: Impossible de charger les polices\n");
        if (bg) SDL_DestroyTexture(bg);
        return 0;
    }
    
    int reponse_choisie = -1;
    int afficher_message = 0;
    int message_correct = 0;
    Uint32 temps_message = 0;
    int game_over = 0;
    int quit = 0;
    
    SDL_Event e;
    
    // Zones de clic pour les réponses
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
        
        // ✅ Temps écoulé → perte d'une vie
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
            
            // ESPACE pour quitter (fin de partie)
            if (e.type == SDL_KEYDOWN && (game_over || jeu.question_actuelle == -1)) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    quit = 1;
                }
            }
        }
        
        // ===== RENDU =====
        SDL_RenderClear(renderer);
        
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
        
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Color or    = {255, 215, 0,   255};
        SDL_Color rouge = {255, 80,  80,  255};
        SDL_Color vert  = {80,  255, 80,  255};
        
        // Score
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
        
        // Vies
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

        // Compteur de questions
        char texte_qnum[30];
        int q_actuelle = 0;
        for (int i = 0; i < nb_questions; i++) if (questions[i].deja_vu) q_actuelle++;
        if (jeu.question_actuelle != -1) q_actuelle++;
        sprintf(texte_qnum, "Q %d/%d", q_actuelle, nb_questions);
        SDL_Surface *surf_qnum = TTF_RenderText_Blended(font_titre, texte_qnum, blanc);
        if (surf_qnum) {
            SDL_Texture *tex_qnum = SDL_CreateTextureFromSurface(renderer, surf_qnum);
            SDL_Rect rect_qnum = {430, 18, 120, 40};
            SDL_RenderCopy(renderer, tex_qnum, NULL, &rect_qnum);
            SDL_FreeSurface(surf_qnum);
            SDL_DestroyTexture(tex_qnum);
        }
        
        // ===== HORLOGE ANIMÉE (sans texte numérique) =====
        int horloge_x = 920;
        int horloge_y = 35;
        int horloge_rayon = 25;
        
        dessinerHorloge(renderer, jeu.temps_restant, 30, horloge_x, horloge_y, horloge_rayon);
        
        // ===== GAME OVER =====
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
            
            char score_txt[100];
            sprintf(score_txt, "Score final: %d", jeu.score);
            SDL_Surface *surf_sf = TTF_RenderText_Blended(font_reponse, score_txt, blanc);
            if (surf_sf) {
                SDL_Texture *tex_sf = SDL_CreateTextureFromSurface(renderer, surf_sf);
                SDL_Rect rect_sf = {370, 370, 260, 40};
                SDL_RenderCopy(renderer, tex_sf, NULL, &rect_sf);
                SDL_FreeSurface(surf_sf);
                SDL_DestroyTexture(tex_sf);
            }
            
            SDL_Surface *surf_msg = TTF_RenderText_Blended(font_reponse, "Appuyez sur ESPACE pour quitter", blanc);
            if (surf_msg) {
                SDL_Texture *tex_msg = SDL_CreateTextureFromSurface(renderer, surf_msg);
                SDL_Rect rect_msg = {300, 450, 400, 40};
                SDL_RenderCopy(renderer, tex_msg, NULL, &rect_msg);
                SDL_FreeSurface(surf_msg);
                SDL_DestroyTexture(tex_msg);
            }
        }
        // ===== VICTOIRE =====
        else if (jeu.question_actuelle == -1) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
            SDL_Rect overlay = {0, 0, 1000, 700};
            SDL_RenderFillRect(renderer, &overlay);
            
            SDL_Surface *surf_v = TTF_RenderText_Blended(font_question, "VICTOIRE !", or);
            if (surf_v) {
                SDL_Texture *tex_v = SDL_CreateTextureFromSurface(renderer, surf_v);
                SDL_Rect rect_v = {350, 280, 300, 70};
                SDL_RenderCopy(renderer, tex_v, NULL, &rect_v);
                SDL_FreeSurface(surf_v);
                SDL_DestroyTexture(tex_v);
            }
            
            char score_txt[100];
            sprintf(score_txt, "Score final: %d / %d", jeu.score, nb_questions * 100);
            SDL_Surface *surf_sf = TTF_RenderText_Blended(font_reponse, score_txt, blanc);
            if (surf_sf) {
                SDL_Texture *tex_sf = SDL_CreateTextureFromSurface(renderer, surf_sf);
                SDL_Rect rect_sf = {320, 370, 360, 40};
                SDL_RenderCopy(renderer, tex_sf, NULL, &rect_sf);
                SDL_FreeSurface(surf_sf);
                SDL_DestroyTexture(tex_sf);
            }
            
            SDL_Surface *surf_msg = TTF_RenderText_Blended(font_reponse, "Appuyez sur ESPACE pour quitter", blanc);
            if (surf_msg) {
                SDL_Texture *tex_msg = SDL_CreateTextureFromSurface(renderer, surf_msg);
                SDL_Rect rect_msg = {300, 450, 400, 40};
                SDL_RenderCopy(renderer, tex_msg, NULL, &rect_msg);
                SDL_FreeSurface(surf_msg);
                SDL_DestroyTexture(tex_msg);
            }
        }
        // ===== MESSAGE FEEDBACK =====
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
        // ===== AFFICHAGE DE LA QUESTION =====
        else {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect fond_q = {50, 90, 900, 90};
            SDL_RenderFillRect(renderer, &fond_q);
            
            SDL_Color jaune = {255, 255, 200, 255};
            SDL_Surface *surf_q = TTF_RenderText_Blended(font_question, questions[jeu.question_actuelle].question, jaune);
            if (surf_q) {
                SDL_Texture *tex_q = SDL_CreateTextureFromSurface(renderer, surf_q);
                int w, h;
                SDL_QueryTexture(tex_q, NULL, NULL, &w, &h);
                int x = (1000 - w) / 2;
                if (w > 900) { w = 900; x = 50; }
                SDL_Rect rect_q = {x, 105, w, h};
                SDL_RenderCopy(renderer, tex_q, NULL, &rect_q);
                SDL_FreeSurface(surf_q);
                SDL_DestroyTexture(tex_q);
            }
            
            for (int i = 0; i < MAX_REPONSES; i++) {
                if (reponse_choisie == i) {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 200, 220);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
                }
                SDL_RenderFillRect(renderer, &zones_reponses[i]);
                
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 220);
                SDL_RenderDrawRect(renderer, &zones_reponses[i]);
                
                char lettre[3];
                sprintf(lettre, "%c", 'A' + i);
                SDL_Surface *surf_l = TTF_RenderText_Blended(font_reponse, lettre, or);
                if (surf_l) {
                    SDL_Texture *tex_l = SDL_CreateTextureFromSurface(renderer, surf_l);
                    SDL_Rect rect_l = {zones_reponses[i].x + 20, zones_reponses[i].y + 18, 30, 30};
                    SDL_RenderCopy(renderer, tex_l, NULL, &rect_l);
                    SDL_FreeSurface(surf_l);
                    SDL_DestroyTexture(tex_l);
                }
                
                SDL_Surface *surf_r = TTF_RenderText_Blended(font_reponse, questions[jeu.question_actuelle].reponses[i], blanc);
                if (surf_r) {
                    SDL_Texture *tex_r = SDL_CreateTextureFromSurface(renderer, surf_r);
                    int w, h;
                    SDL_QueryTexture(tex_r, NULL, NULL, &w, &h);
                    int y = zones_reponses[i].y + (zones_reponses[i].h - h) / 2;
                    int x = zones_reponses[i].x + 65;
                    if (w > 600) w = 600;
                    SDL_Rect rect_r = {x, y, w, h};
                    SDL_RenderCopy(renderer, tex_r, NULL, &rect_r);
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
    if (font_titre)    TTF_CloseFont(font_titre);
    if (font_question) TTF_CloseFont(font_question);
    if (font_reponse)  TTF_CloseFont(font_reponse);
    
    return jeu.score;
}
