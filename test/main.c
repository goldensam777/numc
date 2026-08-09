/*
 * main.c
 *
 * Execution des tests de numc, via cmocka.
 *
 * Chaque module de tests expose un tableau `const struct CMUnitTest`
 * (ex. numc_status_tests). Ajoute son groupe ici au fur et a mesure.
 */


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test_status.h"


int
main(void)
{
    int failures = 0;

    failures += cmocka_run_group_tests(numc_status_tests, NULL, NULL);

    /* Ajoute ici tes prochains groupes de tests, ex. :
     * failures += cmocka_run_group_tests(numc_complex_tests, NULL, NULL);
     */

    return failures;
}
