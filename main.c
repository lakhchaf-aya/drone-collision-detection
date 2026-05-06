

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>

// Structures de donnees
 
struct Drone { int id; float x; float y; float z; };
struct Paire { struct Drone a; struct Drone b; float distance; };


// Declarations de fonctions
int           generer_essaim(struct Drone **essaim, int n);
int           saisir_essaim(struct Drone **essaim);
void          afficher_essaim(struct Drone *essaim, int n);
float         distance_euclidienne(const struct Drone *a, const struct Drone *b);
void          echanger_drones(struct Drone *a, struct Drone *b);
int           partition_qs(struct Drone *e, int bas, int haut);
void          quicksort_par_x(struct Drone *e, int bas, int haut);
struct Paire  bruteforce(struct Drone *e, int debut, int fin);
struct Paire  bande(struct Drone *b, int tb, struct Paire m);
struct Paire  diviser_regner(struct Drone *e, int debut, int fin);
void          afficher_resultat(struct Paire p, double ms, int n);
void          ligne(char c, int n);

//Utilitaire : trace une ligne de n caracteres c
 
void ligne(char c, int n) { int i; for(i=0;i<n;i++) putchar(c); putchar('\n'); }

//MAIN
int main(void)
{
    struct Drone *essaim = NULL;
    struct Paire  res;
    int n, choix; clock_t t0, t1; double ms;

    ligne('=',62);
    printf("  SYSTEME DE DETECTION DE COLLISION -- ESSAIM UAV\n");
    ligne('=',62);
    printf("\n  Mode d'entree des donnees :\n");
    printf("    [1] Saisie manuelle des coordonnees\n");
    printf("    [2] Generation aleatoire (simulation grande echelle)\n");
    printf("\n  Votre choix : ");
    fflush(stdout);

    if(scanf("%d",&choix)!=1) choix=1;
    while(getchar()!='\n');   /* vider le buffer apres le choix */

    if(choix == 2){
        /* 
         * MODE GENERATION ALEATOIRE
         * Permet de simuler un essaim de 10 000 drones
         * (ou toute taille souhaitee) sans saisie manuelle.
        */
        printf("  Nombre de drones a generer (min 2) : ");
        fflush(stdout);
        if(scanf("%d",&n)!=1 || n<2){
            printf("\n  [ERREUR] Invalide (min 2).\n");
            return 1;
        }
        if(generer_essaim(&essaim, n) < 0){
            printf("\n  [ERREUR] Generation echouee.\n");
            free(essaim);
            return 1;
        }
    } else {
        /* MODE SAISIE MANUELLE */
        n = saisir_essaim(&essaim);
        if(n < 2 || essaim == NULL){
            printf("\n  [ERREUR] Minimum 2 drones.\n");
            free(essaim);
            return 1;
        }
    }

    /* Affichage recapitulatif (limite a 20 drones pour lisibilite) */
    if(n <= 20){
        printf("\n");
        afficher_essaim(essaim, n);
    } else {
        printf("\n  [INFO] Essaim de %d drones charge (affichage omis).\n", n);
    }

    printf("\n  [1/2] Tri QuickSort (axe X)    ...");
    t0 = clock();
    quicksort_par_x(essaim, 0, n-1);
    printf(" OK\n");

    printf("  [2/2] Recherche paire minimale ...");
    res = diviser_regner(essaim, 0, n-1);
    t1  = clock();
    ms  = ((double)(t1-t0) / CLOCKS_PER_SEC) * 1000.0;
    printf(" OK\n");

    afficher_resultat(res, ms, n);
    free(essaim);
    return 0;
}

/* 
 *  GENERATION ALEATOIRE (CORRECTION 1 -- simulation grande echelle)
 *  Genere n drones avec des coordonnees float aleatoires
 *  dans le cube [-500 m, +500 m]^3.
 *  Utilise l'arithmetique de pointeurs -- aucun crochet.
 */
int generer_essaim(struct Drone **essaim, int n)
{
    int i; struct Drone *ptr;
    srand((unsigned int)time(NULL));

    *essaim = (struct Drone *)malloc(n * sizeof(struct Drone));
    if(!*essaim){ printf("\n  [ERREUR] malloc echoue.\n"); return -1; }

    for(i=0; i<n; i++){
        ptr = *essaim + i;          /* arithmetique de pointeurs -- pas de [] */
        ptr->id = i;
        ptr->x  = -500.0f + (float)rand() / (float)(RAND_MAX / 1000.0f);
        ptr->y  = -500.0f + (float)rand() / (float)(RAND_MAX / 1000.0f);
        ptr->z  = -500.0f + (float)rand() / (float)(RAND_MAX / 1000.0f);
    }
    printf("\n  [OK] %d drones generes aleatoirement dans [-500,+500]^3.\n", n);
    return n;
}

// SAISIE MANUELLE

