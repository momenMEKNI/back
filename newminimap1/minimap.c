/* =====================================================================
   minimap.c  —  Implémentation des fonctions de la mini-carte
   =====================================================================

   RÔLE DE CE FICHIER
   ------------------
   Ce fichier contient UNIQUEMENT le corps (le code) des 3 fonctions
   déclarées dans minimap.h :
     • initMiniMap    — créer la texture miniaturisée du fond
     • afficherMiniMap — dessiner la minimap à chaque frame
     • freeMiniMap    — libérer la mémoire

   Il contient aussi 3 fonctions "privées" (static) utilisées en
   interne uniquement : dessineTexte, dessinePastille, mondeVersMinimap.

   CE QU'IL NE FAIT PAS
   --------------------
   Il ne déclare pas la struct MiniMap   → c'est dans minimap.h
   Il n'a pas de fonction main()         → c'est dans main.c
   Il ne charge pas le background PNG    → c'est dans main.c

   CONTEXTE DU JEU
   ---------------
   Fenetre       : 800 x 600 px
   Background L1 : 19 180 x 4 282 px  (BG_LEVEL_1_VECTOR_ART.png)
   Scrolling     : bgOffset in [0 , bgW x 2[
                   segment pair   = image normale
                   segment impair = image retournee (miroir)
   ===================================================================== */

#include "minimap.h"   /* struct MiniMap, constantes MM_X/Y/W/H, prototypes */
#include <stdio.h>     /* printf() -- messages de debug dans la console      */

/* =====================================================================
   MACRO UTILITAIRE : CLAMP
   =====================================================================
   Force la valeur (v) a rester dans l'intervalle [lo , hi].
   Utilisee pour empecher les pastilles de sortir du bord de la minimap.

   Exemples :
     CLAMP(210, 5, 200) = 200   (trop grand  -> ramene a hi)
     CLAMP(-3,  5, 200) =   5   (trop petit  -> ramene a lo)
     CLAMP(100, 5, 200) = 100   (dans l'intervalle -> inchange)
   ===================================================================== */
#define CLAMP(v, lo, hi)  ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

/* =====================================================================
   [PRIVE] dessineTexte
   =====================================================================
   Affiche une chaine a l'ecran via SDL_ttf.
   "static" = visible uniquement dans ce fichier .c (fonction privee).
   Reproduit le style de renderText() de main.c (AzizRodesly).

   Flux SDL pour afficher du texte :
     1. TTF_RenderText_Solid -> SDL_Surface  (pixels en RAM)
     2. SDL_CreateTextureFromSurface -> SDL_Texture  (pixels en VRAM)
     3. SDL_RenderCopy -> dessine a l'ecran
     4. FreeSurface + DestroyTexture -> libere la memoire

   Parametres :
     renderer -> cible de rendu SDL (l'ecran ou une texture)
     font     -> police ouverte avec TTF_OpenFont()
     texte    -> chaine C  (ex: "P1", "LEVEL 1  MINI MAP")
     couleur  -> {R, G, B, A} chaque composante dans [0, 255]
     x, y     -> coin superieur gauche du texte en pixels ecran
   ===================================================================== */
static void dessineTexte(SDL_Renderer *renderer, TTF_Font *font,
                          const char *texte, SDL_Color couleur, int x, int y)
{
    /* Securite : ne pas appeler TTF ou SDL si les pointeurs sont NULL   */
    if (!font  || !texte) return;

    /* Etape 1 : texte -> surface (image en memoire vive, cote CPU)      */
    SDL_Surface *surf = TTF_RenderText_Solid(font, texte, couleur);
    if (!surf) return;   /* si la police est corrompue -> sortir proprement */

    /* Etape 2 : surface -> texture (image en memoire video, cote GPU)   */
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);

    /* Etape 3 : rectangle de destination = meme taille que le texte rendu */
    SDL_Rect dst = { x, y, surf->w, surf->h };

    /* Etape 4 : copier la texture sur la cible de rendu (l'ecran ici)  */
    SDL_RenderCopy(renderer, tex, NULL, &dst);

    /* Etape 5 : liberer les ressources -- OBLIGATOIRE (pas de GC en C) */
    SDL_FreeSurface(surf);    /* libere la RAM (surface CPU)             */
    SDL_DestroyTexture(tex);  /* libere la VRAM (texture GPU)            */
}

