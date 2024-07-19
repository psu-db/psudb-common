#include <string>

#include "psu-io/PagedFile.h"
#include "psu-io/PagedFileIterator.h"

#include <check.h>

using namespace psudb;

std::string existing_file1 = "bin/tests/data/test_file1.dat";
std::string nonexisting_file = "bin/tests/data/nonexisting_file.dat";
std::string new_file = "bin/tests/data/new_file.dat";

bool initialize_test_file(std::string fname, size_t page_cnt)
{
    auto flags = O_RDWR | O_CREAT | O_TRUNC;
    mode_t mode = 0640;
    char *page = nullptr;

    int fd = open(fname.c_str(), flags, mode);
    if (fd == -1) {
        goto error;
    }

    page = (char *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE);
    if (!page) {
        goto error_opened;
    }

    for (size_t i=0; i<=page_cnt; i++) {
        *((int *) page) = i;
        if (write(fd, page, PAGE_SIZE) == -1) {
            goto error_alloced;
        }
    }

    free(page);

    return 1;

error_alloced:
    free(page);

error_opened:
    close(fd);

error:
    return 0;
}


START_TEST(t_create)
{
    auto pfile = PagedFile::create(new_file, true);

    ck_assert_ptr_nonnull(pfile.get());
    ck_assert_int_eq(pfile->get_page_count(), 0);
    ck_assert_int_eq(pfile->get_file_size(), PAGE_SIZE);

    std::string fname = pfile->get_fname();
    ck_assert_str_eq(fname.c_str(), new_file.c_str());
}
END_TEST


START_TEST(t_create_fail)
{
    auto pfile = PagedFile::create(nonexisting_file, false);

    ck_assert_ptr_null(pfile.get());
}
END_TEST


START_TEST(t_create_open)
{
    size_t pg_cnt = 10;
    ck_assert(initialize_test_file(existing_file1, pg_cnt));

    auto pfile = PagedFile::create(existing_file1, false);
    ck_assert_ptr_nonnull(pfile.get());
    ck_assert_int_eq(pfile->get_page_count(), pg_cnt);
    ck_assert_int_eq(pfile->get_file_size(), PAGE_SIZE * (pg_cnt + 1));

    std::string fname = pfile->get_fname();
    ck_assert_str_eq(fname.c_str(), existing_file1.c_str());
}
END_TEST


START_TEST(t_read_page)
{
    size_t pg_cnt = 10;
    ck_assert(initialize_test_file(existing_file1, pg_cnt));
    auto pfile = PagedFile::create(existing_file1, false);
    ck_assert_ptr_nonnull(pfile.get());

    byte *buffer = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE);

    for (size_t i=1; i<=pg_cnt; i++) {
        ck_assert_int_eq(pfile->read_page(i, buffer), 1);
        ck_assert_int_eq(*((int *) buffer), i);
    }

    ck_assert_int_eq(pfile->read_page(0, buffer), 0);
    ck_assert_int_eq(pfile->read_page(11, buffer), 0);
    
    free(buffer);
}
END_TEST


START_TEST(t_read_pages_sg)
{
    size_t pg_cnt = 20;
    ck_assert(initialize_test_file(existing_file1, pg_cnt));
    auto pfile = PagedFile::create(existing_file1, false);
    ck_assert_ptr_nonnull(pfile.get());

    size_t buf_cnt = 10;
    std::vector<byte *> buffers(buf_cnt);
    std::vector<std::pair<PageNum, byte*>> reads(buf_cnt);
    std::vector<PageNum> to_read = {1, 2, 3, 5, 9, 7, 8, 11, 12, 15};
    for (size_t i=0; i<buf_cnt; i++) {
        buffers[i] = (byte *) std::aligned_alloc(SECTOR_SIZE, PAGE_SIZE); 
        reads[i] = {to_read[i], buffers[i]};
    }

    ck_assert_int_eq(pfile->read_pages(reads), 1);

    for (size_t i=0; i<buf_cnt; i++) {
        ck_assert_int_eq(*((int*) buffers[i]), to_read[i]);
    }

    for (size_t i=0; i<buf_cnt; i++) {
        free(buffers[i]);
    }
}
END_TEST


