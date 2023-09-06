#include <memory>

#include "psu-util/alignment.h"
#include "psu-io/PagedFile.h"

namespace psudb {
std::unique_ptr<PagedFileIterator> create_pagedfile_itr(PagedFile *, PageNum, PageNum);

class PagedFileIterator {
    friend std::unique_ptr<PagedFileIterator> create_pagedfile_itr(PagedFile *, PageNum, PageNum);

public:
    bool next() {
        while (m_current_pnum < m_stop_pnum) {
            if (m_pfile->read_page(++m_current_pnum, m_buffer)) {
                return true;
            }
            
            /* IO error of some kind */
            return false;
        }

        /* no more pages to read */
        return false;
    }

    byte *get_item() {
        return m_buffer;
    }

    ~PagedFileIterator() {
        free(m_buffer);
    }

private:
    PagedFile *m_pfile;
    PageNum m_current_pnum;
    PageNum m_start_pnum;
    PageNum m_stop_pnum;

    byte *m_buffer;

    PagedFileIterator(PagedFile *pfile, PageNum start_page, PageNum stop_page)
      : m_pfile(pfile)
      , m_current_pnum((start_page == INVALID_PNUM) ? 0 : start_page - 1)
      , m_start_pnum(start_page)
      , m_stop_pnum(stop_page)
      , m_buffer((byte *)aligned_alloc(SECTOR_SIZE, PAGE_SIZE)) {}
};


std::unique_ptr<PagedFileIterator> create_pagedfile_itr(PagedFile *pfile, PageNum start_page=0, PageNum stop_page=0) {
    auto itr = new PagedFileIterator(pfile, start_page, stop_page);
    return std::unique_ptr<PagedFileIterator>(itr);
}
}
