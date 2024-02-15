

#include "psu-util/progress.h"
#include <unistd.h>
#include <check.h>

using namespace psudb;

START_TEST(t_change_width) 
{
    ck_assert(change_progress_width(15));
    ck_assert(!change_progress_width(80));
}
END_TEST

START_TEST(t_progress)
{
    ck_assert(change_progress_width(50));

    for (size_t i=0; i<101; i++) {
        progress_update(static_cast<double>(i) / 100.0, "Testing");
        usleep(5000);
    }
}
END_TEST

Suite *unit_testing()
{
    Suite *unit = suite_create("Progress Bar Unit Testing");
    TCase *bar = tcase_create("Progress Bar Testing");
    tcase_add_test(bar, t_progress);
    tcase_add_test(bar, t_change_width);
    suite_add_tcase(unit, bar);

    return unit;
}


int run_unit_tests()
{
    int failed = 0;
    Suite *unit = unit_testing();
    SRunner *unit_runner = srunner_create(unit);

    srunner_run_all(unit_runner, CK_NORMAL);
    failed = srunner_ntests_failed(unit_runner);
    srunner_free(unit_runner);

    return failed;
}


int main() 
{
    int unit_failed = run_unit_tests();

    return (unit_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

