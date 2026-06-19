#include "puzzle.h"

int positionOK(Piece *p, int dossier) {
    int x = 300, y = 20;
    if (dossier == 1) { x = 293; y = 363; }
    if (dossier == 2) { x = 440; y = 343; }
    if (dossier == 3) { x = 301; y = 15; }
    if (dossier == 4) { x = 480; y = 153; }
    if (dossier == 5) { x = 298; y = 17; }

    return abs(p->rect.x - x) < SEUIL_CIBLE &&
           abs(p->rect.y - y) < SEUIL_CIBLE;
}

void initJeu(Jeu *jeu, SDL_Renderer *r, TTF_Font *font) {
    SDL_Surface *s = IMG_Load("bg&police/background.jpg");
    jeu->fond.texture = SDL_CreateTextureFromSurface(r, s);
    jeu->fond.rect = (SDL_Rect){0,0,LARGEUR_FENETRE,HAUTEUR_FENETRE};
    SDL_FreeSurface(s);

    srand(time(NULL));
    jeu->dossier = rand()%5 + 1;

    for(int i=0;i<NB_PIECES;i++){
        char path[128];
        sprintf(path,"puzzlerand/%d/puzzle%d.png",jeu->dossier,i+1);
        s = IMG_Load(path);
        jeu->pieces[i].texture = SDL_CreateTextureFromSurface(r,s);
        jeu->pieces[i].rect = (SDL_Rect){0,0,s->w,s->h};
        jeu->pieces[i].original = jeu->pieces[i].rect;
        jeu->pieces[i].visible = 1;
        jeu->pieces[i].secoue = 0;
        SDL_FreeSurface(s);
    }

    jeu->pieces[0].rect.x=300; jeu->pieces[0].rect.y=20;
    jeu->pieces[1].rect.x=200; jeu->pieces[1].rect.y=550;
    jeu->pieces[2].rect.x=500; jeu->pieces[2].rect.y=550;
    jeu->pieces[3].rect.x=900; jeu->pieces[3].rect.y=550;

    for(int i=0;i<11;i++){
        char c[16];
        sprintf(c,"chrono/%d.png",10-i);
        s = IMG_Load(c);
        jeu->chrono.textures[i]=SDL_CreateTextureFromSurface(r,s);
        SDL_FreeSurface(s);
    }

    int w,h;
    SDL_QueryTexture(jeu->chrono.textures[0],NULL,NULL,&w,&h);
    jeu->chrono.rect=(SDL_Rect){1000,20,w,h};
    jeu->chrono.original=jeu->chrono.rect;
    jeu->chrono.index=0;
    jeu->chrono.secondes=10;
    jeu->chrono.fini=0;
    jeu->chrono.alerte=0;
    jeu->chrono.dernier=SDL_GetTicks();

    SDL_Color c1={255,215,0,255};
    SDL_Color c2={255,0,0,255};

    s = TTF_RenderText_Solid(font,"GOOD!",c1);
    jeu->victoire.texture=SDL_CreateTextureFromSurface(r,s);
    jeu->victoire.rect=(SDL_Rect){500,500,s->w,s->h};
    jeu->victoire.visible=0;
    jeu->victoire.zoom=0.1f;
    SDL_FreeSurface(s);

    s = TTF_RenderText_Solid(font,"TIME OVER!",c1);
    jeu->temps.texture=SDL_CreateTextureFromSurface(r,s);
    jeu->temps.rect=(SDL_Rect){500,500,s->w,s->h};
    jeu->temps.visible=0;
    jeu->temps.zoom=0.1f;
    SDL_FreeSurface(s);

    s = TTF_RenderText_Solid(font,"ERREUR -2s",c2);
    jeu->erreur.texture=SDL_CreateTextureFromSurface(r,s);
    jeu->erreur.rect=(SDL_Rect){500,650,s->w,s->h};
    jeu->erreur.visible=0;
    SDL_FreeSurface(s);

    jeu->active=NULL;
    jeu->fini=0;
}

