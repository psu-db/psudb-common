
#include "psu-util/alignment.h"

#ifdef PF_COUNT_IO
    #define INC_READ() m_read_cnt++
    #define INC_WRITE() m_write_cnt++
    #define RESET_IO_CNT() \
        m_read_cnt = 0; \
        m_write_cnt = 0
#else
    #define INC_READ() do {} while (0)
    #define INC_WRITE() do {} while (0)
    #define RESET_IO_CNT() do {} while (0)
#endif


