/*
 * include/psu-io/PagedFile.h
 *
 * Copyright (C) 2023 Douglas B. Rumbaugh <drumbaugh@psu.edu> 
 *
 * All rights reserved. Published under the Modified BSD License.
 *
 */
#pragma once
#define __GNU_SOURCE

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <memory>

#include "psu-util/alignment.h"
#include "psu-io/IOTypes.h"
#include "psu-io/IOCounter.h"

namespace psudb {

class PagedFile;
class PagedFileIterator;
extern std::unique_ptr<PagedFileIterator> create_pagedfile_itr(PagedFile *, PageNum, PageNum);


class PagedFile {
public:
    static std::shared_ptr<PagedFile> create_shared(const std::string fname, bool new_file=true, bool o_direct=true) {
        return std::shared_ptr<PagedFile>(PagedFile::setup(fname, new_file, o_direct));
    }

    static std::unique_ptr<PagedFile> create(const std::string fname, bool new_file=true, bool o_direct=true) {
        return std::unique_ptr<PagedFile>(PagedFile::setup(fname, new_file, o_direct));
    }

    /*
     * Add new_page_count new pages to the file in bulk, and returns the
     * PageId of the first page in the new range. 
     *
     * If the allocation fails, returns INVALID_PID. Also returns INVALID_PID
     * if bulk allocation is not supported by the implementation. This can be
     * checked via the supports_allocation method.
     */
    PageNum allocate_pages(PageNum count=1) {
        PageNum new_first = get_page_count() + 1;
        size_t alloc_size = count * PAGE_SIZE;

        if (raw_allocate(alloc_size)) {
            return new_first;
        }

        return INVALID_PNUM;
    }

    /*
     * Reads data from the specified page into a buffer pointed to by
     * buffer_ptr. It is necessary for buffer_ptr to be parm::SECTOR_SIZE
     * aligned, and also for it to be large enough to accommodate
     * parm::PAGE_SIZE bytes. If the read succeeds, returns 1. Otherwise
     * returns 0. The contents of the input buffer are undefined in the case of
     * an error.
     */
    int read_page(PageNum pnum, byte *buffer_ptr) {
        if (check_pnum(pnum)) {
            return raw_read(buffer_ptr, PAGE_SIZE, pnum_to_offset(pnum));
        }

        return 0;
    }

    /*
     * Reads several pages into associated buffers. It is necessary for the
     * buffer referred to by each pointer to be parm::SECTOR_SIZE aligned and
     * large enough to accommodate parm::PAGE_SIZE bytes. If possible,
     * vectorized IO may be used to read adjacent pages. If the reads succeed,
     * returns 1. If a read fails, returns 0. The contents of all the buffers
     * are undefined in the case of an error.
     */
    int read_pages(std::vector<std::pair<PageNum, byte*>> pages) {
        if (pages.size() == 0) {
            return 0;
        }

        if (pages.size() == 1) {
            read_page(pages[0].first, pages[0].second);
        }

        std::sort(pages.begin(), pages.end());

        PageNum range_start = pages[0].first;
        PageNum prev_pnum = range_start;

        std::vector<byte *> buffers;
        buffers.push_back(pages[0].second);

        for (size_t i=1; i<pages.size(); i++) {
            if (pages[i].first == prev_pnum + 1) {
                buffers.push_back(pages[i].second);
                prev_pnum = pages[i].first;
            } else {
                if (!raw_readv(buffers, PAGE_SIZE, pnum_to_offset(range_start))) {
                    return 0;
                }

                range_start = pages[i].first;
                prev_pnum = range_start;

                buffers.clear();
                buffers.push_back(pages[i].second);
            }
        }

        return raw_readv(buffers, PAGE_SIZE, pnum_to_offset(range_start));
    }

    /*
     * Reads several pages stored contiguously into a single buffer. It is 
     * necessary that buffer_ptr be SECTOR_SIZE aligned and also
     * at least page_cnt * PAGE_SIZE bytes large.
     */
    int read_pages(PageNum first_page, size_t page_cnt, byte *buffer_ptr) {
        if (check_pnum(first_page) && check_pnum(first_page + page_cnt - 1)) {
            return raw_read(buffer_ptr, page_cnt * PAGE_SIZE, first_page * PAGE_SIZE); 
        }

        return 0;
    }

    /*
     * Writes data from the provided buffer into the specified page within the
     * file. It is necessary for buffer_ptr to be parm::SECTOR_SIZE aligned,
     * and also for it to be at least parm::PAGE_SIZE bytes large. If it is
     * larger, only the first parm::PAGE_SIZE bytes will be written. If it is
     * smaller, the result is undefined.
     *
     * If the write succeeds, returns 1. Otherwise returns 0. The contents of
     * the specified page within the file are undefined in the case of an error.
     */
    int write_page(PageNum pnum, const byte *buffer_ptr) {
        if (check_pnum(pnum)) {
            return raw_write(buffer_ptr, PAGE_SIZE, pnum_to_offset(pnum));
        }

        return 0;
    }

