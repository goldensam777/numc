/*
 * test_status.c
 *
 * Tests du module de gestion d'erreur de numc.
 */


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "numc.h"
#include "test_status.h"


static void
test_last_status_defaults_to_ok(void** state)
{
    (void) state;
    assert_int_equal(numc_last_status(), NUMC_OK);
}


static void
test_set_error_records_status(void** state)
{
    (void) state;
    numc_set_error(NUMC_ERR_INVALID_ARG, "dimension %d invalide", -3);
    assert_int_equal(numc_last_status(), NUMC_ERR_INVALID_ARG);
    assert_string_equal(numc_error(), "dimension -3 invalide");
}


static void
test_ok_resets_last_error(void** state)
{
    (void) state;
    numc_set_error(NUMC_ERR_DOMAIN, "sqrt(-1)");
    numc_set_error(NUMC_OK, NULL);
    assert_int_equal(numc_last_status(), NUMC_OK);
    assert_null(numc_error());
}


static void
test_status_string_known_codes(void** state)
{
    (void) state;
    assert_string_equal(numc_status_string(NUMC_OK), "success");
    assert_string_equal(numc_status_string(NUMC_ERR_ALLOC), "allocation failure");
    assert_string_equal(numc_status_string(NUMC_ERR_NOT_IMPLEMENTED),
                        "not implemented");
}


static void
test_status_string_unknown_code(void** state)
{
    (void) state;
    assert_string_equal(numc_status_string((numc_status_t) 999),
                        "unknown status");
}


const struct CMUnitTest numc_status_tests[] = {
    cmocka_unit_test(test_last_status_defaults_to_ok),
    cmocka_unit_test(test_set_error_records_status),
    cmocka_unit_test(test_ok_resets_last_error),
    cmocka_unit_test(test_status_string_known_codes),
    cmocka_unit_test(test_status_string_unknown_code),
};