void updateJeu(Jeu *jeu) {

    if(!jeu->chrono.fini && !jeu->fini){
        if(SDL_GetTicks() - jeu->chrono.dernier > 1000){
            jeu->chrono.secondes--;
            jeu->chrono.index=10-jeu->chrono.secondes;
            jeu->chrono.dernier=SDL_GetTicks();

            if(jeu->chrono.secondes<=3 && jeu->chrono.secondes>0)
                jeu->chrono.alerte=1;
            else
                jeu->chrono.alerte=0;

            if(jeu->chrono.secondes<=0){
                jeu->chrono.secondes=0;
                jeu->chrono.fini=1;
                jeu->temps.visible=1;
                jeu->fini=1;
            }
        }
    }

    if(jeu->chrono.alerte){
        int dx=(rand()%7)-3;
        int dy=(rand()%7)-3;
        jeu->chrono.rect.x=jeu->chrono.original.x+dx;
        jeu->chrono.rect.y=jeu->chrono.original.y+dy;
    } else {
        jeu->chrono.rect=jeu->chrono.original;
    }

    for(int i=0;i<NB_PIECES;i++){
        if(jeu->pieces[i].secoue){
            if(SDL_GetTicks() - jeu->pieces[i].debut > 500){
                jeu->pieces[i].secoue=0;
                jeu->pieces[i].rect=jeu->pieces[i].original;
            } else {
                int dx=(rand()%10)-5;
                int dy=(rand()%10)-5;
                jeu->pieces[i].rect.x=jeu->pieces[i].original.x+dx;
                jeu->pieces[i].rect.y=jeu->pieces[i].original.y+dy;
            }
        }
    }

    if(jeu->victoire.visible && jeu->victoire.zoom<1.0f)
        jeu->victoire.zoom+=0.05f;

    if(jeu->temps.visible && jeu->temps.zoom<1.0f)
        jeu->temps.zoom+=0.05f;

    if(jeu->erreur.visible){
        if(SDL_GetTicks() - jeu->tempsErreur > 1000)
            jeu->erreur.visible=0;
    }
}

void renderJeu(Jeu *jeu, SDL_Renderer *r) {
    SDL_RenderClear(r);
    SDL_RenderCopy(r,jeu->fond.texture,NULL,&jeu->fond.rect);

    if(jeu->chrono.alerte && SDL_GetTicks()%200<100)
        SDL_SetTextureColorMod(jeu->chrono.textures[jeu->chrono.index],255,100,100);

    SDL_RenderCopy(r,
        jeu->chrono.textures[jeu->chrono.index],
        NULL,&jeu->chrono.rect);

    SDL_SetTextureColorMod(jeu->chrono.textures[jeu->chrono.index],255,255,255);

    for(int i=0;i<NB_PIECES;i++)
        if(jeu->pieces[i].visible)
            SDL_RenderCopy(r,jeu->pieces[i].texture,NULL,&jeu->pieces[i].rect);

    if(jeu->victoire.visible){
        int w=jeu->victoire.rect.w*jeu->victoire.zoom;
        int h=jeu->victoire.rect.h*jeu->victoire.zoom;
        SDL_Rect d={600-w/2,550-h/2,w,h};
        SDL_RenderCopy(r,jeu->victoire.texture,NULL,&d);
    }

    if(jeu->temps.visible){
        int w=jeu->temps.rect.w*jeu->temps.zoom;
        int h=jeu->temps.rect.h*jeu->temps.zoom;
        SDL_Rect d={600-w/2,550-h/2,w,h};
        SDL_RenderCopy(r,jeu->temps.texture,NULL,&d);
    }

    if(jeu->erreur.visible)
        SDL_RenderCopy(r,jeu->erreur.texture,NULL,&jeu->erreur.rect);

    SDL_RenderPresent(r);
}

void handleEvents(Jeu *jeu, SDL_Event *e) {
    if(e->type==SDL_MOUSEBUTTONDOWN){
        for(int i=1;i<NB_PIECES;i++){
            if(e->button.x>=jeu->pieces[i].rect.x &&
               e->button.x<=jeu->pieces[i].rect.x+jeu->pieces[i].rect.w &&
               e->button.y>=jeu->pieces[i].rect.y &&
               e->button.y<=jeu->pieces[i].rect.y+jeu->pieces[i].rect.h){
                jeu->active=&jeu->pieces[i];
            }
        }
    }

    if(e->type==SDL_MOUSEBUTTONUP && jeu->active){
        if(jeu->active==&jeu->pieces[1] &&
           positionOK(jeu->active,jeu->dossier)){
            jeu->victoire.visible=1;
            jeu->chrono.fini=1;
            jeu->fini=1;
        } else {
            jeu->active->secoue=1;
            jeu->active->debut=SDL_GetTicks();
            jeu->erreur.visible=1;
            jeu->tempsErreur=SDL_GetTicks();
            if(jeu->chrono.secondes>PENALITE_TEMPS)
                jeu->chrono.secondes-=PENALITE_TEMPS;
        }
        jeu->active=NULL;
    }

    if(e->type==SDL_MOUSEMOTION && jeu->active){
        jeu->active->rect.x=e->motion.x;
        jeu->active->rect.y=e->motion.y;
    }
}

void cleanJeu(Jeu *jeu) {
    SDL_DestroyTexture(jeu->fond.texture);

    for(int i=0;i<NB_PIECES;i++)
        SDL_DestroyTexture(jeu->pieces[i].texture);

    for(int i=0;i<11;i++)
        SDL_DestroyTexture(jeu->chrono.textures[i]);

    SDL_DestroyTexture(jeu->victoire.texture);
    SDL_DestroyTexture(jeu->temps.texture);
    SDL_DestroyTexture(jeu->erreur.texture);
}