START_TEST(t_read_pages_seq)
{
    size_t pg_cnt = 20;
    ck_assert(initialize_test_file(existing_file1, pg_cnt));
    auto pfile = PagedFile::create(existing_file1, false);
    ck_assert_ptr_nonnull(pfile.get());

    PageNum read_cnt = 10;
    PageNum start_pg = 5;
    byte *buffer = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE*25);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_int_eq(pfile->read_pages(start_pg, read_cnt, buffer), 1);
    for (size_t i=0; i<read_cnt; i++) {
        ck_assert_int_eq(i + start_pg, *((int*) (buffer + PAGE_SIZE * i)));    
    }

    read_cnt = 25;
    ck_assert_int_eq(pfile->read_pages(start_pg, start_pg + read_cnt, buffer), 0);

    free(buffer);
}
END_TEST

START_TEST(t_allocate_pages)
{
    auto pfile = PagedFile::create(new_file, true);
    ck_assert_ptr_nonnull(pfile.get());

    ck_assert_int_eq(pfile->get_page_count(), 0);

    ck_assert_int_eq(pfile->allocate_pages(1), 1);
    ck_assert_int_eq(pfile->get_page_count(), 1);
    ck_assert_int_eq(pfile->get_file_size(), 2*PAGE_SIZE);

    ck_assert_int_eq(pfile->allocate_pages(10), 2);
    ck_assert_int_eq(pfile->get_page_count(), 11);
    ck_assert_int_eq(pfile->get_file_size(), 12*PAGE_SIZE);
}
END_TEST

START_TEST(t_write)
{
    auto pfile = PagedFile::create(new_file, true);
    ck_assert_ptr_nonnull(pfile.get());

    byte *buffer = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE*2);
    *((int*) buffer) = 123;
 
    // writing to an unallocated page fails
    ck_assert_int_eq(pfile->write_page(1, buffer), 0);

    pfile->allocate_pages(1);

    ck_assert_int_eq(pfile->write_page(1, buffer), 1);
    ck_assert_int_eq(pfile->write_page(2, buffer), 0);
    
    auto pfile2 = PagedFile::create(new_file, false);
    ck_assert_int_eq(pfile2->get_page_count(), 1);

    byte *buffer2 = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE);
    ck_assert_int_eq(pfile2->read_page(1, buffer2), 1);
    ck_assert_int_eq(123, *((int*) buffer2));

    free(buffer);
    free(buffer2);
}
END_TEST


START_TEST(t_write_pages)
{
    auto pfile = PagedFile::create(new_file, true);
    ck_assert_ptr_nonnull(pfile.get());

    PageNum page_cnt = 13;
    byte *buffer = (byte *) aligned_alloc(SECTOR_SIZE, page_cnt*PAGE_SIZE);

    for (size_t i=0; i<page_cnt; i++) {
        *((int*) (buffer + PAGE_SIZE * i)) = i;
    }
    
    ck_assert_int_eq(pfile->write_pages(1, 13, buffer), 0);

    pfile->allocate_pages(20);
    
    PageNum start_pg = 3;
    ck_assert_int_eq(pfile->write_pages(start_pg, page_cnt, buffer), 1);

    free(buffer);

    auto pfile2 = PagedFile::create(new_file, false);
    buffer = (byte *) aligned_alloc(SECTOR_SIZE, page_cnt*PAGE_SIZE);

    ck_assert_int_eq(pfile2->read_pages(start_pg, page_cnt, buffer), 1);
    for (size_t i=0; i<page_cnt; i++) {
        ck_assert_int_eq(*((int*) (buffer + PAGE_SIZE * i)), i);
    }

    free(buffer);
}
END_TEST


START_TEST(t_remove)
{
    auto pfile = PagedFile::create(new_file, true);
    ck_assert_ptr_nonnull(pfile.get());
    pfile.reset();

    pfile = PagedFile::create(new_file, false);
    ck_assert_ptr_nonnull(pfile.get());

    ck_assert_int_eq(pfile->remove_file(), 1);
    pfile.reset();

    pfile = PagedFile::create(new_file, false);
    ck_assert_ptr_null(pfile.get());
}
END_TEST


START_TEST(t_iterator)
{
    size_t pg_cnt = 20;
    ck_assert(initialize_test_file(existing_file1, pg_cnt));
    auto pfile = PagedFile::create(existing_file1, false);
    ck_assert_ptr_nonnull(pfile.get());

    auto iter = pfile->start_scan();
    ck_assert_ptr_nonnull(iter.get());
    size_t i=0;
    while (iter->next()) {
        i++;
        ck_assert_int_eq(i, *((int*) iter->get_item()));
    }

    ck_assert_int_eq(i, pg_cnt);
}
END_TEST


