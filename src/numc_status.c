/*
 * numc_status.c
 *
 * Implementation du module de gestion d'erreur de numc.
 *
 * L'etat global (dernier statut + message) est _Thread_local
 * (C11) : chaque thread de l'appelant garde son propre etat.
 */


#include "numc_status.h"

#include <stdarg.h>
#include <stdio.h>


#define NUMC_ERROR_MSG_CAP 256


/* Etat global thread-local de la bibliotheque */
static _Thread_local numc_status_t numc_tl_status = NUMC_OK;
static _Thread_local char numc_tl_message[NUMC_ERROR_MSG_CAP];


numc_status_t
numc_last_status(void)
{
    return numc_tl_status;
}


const char *
numc_error(void)
{
    return (numc_tl_status == NUMC_OK) ? NULL : numc_tl_message;
}


void
numc_set_error(numc_status_t status, const char* format, ...)
{
    numc_tl_status = status;
    numc_tl_message[0] = '\0';

    if (status == NUMC_OK || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    vsnprintf(numc_tl_message, sizeof numc_tl_message, format, args);
    va_end(args);
}


const char *
numc_status_string(numc_status_t status)
{
    switch (status) {
        case NUMC_OK:                 return "success";
        case NUMC_ERR_INVALID_ARG:    return "invalid argument";
        case NUMC_ERR_ALLOC:          return "allocation failure";
        case NUMC_ERR_DOMAIN:         return "domain error";
        case NUMC_ERR_OVERFLOW:       return "overflow";
        case NUMC_ERR_UNDERFLOW:      return "underflow";
        case NUMC_ERR_SINGULAR:       return "singular value";
        case NUMC_ERR_DIVERGENCE:     return "iterative algorithm diverged";
        case NUMC_ERR_NOT_IMPLEMENTED: return "not implemented";
        case NUMC_ERR_INTERNAL:       return "internal error";
        default:                      return "unknown status";
    }
}