    /*
     * Writes multiple pages stored sequentially in the provided buffer into
     * a contiguous region of the file, starting at first_page. If the write
     * would overrun the allocated space in the file, no data is written.
     *
     * It is necessary for buffer_ptr to be SECTOR_SIZE aligned, and at 
     * least PAGE_SIZE * page_cnt bytes large.
     *
     * Returns the number of complete pages successfully written.
     */
    int write_pages(PageNum first_page, size_t page_cnt, const byte *buffer_ptr) {
        if (check_pnum(first_page) && check_pnum(first_page + page_cnt - 1)) {
           return raw_write(buffer_ptr, page_cnt * PAGE_SIZE, first_page * PAGE_SIZE); 
        }

        return 0;
    }

    /*
     * Returns the number of allocated paged in the file.
     */
    PageNum get_page_count() {
        return m_size / PAGE_SIZE - 1;
    }

    /*
     * Delete this file from the underlying filesystem. Once this has been called,
     * this object will be closed, and all operations other than destructing it are
     * undefined. Returns 1 on successful removal of the file, and 0 on failure.
     */
    int remove_file() {
        if (m_file_open) {
            close(m_fd);
        }

        if (unlink(m_fname.c_str())) {
            return 0;
        }

        m_file_open = false;

        return 1;
    }


    /*
     * Returns the raw number of bytes allocated in the backing file.
     */
    off_t get_file_size() const {
        return m_size;
    }

    std::unique_ptr<PagedFileIterator> start_scan(PageNum start_page=1, PageNum end_page=0) {
        if (end_page == INVALID_PNUM) {
            end_page = get_page_count();
        }

        if (check_pnum(start_page) && check_pnum(end_page)) {
            return create_pagedfile_itr(this, start_page, end_page);
        }

        return nullptr;
    }

    std::string get_fname() {
        return m_fname;
    }

    void rename_file(std::string fname) {
        if(rename(m_fname.c_str(), fname.c_str())) {
            fprintf(stderr, "%s -> %s\n", m_fname.c_str(), fname.c_str());
            perror("IN RENAME:");
            assert(false);
        }

        m_fname = fname;
    }

    ~PagedFile() {
        if (m_file_open) {
            close(m_fd);
        }
    }

private:

    static PagedFile *setup(const std::string fname, bool new_file, bool o_direct) {
        auto flags = O_RDWR;

        if (o_direct) {
            flags |= O_DIRECT;
        }

        if (new_file) {
            flags |= O_CREAT | O_TRUNC;
        }

        mode_t mode = 0640;
        off_t size = 0;

        int fd = open(fname.c_str(), flags, mode);
        if (fd == -1) {
            return nullptr;
        }
        
        if (new_file) {
            if(fallocate(fd, 0, 0, PAGE_SIZE)) {
                return nullptr;
            }

            size = PAGE_SIZE;
        } else {
            struct stat buf;
            if (fstat(fd, &buf) == -1) {
                return nullptr;
            }

            size = buf.st_size;
        } 

        if (fd) {
            return new PagedFile(fd, fname, size, mode);
        }

        return nullptr;
    }

    PagedFile(int fd, std::string fname, off_t size, mode_t mode) {
        m_file_open = true;
        m_fd = fd;
        m_fname = fname;
        m_size = size;
        m_mode = mode;
    }

    bool check_pnum(PageNum pnum) const {
        return pnum != INVALID_PNUM && pnum < (get_file_size() / PAGE_SIZE);
    }

    int raw_read(byte *buffer, off_t amount, off_t offset) {
        if (!verify_io_parms(amount, offset)) {
            return 0;
        }

        if (pread(m_fd, buffer, amount, offset) != amount) {
            return 0;
        }

        INC_READ();

        return 1;
    }

    int raw_readv(std::vector<byte *> buffers, off_t buffer_size, off_t initial_offset) {
        size_t buffer_cnt = buffers.size();

        off_t amount = buffer_size * buffer_cnt;
        if (!verify_io_parms(amount, initial_offset)) {
            return 0;
        }

        auto iov = new iovec[buffer_cnt];
        for (size_t i=0; i<buffer_cnt; i++) {
            iov[i].iov_base = buffers[i];
            iov[i].iov_len = buffer_size;
        }

        if (preadv(m_fd, iov, buffer_cnt, initial_offset) != amount) {
            return 0;
        }

        INC_READ();

        delete[] iov;

        return 1;
    }

    int raw_write(const byte *buffer, off_t amount, off_t offset) {
        if (!verify_io_parms(amount, offset)) {
            return 0;
        }

        if (pwrite(m_fd, buffer, amount, offset) != amount) {
            return 0;
        }

        INC_WRITE();

        return 1;
    }

    int raw_allocate(size_t amount) {
        if (!m_file_open || (amount % SECTOR_SIZE != 0)) {
            return 0;
        }

        int alloc_mode = 0;
        if (fallocate(m_fd, alloc_mode, m_size, amount)) {
            return 0;
        }

        m_size += amount;
        return 1;

    }

    bool verify_io_parms(off_t amount, off_t offset) {
        if (!m_file_open || amount + offset > m_size) {
            return false;
        }

        if (amount % SECTOR_SIZE != 0) {
            return false;
        }

        if (offset % SECTOR_SIZE != 0) {
            return false;
        }

        return true;
    }

    int m_fd;
    bool m_file_open;
    off_t m_size;
    mode_t m_mode;
    std::string m_fname;
    int m_flags;

    size_t m_read_cnt;
    size_t m_write_cnt;
};

}
