

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

START_TEST(t_change_width_boundaries)
{
    /* Width of 1 is the minimum valid value */
    ck_assert(change_progress_width(1));

    /* Width of 70 is the maximum (g_prog_bar.length()) */
    ck_assert(change_progress_width(70));

    /* 0 is out of range */
    ck_assert(!change_progress_width(0));

    /* 71 is out of range */
    ck_assert(!change_progress_width(71));

    /* Very large value */
    ck_assert(!change_progress_width(10000));
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

START_TEST(t_progress_to_file)
{
    /* Redirect output to a temporary file and verify content. */
    FILE *tmp = tmpfile();
    ck_assert_ptr_nonnull(tmp);

    ck_assert(change_progress_width(20));
    progress_update(0.5, "Half", tmp);
    fflush(tmp);

    /* Read back and verify the output contains expected fragments. */
    rewind(tmp);
    char buf[512] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    ck_assert_int_gt(n, 0);

    /* Should contain the percentage and the prompt */
    ck_assert_ptr_nonnull(strstr(buf, "50%"));
    ck_assert_ptr_nonnull(strstr(buf, "Half"));

    fclose(tmp);
}
END_TEST

START_TEST(t_progress_at_zero)
{
    FILE *tmp = tmpfile();
    ck_assert_ptr_nonnull(tmp);

    ck_assert(change_progress_width(20));
    progress_update(0.0, "Start", tmp);
    fflush(tmp);

    rewind(tmp);
    char buf[512] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    ck_assert_int_gt(n, 0);
    ck_assert_ptr_nonnull(strstr(buf, "0%"));
    ck_assert_ptr_nonnull(strstr(buf, "Start"));

    fclose(tmp);
}
END_TEST

START_TEST(t_progress_at_completion)
{
    /* At 1.0, progress_update should emit a trailing newline. */
    FILE *tmp = tmpfile();
    ck_assert_ptr_nonnull(tmp);

    ck_assert(change_progress_width(20));
    progress_update(1.0, "Done", tmp);
    fflush(tmp);

    rewind(tmp);
    char buf[512] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    ck_assert_int_gt(n, 0);
    ck_assert_ptr_nonnull(strstr(buf, "100%"));
    ck_assert_ptr_nonnull(strstr(buf, "Done"));

    /* Verify trailing newline */
    ck_assert_int_eq(buf[n - 1], '\n');

    fclose(tmp);
}
END_TEST

START_TEST(t_progress_no_newline_before_complete)
{
    /* Before 100%, output should NOT end with a newline. */
    FILE *tmp = tmpfile();
    ck_assert_ptr_nonnull(tmp);

    ck_assert(change_progress_width(20));
    progress_update(0.5, "Mid", tmp);
    fflush(tmp);

    rewind(tmp);
    char buf[512] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    ck_assert_int_gt(n, 0);
    ck_assert_int_ne(buf[n - 1], '\n');

    fclose(tmp);
}
END_TEST

START_TEST(t_progress_empty_prompt)
{
    FILE *tmp = tmpfile();
    ck_assert_ptr_nonnull(tmp);

    ck_assert(change_progress_width(20));
    progress_update(0.25, "", tmp);
    fflush(tmp);

    rewind(tmp);
    char buf[512] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    ck_assert_int_gt(n, 0);
    ck_assert_ptr_nonnull(strstr(buf, "25%"));

    fclose(tmp);
}
END_TEST

Suite *unit_testing()
{
    Suite *unit = suite_create("Progress Bar Unit Testing");
    TCase *bar = tcase_create("Progress Bar Testing");
    tcase_add_test(bar, t_progress);
    tcase_add_test(bar, t_change_width);
    tcase_add_test(bar, t_change_width_boundaries);
    tcase_add_test(bar, t_progress_to_file);
    tcase_add_test(bar, t_progress_at_zero);
    tcase_add_test(bar, t_progress_at_completion);
    tcase_add_test(bar, t_progress_no_newline_before_complete);
    tcase_add_test(bar, t_progress_empty_prompt);
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