int saisir_essaim(struct Drone **essaim)
{
    int n, i; struct Drone *ptr;
    printf("\n"); ligne('-',62);
    printf("  Combien de drones dans l'essaim ? ");
    fflush(stdout);
    if(scanf("%d",&n)!=1 || n<2){ printf("\n  [ERREUR] Invalide (min 2).\n"); return -1; }
    while(getchar()!='\n');   /* vider le buffer */

    *essaim = (struct Drone *)malloc(n * sizeof(struct Drone));
    if(!*essaim){ printf("\n  [ERREUR] malloc echoue.\n"); return -1; }

    printf("\n"); ligne('-',62);
    printf("  Saisie des coordonnees 3D pour chaque drone\n"); ligne('-',62);

    for(i=0; i<n; i++){
        ptr = *essaim + i;          /* arithmetique de pointeurs - pas de [] */
        ptr->id = i;
        printf("\n  Drone #%d\n    X (metres) : ", i);
        fflush(stdout);
        while(scanf("%f",&ptr->x)!=1){ printf("    Invalide. X : "); fflush(stdout); while(getchar()!='\n'); }
        printf("    Y (metres) : ");
        fflush(stdout);
        while(scanf("%f",&ptr->y)!=1){ printf("    Invalide. Y : "); fflush(stdout); while(getchar()!='\n'); }
        printf("    Z (metres) : ");
        fflush(stdout);
        while(scanf("%f",&ptr->z)!=1){ printf("    Invalide. Z : "); fflush(stdout); while(getchar()!='\n'); }
        printf("    --> Drone #%d : (%.2f, %.2f, %.2f)\n", i, ptr->x, ptr->y, ptr->z);
    }
    return n;
}

// AFFICHAGE DE L'ESSAIM
 
void afficher_essaim(struct Drone *essaim, int n)
{
    int i; struct Drone *ptr;
    ligne('=',62);
    printf("  RECAPITULATIF -- %d DRONES ENREGISTRES\n", n);
    ligne('=',62);
    printf("  %-8s  %-6s  %-12s  %-12s  %-12s\n","DRONE","ID","X (m)","Y (m)","Z (m)");
    ligne('-',62);
    for(i=0; i<n; i++){
        ptr = essaim + i;           /* arithmetique de pointeurs -- pas de [] */
        printf("  %-8s  %-6d  %-12.4f  %-12.4f  %-12.4f\n",
               "Drone", ptr->id, ptr->x, ptr->y, ptr->z);
    }
    ligne('-',62);
}

/* ============================================================
 *  DISTANCE EUCLIDIENNE 3D
 * ============================================================ */
