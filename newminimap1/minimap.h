/* =====================================================================
   minimap.h  —  Déclarations publiques de la mini-carte
   =====================================================================

   RÔLE DE CE FICHIER
   ------------------
   Un fichier .h (header) ne contient QUE des déclarations :
     • les #define (constantes)
     • le typedef struct  (structure de données)
     • les prototypes de fonctions (signatures sans corps)

   Il ne contient JAMAIS de code exécutable.
   Le code est dans minimap.c.

   INCLURE CE FICHIER
   ------------------
   Dans main.c     →  #include "minimap.h"
   Dans minimap.c  →  #include "minimap.h"  (il s'inclut lui-même)

   ARCHITECTURE DU PROJET
   ----------------------
   joueur.h   ←── minimap.h ←── minimap.c
                              ←── main.c
   ===================================================================== */

#ifndef MINIMAP_H   /* garde d'inclusion : si MINIMAP_H n'est pas défini …  */
#define MINIMAP_H   /* … le définir maintenant et lire tout ce qui suit      */
                    /* Si on #include "minimap.h" deux fois, la 2e fois      */
                    /* MINIMAP_H est déjà défini → le fichier entier est sauté */

/* -------------------------------------------------------------------
   DÉPENDANCES SDL2
   Ces includes amènent les types SDL_Renderer, SDL_Texture, SDL_Rect,
   TTF_Font que la structure MiniMap et les prototypes utilisent.
   ------------------------------------------------------------------- */
#include <SDL2/SDL.h>         /* types de base SDL2 : SDL_Renderer, SDL_Rect … */
#include <SDL2/SDL_image.h>   /* IMG_Load pour charger les PNG                  */
#include <SDL2/SDL_ttf.h>     /* TTF_Font pour le rendu de texte                */

/* -------------------------------------------------------------------
   DÉPENDANCE PROJET : structure Joueur
   minimap.h a besoin de connaître la struct Joueur pour déclarer les
   prototypes qui reçoivent Joueur* en paramètre.
   ------------------------------------------------------------------- */
#include "joueur.h"   /* définit typedef struct { SDL_Rect pos; … } Joueur; */

/* ===================================================================
   SECTION 1 — CONSTANTES DE POSITION ET TAILLE DE LA MINIMAP
   ===================================================================
   La fenêtre de jeu fait 800 × 600 px.
   La minimap est placée en haut au centre pour ne pas couvrir
   les HUD des joueurs (score, vies) situés en haut à gauche/droite.
   =================================================================== */

#define MM_X   312   /* pixel X du coin gauche de la minimap sur l'écran   */
                     /* 312 = (800 - 175) / 2 + 1 ≈ centré horizontalement  */

#define MM_Y     5   /* pixel Y du coin haut  de la minimap sur l'écran    */
                     /* 5 = juste sous le bord haut de la fenêtre           */

#define MM_W   175   /* largeur  de la minimap en pixels                    */
                     /* assez large pour voir la progression sur 19 180 px  */

#define MM_H    90   /* hauteur  de la minimap en pixels                    */
                     /* proportionnel à la fenêtre : 90/600 = 15%           */

/* -------------------------------------------------------------------
   CONSTANTE DE HAUTEUR ÉCRAN
   Le joueur se déplace en coordonnées ÉCRAN (pos.y ∈ [0, 600]).
   On utilise cette valeur pour calculer scaleY dans initMiniMap.
   NE PAS utiliser bgH (hauteur du PNG = 4282 px) : c'est la résolution
   de l'image, pas la hauteur de déplacement du joueur.
   ------------------------------------------------------------------- */
#define SCREEN_H_JEU  600   /* hauteur de la fenêtre SDL en pixels */

/* ===================================================================
   SECTION 2 — STRUCTURE MiniMap
   ===================================================================
   Un typedef struct regroupe toutes les données dont la minimap a
   besoin. On passe toujours un pointeur MiniMap* entre les fonctions
   pour éviter de copier toute la structure sur la pile.
   =================================================================== */
