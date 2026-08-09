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

#include "test_complex.h"
#include "test_status.h"


int
main(void)
{
    int failures = 0;

    failures += numc_status_tests_run();
    failures += numc_complex_tests_run();

    /* Ajoute ici tes prochains groupes de tests, ex. :
     * failures += numc_lin_alg_tests_run();
     */

    return failures;
}
