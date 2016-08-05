#include <stdlib.h>
#include <check.h>
#include "tests.h"

int main(void)
{
    int nfail;
    Suite *s = create_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    nfail = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (nfail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