/* =====================================================================
   [PRIVE] dessinePastille
   =====================================================================
   Dessine un petit carre colore + contour noir representant un joueur
   sur la minimap.

   Parametres :
     renderer  -> cible de rendu SDL
     cx, cy    -> centre de la pastille en pixels ecran
     taille    -> cote du carre en pixels (recommande : 7 px)
     r, g, b   -> couleur RGB (Uint8 = entier non signe 0..255)
   ===================================================================== */
static void dessinePastille(SDL_Renderer *renderer,
                             int cx, int cy, int taille,
                             Uint8 r, Uint8 g, Uint8 b)
{
    /* Rectangle centre sur (cx, cy) :
       on recule de taille/2 pour obtenir le coin superieur gauche       */
    SDL_Rect carre = {
        cx - taille / 2,   /* X coin gauche  */
        cy - taille / 2,   /* Y coin haut    */
        taille,             /* largeur        */
        taille              /* hauteur        */
    };

    /* Remplissage plein avec la couleur du joueur (alpha 255 = opaque)  */
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &carre);

    /* Contour noir : lisible sur fond clair ET fond fonce               */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &carre);
}

/* =====================================================================
   [PRIVE] mondeVersMinimap
   =====================================================================
   Convertit une position "espace monde" en pixel dans la minimap.

   FORMULES :
     miniX = rect.x  +  worldX x scaleX
     miniY = rect.y  +  worldY x scaleY
   Puis CLAMP pour rester a l'interieur du rectangle (marge 3 px).

   Parametres :
     mm          -> structure MiniMap (rect, scaleX, scaleY)
     worldX      -> position X monde  dans [0 , bgW[
     worldY      -> position Y ecran  dans [0 , SCREEN_H_JEU[
     outX, outY  -> [sortie] pixels ecran a l'interieur de la minimap
   ===================================================================== */
static void mondeVersMinimap(const MiniMap *mm,
                               int worldX, int worldY,
                               int *outX, int *outY)
{
    /* Conversion horizontale : position monde -> pixel minimap          */
    *outX = mm->rect.x + (int)((float)worldX * mm->scaleX);

    /* Conversion verticale : coordonnee ecran -> pixel minimap          */
    *outY = mm->rect.y + (int)((float)worldY * mm->scaleY);

    /* Clamper X dans [rect.x+3 , rect.x+rect.w-3]
       La marge de 3 px garde la pastille (7x7) entierement visible     */
    *outX = CLAMP(*outX, mm->rect.x + 3, mm->rect.x + mm->rect.w - 3);

    /* Clamper Y dans [rect.y+3 , rect.y+rect.h-3]                      */
    *outY = CLAMP(*outY, mm->rect.y + 3, mm->rect.y + mm->rect.h - 3);
}

/* =====================================================================
   initMiniMap   (fonction PUBLIQUE)
   =====================================================================
   Initialise la structure MiniMap et pre-genere bgMini :
   une copie du fond complet (19 180 x 4 282 px) reduite une seule fois
   dans une texture 175 x 90 px.

   Appel dans main.c :
     initMiniMap(&minimap, renderer, gameBackground, bgWidth, bgHeight, 1);
   ===================================================================== */
