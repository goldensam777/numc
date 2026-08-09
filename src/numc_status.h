/*
 * numc_status.h
 *
 * Convention de gestion d'erreur de numc.
 *
 * Toute fonction de la bibliotheque retourne un numc_status_t
 * (NUMC_OK en cas de succes) et ecrit ses resultats dans des
 * parametres de sortie. Le dernier statut rencontre est conserve
 * dans un etat global thread-local et interrogeable via
 * numc_error() / numc_last_status().
 *
 * Exemple :
 *   double a;
 *   if (numc_vec_dot(&a, u, v) != NUMC_OK) {
 *       fprintf(stderr, "numc: %s\n", numc_error());
 *   }
 */


#ifndef NUMC_STATUS_H
#define NUMC_STATUS_H


/* Statuts retournes par les fonctions de la bibliotheque */
typedef enum numc_status
{
    NUMC_OK = 0,                 /* succes */
    NUMC_ERR_INVALID_ARG,        /* argument invalide (NULL, dimension, ...) */
    NUMC_ERR_ALLOC,              /* echec d'allocation memoire */
    NUMC_ERR_DOMAIN,             /* valeur hors du domaine de definition */
    NUMC_ERR_OVERFLOW,           /* depassement superieur */
    NUMC_ERR_UNDERFLOW,          /* depassement inferieur */
    NUMC_ERR_SINGULAR,           /* matrice / donnee singuliere */
    NUMC_ERR_DIVERGENCE,         /* algorithme iteratif qui ne converge pas */
    NUMC_ERR_NOT_IMPLEMENTED,    /* fonctionnalite prevue mais non ecrite */
    NUMC_ERR_INTERNAL,           /* erreur interne inattendue */
    NUMC_ERR_DIM_MISMATCH,       /* dimensions incompatibles entre operandes */
    NUMC_ERR_OUT_OF_BOUNDS,      /* indice hors bornes */

} numc_status_t;


/* Renvoie le dernier statut enregistre par la bibliotheque. */
numc_status_t numc_last_status(void);

/* Dernier message d'erreur (texte libre), ou NULL si aucune erreur. */
const char *numc_error(void);

/* Message lisible associe a un statut. */
const char *numc_status_string(numc_status_t status);

/* Usage interne : enregistre le statut et un message d'erreur. */
void numc_set_error(numc_status_t status, const char* format, ...);


#endif /* NUMC_STATUS_H */
