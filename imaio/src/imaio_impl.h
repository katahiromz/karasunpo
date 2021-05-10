#ifndef IMAIO_IMPL_H_
#define IMAIO_IMPL_H_

#define IMAIO_BUILDING 1

#include "imaio.h"

#include <tchar.h>
#include <string.h>
#include <mbstring.h>
#include <assert.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <setjmp.h>

#define II_WIDTHBYTES(i) (((i) + 31) / 32 * 4)

typedef struct tagII_BITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} II_BITMAPINFOEX, FAR * LPII_BITMAPINFOEX;

#ifndef LR_LOADREALSIZE
    #define LR_LOADREALSIZE 128
#endif

typedef struct II_MEMORY
{
    const uint8_t *     m_pb;           /* pointer to memory */
    uint32_t            m_i;            /* reading position */
    uint32_t            m_size;         /* memory size */
    void *              p_user;         /* user data pointer */
    size_t              i_user;         /* user data integer */
} II_MEMORY;

#endif  /* ndef IMAIO_IMPL_H_ */