void initMiniMap(MiniMap *mm, SDL_Renderer *renderer,
                 SDL_Texture *bgTex, int bgW, int bgH, int niveau)
{
    /* ------------------------------------------------------------------
       1. Geometrie de la minimap sur l'ecran
       Les constantes viennent de minimap.h (MM_X, MM_Y, MM_W, MM_H).
       ------------------------------------------------------------------ */
    mm->rect.x = MM_X;   /* coin gauche X  : 312 px                      */
    mm->rect.y = MM_Y;   /* coin haut   Y  :   5 px                      */
    mm->rect.w = MM_W;   /* largeur        : 175 px                      */
    mm->rect.h = MM_H;   /* hauteur        :  90 px                      */

    /* ------------------------------------------------------------------
       2. Dimensions du fond
       Protection contre division par zero si background non charge.
       ------------------------------------------------------------------ */
    mm->niveau = niveau;                 /* niveau 1 ou 2                */
    mm->bgW    = (bgW > 0) ? bgW : 800; /* largeur fond (19 180 px)      */
    mm->bgH    = (bgH > 0) ? bgH : 600; /* hauteur fond (4 282 px)       */

    /* ------------------------------------------------------------------
       3. Facteurs d'echelle
       ------------------------------------------------------------------ */

    /* scaleX = pixels minimap / pixels monde  (horizontal)
       Level 1 : 175 / 19 180 = 0.00912
       -> 1 000 px monde = 9.12 px minimap                               */
    mm->scaleX = (float)MM_W / (float)mm->bgW;

    /* scaleY mappe les coordonnees ECRAN (0-600) sur la minimap (0-90).
       On utilise SCREEN_H_JEU = 600, PAS bgH = 4 282.
       Raison : pos.y du joueur est toujours en pixels ecran (0-600).
       scaleY = 90 / 600 = 0.15                                          */
    mm->scaleY = (float)MM_H / (float)SCREEN_H_JEU;

    /* ------------------------------------------------------------------
       4. Creation de bgMini (fond miniaturise)
       On cree une texture "cible de rendu" de 175 x 90 px,
       on y copie le fond complet (SDL fait le scaling automatiquement),
       puis on revient au rendu normal vers l'ecran.
       bgMini est pre-calculee une seule fois -> affichage rapide.
       ------------------------------------------------------------------ */
    mm->bgMini = NULL;   /* valeur par defaut = pas de texture            */

    if (bgTex != NULL)   /* seulement si le background a ete charge       */
    {
        /* Creer une texture vide de taille MM_W x MM_H (175 x 90 px)   */
        mm->bgMini = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,   /* 4 octets par pixel, avec canal alpha */
            SDL_TEXTUREACCESS_TARGET,   /* peut etre utilisee comme cible de rendu */
            MM_W, MM_H                  /* 175 x 90 pixels                        */
        );

        if (mm->bgMini != NULL)
        {
            /* Rediriger le rendu vers bgMini (au lieu de l'ecran)       */
            SDL_SetRenderTarget(renderer, mm->bgMini);

            /* Copier bgTex (19 180 x 4 282) dans bgMini (175 x 90)
               src=NULL = tout le fond | dst=NULL = toute la texture cible
               SDL calcule et applique le ratio de reduction.            */
            SDL_RenderCopy(renderer, bgTex, NULL, NULL);

            /* Restaurer le rendu vers l'ecran principal                 */
            SDL_SetRenderTarget(renderer, NULL);

            /* Activer le blend pour SDL_SetTextureAlphaMod dans afficher */
            SDL_SetTextureBlendMode(mm->bgMini, SDL_BLENDMODE_BLEND);
        }
        else
        {
            /* Echec de creation (VRAM pleine ?) -> afficher l'erreur    */
            printf("[MiniMap] ERREUR SDL_CreateTexture : %s\n", SDL_GetError());
        }
    }

    /* ------------------------------------------------------------------
       5. Message de confirmation (pratique lors de l'integration)
       ------------------------------------------------------------------ */
    printf("[MiniMap] Init OK | niveau=%d | fond=%dx%d"
           " | minimap=%dx%d | scaleX=%.5f | scaleY=%.4f\n",
           niveau, mm->bgW, mm->bgH, MM_W, MM_H, mm->scaleX, mm->scaleY);
}

/* =====================================================================
   afficherMiniMap   (fonction PUBLIQUE)
   =====================================================================
   Dessine la minimap complete a chaque frame.

   Ordre de rendu (fond -> premier plan) :
     1. Fond miniaturise (bgMini), semi-transparent
     2. Ligne du sol (verte)
     3. Rectangles de camera (zones visibles des joueurs)
     4. Pastilles des joueurs (carres colores)
     5. Labels "P1" / "P2"
     6. Cadre de la minimap
     7. Label du niveau au-dessus

   Appel dans main.c (juste avant SDL_RenderPresent) :
     afficherMiniMap(&minimap, renderer, font,
                     &player1, bgOffset1,
                     &player2, bgOffset2, isMultiplayer);
   ===================================================================== */