float distance_euclidienne(const struct Drone *a, const struct Drone *b)
{
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

// ECHANGE DE DEUX DRONES (utilisee par QuickSort)

void echanger_drones(struct Drone *a, struct Drone *b)
{
    struct Drone t = *a; *a = *b; *b = t;
}

//  QUICKSORT -- partition (pivot = dernier element)
 
int partition_qs(struct Drone *e, int bas, int haut)
{
    float px = (e+haut)->x;
    int i = bas-1, j;
    for(j=bas; j<haut; j++)
        if((e+j)->x <= px){ i++; echanger_drones(e+i, e+j); }
    echanger_drones(e+i+1, e+haut);
    return i+1;
}

//QUICKSORT -- tri recursif sur l'axe X  O(n log n) moyen
 
void quicksort_par_x(struct Drone *e, int bas, int haut)
{
    int p;
    if(bas < haut){
        p = partition_qs(e, bas, haut);
        quicksort_par_x(e, bas,   p-1);
        quicksort_par_x(e, p+1, haut);
    }
}

/* 
 *  BRUTEFORCE -- paire la plus proche sur un sous-tableau
 *
 *  CORRECTION 2 : guard explicite sur les cas degeneres.
 *    - debut == fin   : 1 seul drone, distance infinie
 *    - debut+1 == fin : exactement 2 drones, calcul direct
 *  Evite l'acces hors-limite a *(e+debut+1) quand debut==fin.
 */
struct Paire bruteforce(struct Drone *e, int debut, int fin)
{
    struct Paire m;
    float dmin, d;
    int i, j;
    struct Drone *di, *dj;

    /* Cas degenere : un seul drone -- distance infinie, paire vide */
    if(debut == fin){
        m.a        = *(e+debut);
        m.b        = *(e+debut);
        m.distance = FLT_MAX;
        return m;
    }

    // Initialisation avec la premiere paire valide 
    m.a        = *(e+debut);
    m.b        = *(e+debut+1);
    m.distance = distance_euclidienne(e+debut, e+debut+1);
    dmin       = m.distance;

    for(i=debut; i<=fin; i++){
        di = e+i;
        for(j=i+1; j<=fin; j++){
            dj = e+j;
            d  = distance_euclidienne(di, dj);
            if(d < dmin){ dmin = d; m.a = *di; m.b = *dj; m.distance = d; }
        }
    }
    return m;
}

/* 
 *  BANDE -- affine le minimum dans la bande verticale
 *  Tri par insertion sur la bande (O b^2 dans le pire cas,
 *  O b log b en moyenne via la propriete geometrique :
 *  au plus 8 points par carre delta x delta en 2D, ~O(b)).
 *  Pour un essaim totalement aleatoire la bande reste petite.
 */
struct Paire bande(struct Drone *b, int tb, struct Paire m)
{
    float d;
    int i, j, k, p;
    struct Drone tmp, *di, *dj;

    /* Tri par Y -- tri par insertion O(tb^2) */
    for(k=1; k<tb; k++){
        tmp = *(b+k);
        p   = k-1;
        while(p>=0 && (b+p)->y > tmp.y){ *(b+p+1) = *(b+p); p--; }
        *(b+p+1) = tmp;
    }

    /*
     * Propriete geometrique (Shamos-Hoey) : dans la bande triee par Y,
     * seuls les points dont la difference en Y est inferieure a m.distance
     * peuvent battre le minimum courant. En pratique <= 7 comparaisons
     * par point en 2D ; en 3D le nombre reste borne.
     */
    for(i=0; i<tb; i++){
        di = b+i;
        for(j=i+1; j<tb && ((b+j)->y - di->y) < m.distance; j++){
            dj = b+j;
            d  = distance_euclidienne(di, dj);
            if(d < m.distance){ m.a = *di; m.b = *dj; m.distance = d; }
        }
    }
    return m;
}

/* 
 *  DIVISER-POUR-REGNER -- algorithme principal  O(n log n)
 *
 *  CORRECTION 2 (suite) : le seuil de bascule vers bruteforce
 *  est porte a fin-debut < 3 (au lieu de <=2) et la fonction
 *  bruteforce gere desormais le cas 1 drone sans acces hors-limite.
 */
struct Paire diviser_regner(struct Drone *e, int debut, int fin)
{
    struct Paire g, dr, m;
    struct Drone *b, *ps;
    float xm;
    int mid, tb, cap, k;

    /*
     * Cas de base : 3 elements ou moins --> brute-force directe.
     * bruteforce() gere maintenant le cas debut==fin sans UB.
     */
    if(fin - debut < 3) return bruteforce(e, debut, fin);

    mid = (debut + fin) / 2;
    xm  = (e+mid)->x;

    /* Sous-probleme gauche et droit */
    g  = diviser_regner(e, debut,   mid);
    dr = diviser_regner(e, mid+1,   fin);
    m  = (g.distance < dr.distance) ? g : dr;

    /* Construction de la bande : points a moins de m.distance de xm */
    cap = fin - debut + 1;
    b   = (struct Drone *)malloc(cap * sizeof(struct Drone));
    if(!b) return m;     /* si malloc echoue, on retourne le meilleur connu */

    tb = 0;
    for(k=debut; k<=fin; k++){
        ps = e+k;                   /* arithmetique de pointeurs -- pas de [] */
        if(fabsf(ps->x - xm) < m.distance){ *(b+tb) = *ps; tb++; }
    }

    /*
     * bande() retourne une Paire par VALEUR (copie complete des champs).
     * Apres free(b), m reste valide : elle ne contient aucun pointeur
     * vers le tampon b, uniquement des struct Drone copies.
     */
    m = bande(b, tb, m);
    free(b); b = NULL;
    return m;
}

//  AFFICHAGE DU RAPPORT FINAL
 
void afficher_resultat(struct Paire p, double ms, int n)
{
    ligne('=',62);
    printf("  RAPPORT DE SECURITE -- ALERTE COLLISION\n");
    ligne('=',62);
    printf("  %-8s  %-6s  %-12s  %-12s  %-12s\n","DRONE","ID","X (m)","Y (m)","Z (m)");
    ligne('-',62);
    printf("  %-8s  %-6d  %-12.4f  %-12.4f  %-12.4f\n",
           "Drone A", p.a.id, p.a.x, p.a.y, p.a.z);
    printf("  %-8s  %-6d  %-12.4f  %-12.4f  %-12.4f\n",
           "Drone B", p.b.id, p.b.x, p.b.y, p.b.z);
    ligne('-',62);
    printf("  %-30s : %.6f m\n",  "Distance minimale",    p.distance);
    printf("  %-30s : %.4f ms\n", "Temps d'execution",    ms);
    printf("  %-30s : %d\n",      "Taille essaim",         n);
    printf("  %-30s : %s\n",      "Algorithme",
           n <= 3 ? "Brute-force (n<=3)"
                  : "QuickSort + Diviser-pour-Regner");
    printf("  %-30s : O(n log n)\n", "Complexite globale");
    ligne('=',62);
    printf("  ACTION : Manouvre d'evitement declenchee\n");
    printf("           Drones #%d et #%d mis en securite\n", p.a.id, p.b.id);
    ligne('=',62);
}