typedef struct
{
    /* --- Géométrie sur l'écran --- */
    SDL_Rect rect;      /* rectangle {x=MM_X, y=MM_Y, w=MM_W, h=MM_H}     */
                        /* définit où la minimap est dessinée sur l'écran  */

    /* --- Texture miniaturisée du fond --- */
    SDL_Texture *bgMini; /* copie du fond réduite à MM_W × MM_H pixels     */
                         /* créée une fois dans initMiniMap, détruite dans  */
                         /* freeMiniMap                                     */

    /* --- Informations sur le niveau --- */
    int niveau;         /* numéro du niveau en cours : 1 ou 2              */
    int bgW;            /* largeur réelle du fond PNG (ex : 19 180 px)     */
    int bgH;            /* hauteur réelle du fond PNG (ex :  4 282 px)     */

    /* --- Facteurs de mise à l'échelle monde → minimap --- */
    float scaleX;       /* = MM_W / bgW  (ex: 175/19180 ≈ 0.00912)        */
                        /* multiplie une coordonnée monde X pour obtenir   */
                        /* la position correspondante sur la minimap        */

    float scaleY;       /* = MM_H / SCREEN_H_JEU  (= 90/600 = 0.15)      */
                        /* multiplie une coordonnée écran Y (0-600) pour   */
                        /* obtenir la position correspondante sur la minimap */

} MiniMap;

/* ===================================================================
   SECTION 3 — PROTOTYPES DES FONCTIONS PUBLIQUES
   ===================================================================
   Un prototype déclare la signature d'une fonction (son nom, ses
   paramètres, son type de retour) SANS écrire son corps.
   Le corps est dans minimap.c.
   main.c a besoin des prototypes pour savoir comment appeler les
   fonctions avant que minimap.c soit compilé.
   =================================================================== */

/* -------------------------------------------------------------------
   initMiniMap
   -------------------------------------------------------------------
   Initialise la structure MiniMap et crée bgMini (fond miniaturisé).
   À appeler UNE SEULE FOIS, après la création du renderer et le
   chargement du background, dans main.c.

   mm       → adresse de la variable MiniMap déclarée dans main.c
   renderer → renderer SDL2 (SDL_CreateRenderer)
   bgTex    → texture du fond (gameBackground dans main.c)
   bgW      → largeur  du fond en pixels (19 180 pour Level 1)
   bgH      → hauteur  du fond en pixels ( 4 282 pour Level 1)
   niveau   → 1 ou 2 (affiché dans le label de la minimap)
   ------------------------------------------------------------------- */
void initMiniMap(MiniMap *mm, SDL_Renderer *renderer,
                 SDL_Texture *bgTex, int bgW, int bgH, int niveau);

/* -------------------------------------------------------------------
   afficherMiniMap
   -------------------------------------------------------------------
   Dessine la minimap complète (fond, cameras, pastilles, labels).
   À appeler à CHAQUE FRAME, après le rendu du fond et des joueurs,
   juste avant SDL_RenderPresent(renderer).

   mm      → adresse de la MiniMap initialisée
   renderer→ renderer SDL2
   font    → police TTF (peut être NULL : la minimap s'adapte)
   p1      → pointeur vers le joueur 1  (&player1 dans main.c)
   off1    → bgOffset1 de main.c (défilement caméra joueur 1)
   p2      → pointeur vers le joueur 2  (&player2 dans main.c)
   off2    → bgOffset2 de main.c (défilement caméra joueur 2)
   isMulti → 1 = mode multijoueur, 0 = mode solo
   ------------------------------------------------------------------- */
void afficherMiniMap(MiniMap *mm, SDL_Renderer *renderer,
                     TTF_Font *font,
                     Joueur *p1, int off1,
                     Joueur *p2, int off2,
                     int isMulti);

/* -------------------------------------------------------------------
   freeMiniMap
   -------------------------------------------------------------------
   Libère la texture bgMini allouée dans initMiniMap.
   À appeler à la fin du programme, avant SDL_DestroyRenderer().

   mm → adresse de la MiniMap à libérer
   ------------------------------------------------------------------- */
void freeMiniMap(MiniMap *mm);

#endif /* MINIMAP_H — fin de la garde d'inclusion */