void afficherMiniMap(MiniMap *mm, SDL_Renderer *renderer,
                     TTF_Font *font,
                     Joueur *p1, int off1,
                     Joueur *p2, int off2,
                     int isMulti)
{
    /* Securite : sortir proprement si les pointeurs essentiels sont NULL */
    if (!mm || !renderer) return;

    /* Alias locaux pour alleger les ecritures dans cette fonction       */
    int mx = mm->rect.x;   /* X gauche minimap = 312                     */
    int my = mm->rect.y;   /* Y haut   minimap =   5                     */
    int mw = mm->rect.w;   /* largeur  minimap = 175                     */
    int mh = mm->rect.h;   /* hauteur  minimap =  90                     */

    /* Activer le mode de fusion (blending) pour les rectangles semi-
       transparents. Sans cela la valeur alpha de SetRenderDrawColor
       est ignoree et tout s'affiche pleinement opaque.                  */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    /* ------------------------------------------------------------------
       ETAPE 1 : Fond miniaturise (bgMini)
       ------------------------------------------------------------------ */
    if (mm->bgMini != NULL)
    {
        /* Opacite 180/255 = ~70 % : fond visible sans etre trop charge  */
        SDL_SetTextureAlphaMod(mm->bgMini, 180);

        /* Dessiner bgMini exactement dans le rectangle de la minimap    */
        SDL_RenderCopy(renderer, mm->bgMini, NULL, &mm->rect);
    }
    else
    {
        /* Pas de texture -> fond uni bleu fonce semi-transparent         */
        SDL_SetRenderDrawColor(renderer, 10, 10, 50, 210);
        SDL_RenderFillRect(renderer, &mm->rect);
    }

    /* ------------------------------------------------------------------
       ETAPE 2 : Ligne du sol (verte)
       Dans joueur.c le sol est a y = 500 px (coordonnees ecran).
       solY_mini = rect.y + 500 x scaleY = 5 + 500 x 0.15 = 80 px
       ------------------------------------------------------------------ */
    int solY = my + (int)(500.0f * mm->scaleY);   /* Y sol sur minimap   */
    SDL_SetRenderDrawColor(renderer, 60, 200, 60, 200);
    SDL_RenderDrawLine(renderer, mx, solY, mx + mw, solY);

    /* ------------------------------------------------------------------
       ETAPE 3 : Rectangles de camera (zones visibles)

       bgOffset = pixels du fond deja defiles a gauche de l'ecran.
       La camera montre :
         800 px du fond en solo   (fenetre entiere)
         400 px du fond en multi  (split-screen : chaque joueur a 1/2)

       Sur la minimap (scaleX ~ 0.00912) :
         Solo  : 800 x 0.00912 ~ 7 px  -> minimum impose a 8 px
         Multi : 400 x 0.00912 ~ 4 px  -> minimum impose a 4 px

       WRAP MIROIR : off1 appartient [0 , bgW x 2[
       On prend off1 % bgW pour revenir dans [0 , bgW[.
       ------------------------------------------------------------------ */
    int camMiniW = (int)((isMulti ? 400 : 800) * mm->scaleX);
    if (camMiniW < 8) camMiniW = 8;   /* minimum visible                 */

    /* ---- Camera joueur 1 -- rectangle JAUNE ---- */
    int camWorldX1 = off1 % mm->bgW;
    if (camWorldX1 < 0) camWorldX1 += mm->bgW;   /* securite si negatif */

    int camMiniX1 = mx + (int)((float)camWorldX1 * mm->scaleX);
    camMiniX1 = CLAMP(camMiniX1, mx, mx + mw - camMiniW);

    SDL_Rect camRect1 = { camMiniX1, my, camMiniW, mh };
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 40);    /* jaune leger  */
    SDL_RenderFillRect(renderer, &camRect1);
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 220);   /* jaune visible */
    SDL_RenderDrawRect(renderer, &camRect1);

    /* ---- Camera joueur 2 -- rectangle BLEU CLAIR (multi seulement) ---- */
    if (isMulti && p2 != NULL)
    {
        int camWorldX2 = off2 % mm->bgW;
        if (camWorldX2 < 0) camWorldX2 += mm->bgW;

        int camMiniX2 = mx + (int)((float)camWorldX2 * mm->scaleX);
        camMiniX2 = CLAMP(camMiniX2, mx, mx + mw - camMiniW);

        SDL_Rect camRect2 = { camMiniX2, my, camMiniW, mh };
        SDL_SetRenderDrawColor(renderer, 80, 180, 255, 40);   /* bleu leger  */
        SDL_RenderFillRect(renderer, &camRect2);
        SDL_SetRenderDrawColor(renderer, 80, 180, 255, 220);  /* bleu visible */
        SDL_RenderDrawRect(renderer, &camRect2);
    }

    /* ------------------------------------------------------------------
       ETAPE 4 : Pastilles des joueurs

       Position monde du centre du joueur :
         worldX = (bgOffset + pos.x_ecran + pos.w/2) % bgW
         worldY =  pos.y_ecran + pos.h/2  (coordonnee ecran directe)
       ------------------------------------------------------------------ */

    /* ---- Pastille joueur 1 -- ROUGE ---- */
    if (p1 != NULL)
    {
        /* Centre horizontal dans l'espace monde                         */
        int worldX1 = (off1 + p1->pos.x + p1->pos.w / 2) % mm->bgW;
        if (worldX1 < 0) worldX1 += mm->bgW;

        /* Centre vertical en coordonnees ecran                          */
        int worldY1 = p1->pos.y + p1->pos.h / 2;

        int dotX1, dotY1;
        mondeVersMinimap(mm, worldX1, worldY1, &dotX1, &dotY1);

        dessinePastille(renderer, dotX1, dotY1, 7, 255, 60, 60);   /* rouge */
    }

    /* ---- Pastille joueur 2 -- BLEU (multi seulement) ---- */
    if (isMulti && p2 != NULL)
    {
        int worldX2 = (off2 + p2->pos.x + p2->pos.w / 2) % mm->bgW;
        if (worldX2 < 0) worldX2 += mm->bgW;

        int worldY2 = p2->pos.y + p2->pos.h / 2;

        int dotX2, dotY2;
        mondeVersMinimap(mm, worldX2, worldY2, &dotX2, &dotY2);

        dessinePastille(renderer, dotX2, dotY2, 7, 60, 100, 255);  /* bleu */
    }

    /* ------------------------------------------------------------------
       ETAPE 5 : Labels "P1" / "P2" a gauche du cadre
       ------------------------------------------------------------------ */
    if (font != NULL)
    {
        SDL_Color rouge = { 255,  80,  80, 255 };
        SDL_Color bleuC = {  80, 100, 255, 255 };

        /* "P1" : 24 px a gauche, centre verticalement                  */
        dessineTexte(renderer, font, "P1", rouge, mx - 24, my + mh / 2 - 16);

        /* "P2" : juste en dessous, mode multi uniquement               */
        if (isMulti)
            dessineTexte(renderer, font, "P2", bleuC, mx - 24, my + mh / 2);
    }

    /* ------------------------------------------------------------------
       ETAPE 6 : Cadre de la minimap
       Blanc interieur + fonce exterieur = effet relief
       ------------------------------------------------------------------ */
    SDL_SetRenderDrawColor(renderer, 220, 220, 255, 255);
    SDL_RenderDrawRect(renderer, &mm->rect);                /* bord blanc */

    SDL_Rect bordure = { mx - 1, my - 1, mw + 2, mh + 2 };
    SDL_SetRenderDrawColor(renderer, 50, 50, 120, 255);
    SDL_RenderDrawRect(renderer, &bordure);                 /* bord fonce */

    /* ------------------------------------------------------------------
       ETAPE 7 : Label du niveau (au-dessus, y = my - 18)
       ------------------------------------------------------------------ */
    if (font != NULL)
    {
        char label[32];
        sprintf(label, "LEVEL %d  MINI MAP", mm->niveau);

        SDL_Color jaune = { 255, 220, 0, 255 };
        dessineTexte(renderer, font, label, jaune, mx, my - 18);
    }
    else
    {
        /* Sans police : bandeau de couleur a la place du texte          */
        SDL_Rect bandeau = { mx, my - 10, mw, 8 };
        if (mm->niveau == 1)
            SDL_SetRenderDrawColor(renderer, 255, 160,   0, 220);
        else
            SDL_SetRenderDrawColor(renderer, 0,   200, 180, 220);
        SDL_RenderFillRect(renderer, &bandeau);
    }
}

/* =====================================================================
   freeMiniMap   (fonction PUBLIQUE)
   =====================================================================
   Detruit bgMini et remet le pointeur a NULL.
   A appeler une seule fois en fin de programme.
   ===================================================================== */
void freeMiniMap(MiniMap *mm)
{
    if (mm == NULL) return;   /* securite : mm NULL -> rien a faire       */

    if (mm->bgMini != NULL)
    {
        SDL_DestroyTexture(mm->bgMini);   /* libere la VRAM              */
        mm->bgMini = NULL;                /* NULL -> evite un double-free */
    }

    printf("[MiniMap] Texture liberee.\n");
}