START_TEST(t_iterator_page_range)
{
    size_t pg_cnt = 20;
    ck_assert(initialize_test_file(existing_file1, pg_cnt));
    auto pfile = PagedFile::create(existing_file1, false);
    ck_assert_ptr_nonnull(pfile.get());

    auto iter = pfile->start_scan(30, 45);
    ck_assert_ptr_null(iter.get());

    iter = pfile->start_scan(5, 13);

    size_t i=4;
    while (iter->next()) {
        i++;
        ck_assert_int_eq(i, *((int*) iter->get_item()));
    }

    ck_assert_int_eq(i, 13);
}
END_TEST

START_TEST(t_meta_page) 
{
    auto pfile = PagedFile::create(new_file, true);

    byte *write_buffer = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE);
    byte *read_buffer = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE);
    *((int*) write_buffer) = 123;

    /* writing to the meta page should succeed */
    ck_assert_int_eq(pfile->write_metapage(write_buffer), 1);

    /* reading from the meta page should also succeed */
    ck_assert_int_eq(pfile->read_metapage(read_buffer), 1);

    /* the contents of the metapage should match the data written earlier */
    ck_assert_int_eq(*((int*) read_buffer), 123);
}

START_TEST(t_meta_noclobber) 
{
    auto pfile = PagedFile::create(new_file, true);
    PageNum page_cnt = 13;
    byte *buffer = (byte *) aligned_alloc(SECTOR_SIZE, page_cnt*PAGE_SIZE);

    for (size_t i=0; i<page_cnt; i++) {
        *((int*) (buffer + PAGE_SIZE * i)) = i;
    }

    pfile->allocate_pages(page_cnt);

    ck_assert_int_eq(pfile->write_pages(1, page_cnt, buffer), 1);

    byte *buffer2 = (byte *) aligned_alloc(SECTOR_SIZE, PAGE_SIZE);
    *((int*) buffer2) = 123;
    ck_assert_int_eq(pfile->write_metapage(buffer2), 1);

    free(buffer);

    auto pfile2 = PagedFile::create(new_file, false);
    buffer = (byte *) aligned_alloc(SECTOR_SIZE, page_cnt*PAGE_SIZE);

    ck_assert_int_eq(pfile2->read_pages(1, page_cnt, buffer), 1);
    for (size_t i=0; i<page_cnt; i++) {
        ck_assert_int_eq(*((int*) (buffer + PAGE_SIZE * i)), i);
    }
    
    ck_assert_int_eq(pfile2->read_metapage(buffer), 1);

    ck_assert_int_eq(*((int*) buffer), 123);

    free(buffer);
}

Suite *unit_testing()
{
    Suite *unit = suite_create("PagedFile Unit Testing");
    TCase *initialize = tcase_create("PagedFile::create Testing");
    tcase_add_test(initialize, t_create);
    tcase_add_test(initialize, t_create_fail);
    tcase_add_test(initialize, t_create_open);
    suite_add_tcase(unit, initialize);

    TCase *read = tcase_create("PagedFile::read_page(s) Testing");
    tcase_add_test(read, t_read_page);
    tcase_add_test(read, t_read_pages_sg);
    tcase_add_test(read, t_read_pages_seq);
    suite_add_tcase(unit, read);

    TCase *allocate = tcase_create("PagedFile::allocate_pages Testing");
    tcase_add_test(allocate, t_allocate_pages);
    suite_add_tcase(unit, allocate);

    TCase *write = tcase_create("PagedFile::write_page(s) Testing");
    tcase_add_test(write, t_write);
    tcase_add_test(write, t_write_pages);
    suite_add_tcase(unit, write);

    TCase *remove = tcase_create("PagedFile::remove_file Testing");
    tcase_add_test(remove, t_remove);
    suite_add_tcase(unit, remove);

    TCase *iter = tcase_create("PagedFile::start_scan Testing");
    tcase_add_test(iter, t_iterator);
    tcase_add_test(iter, t_iterator_page_range);
    suite_add_tcase(unit, iter);

    TCase *meta = tcase_create("PagedFile::metapage Testing");
    tcase_add_test(meta, t_meta_page);
    tcase_add_test(meta, t_meta_noclobber);
    suite_add_tcase(unit, meta);

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

