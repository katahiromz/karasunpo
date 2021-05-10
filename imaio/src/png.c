/*****************************************************************************/
/* png.c --- imaio for PNG                                                   */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#include "imaio_impl.h"

#ifdef HAVE_PNG

/*****************************************************************************/

IMAIO_API HBITMAP IIAPI
ii_png_load_common(FILE *inf, float *dpi)
{
    HBITMAP       hbm;
    png_structp     png;
    png_infop       info;
    png_uint_32     y, width, height, rowbytes;
    int             color_type, depth, widthbytes;
    double          gamma;
    BITMAPINFO      bi;
    uint8_t            *pbBits;
    png_uint_32     res_x, res_y;
    int             unit_type;
    png_bytepp      rows;
    HDC             hdc;

    assert(inf);
    if (inf == NULL)
        return NULL;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(png);
    if (png == NULL || info == NULL || setjmp(png_jmpbuf(png)))
    {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(inf);
        return NULL;
    }

    png_init_io(png, inf);
    png_read_info(png, info);

    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);
    png_set_strip_16(png);
    png_set_gray_to_rgb(png);
    png_set_palette_to_rgb(png);
    png_set_bgr(png);
    png_set_packing(png);
    png_set_interlace_handling(png);
    if (png_get_gAMA(png, info, &gamma))
        png_set_gamma(png, 2.2, gamma);
    else
        png_set_gamma(png, 2.2, 0.45455);

    png_read_update_info(png, info);
    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);

    if (dpi)
    {
        *dpi = 0.0;
        if (png_get_pHYs(png, info, &res_x, &res_y, &unit_type))
        {
            if (unit_type == PNG_RESOLUTION_METER)
                *dpi = (float)(res_x * 2.54 / 100.0);
        }
    }

    rowbytes = (png_uint_32)png_get_rowbytes(png, info);
    rows = (png_bytepp)malloc(height * sizeof(png_bytep));
    for (y = 0; y < height; y++)
    {
        rows[y] = (png_bytep)malloc(rowbytes);
    }

    png_read_image(png, rows);
    png_read_end(png, NULL);
    fclose(inf);

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = width;
    bi.bmiHeader.biHeight      = height;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = depth * png_get_channels(png, info);

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void **)&pbBits,
                           NULL, 0);
    DeleteDC(hdc);
    if (hbm == NULL)
    {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    widthbytes = II_WIDTHBYTES(width * bi.bmiHeader.biBitCount);
    for (y = 0; y < height; y++)
    {
        CopyMemory(pbBits + y * widthbytes,
                   rows[height - 1 - y], rowbytes);
    }

    png_destroy_read_struct(&png, &info, NULL);
    free(rows);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_png_load_a(const char *pszFileName, float *dpi)
{
    FILE            *inf;
    inf = fopen(pszFileName, "rb");
    if (inf)
        return ii_png_load_common(inf, dpi);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_png_load_w(const wchar_t *pszFileName, float *dpi)
{
    FILE            *inf;
    inf = _wfopen(pszFileName, L"rb");
    if (inf)
        return ii_png_load_common(inf, dpi);
    return NULL;
}

static void IICAPI
ii_png_mem_read(png_structp png, png_bytep data, png_size_t length)
{
    II_MEMORY *memory;
    assert(png);
    memory = (II_MEMORY *)png_get_io_ptr(png);
    assert(memory);
    if (memory->m_i + length <= memory->m_size)
    {
        CopyMemory(data, memory->m_pb + memory->m_i, length);
        memory->m_i += (uint32_t)length;
    }
}

IMAIO_API HBITMAP IIAPI
ii_png_load_mem(const void *pv, uint32_t cb)
{
    HBITMAP       hbm;
    png_structp     png;
    png_infop       info;
    png_uint_32     y, width, height, rowbytes;
    int             color_type, depth, widthbytes;
    double          gamma;
    BITMAPINFO      bi;
    LPBYTE          pbBits;
    II_MEMORY       memory;
    png_bytepp      rows;
    HDC             hdc;

    memory.m_pb = (const uint8_t *)pv;
    memory.m_i = 0;
    memory.m_size = cb;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(png);
    if (png == NULL || info == NULL || setjmp(png_jmpbuf(png)))
    {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    png_set_read_fn(png, &memory, ii_png_mem_read);
    png_read_info(png, info);

    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);
    png_set_expand(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    png_set_strip_16(png);
    png_set_gray_to_rgb(png);
    png_set_palette_to_rgb(png);
    png_set_bgr(png);
    png_set_packing(png);
    png_set_interlace_handling(png);
    if (png_get_gAMA(png, info, &gamma))
        png_set_gamma(png, 2.2, gamma);
    else
        png_set_gamma(png, 2.2, 0.45455);

    png_read_update_info(png, info);
    png_get_IHDR(png, info, &width, &height, &depth, &color_type,
                 NULL, NULL, NULL);

    rowbytes = (png_uint_32)png_get_rowbytes(png, info);
    rows = (png_bytepp)malloc(height * sizeof(png_bytep));
    for (y = 0; y < height; y++)
    {
        rows[y] = (png_bytep)malloc(rowbytes);
    }

    png_read_image(png, rows);
    png_read_end(png, NULL);

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = width;
    bi.bmiHeader.biHeight      = height;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = (WORD)(depth * png_get_channels(png, info));

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (VOID **)&pbBits,
                           NULL, 0);
    DeleteDC(hdc);
    if (hbm == NULL)
    {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    widthbytes = II_WIDTHBYTES(width * bi.bmiHeader.biBitCount);
    for (y = 0; y < height; y++)
    {
        CopyMemory(pbBits + y * widthbytes,
                   rows[height - 1 - y], rowbytes);
    }

    png_destroy_read_struct(&png, &info, NULL);
    free(rows);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_png_load_res_a(HMODULE hInstance, const char *pszResName)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    HBITMAP hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceA(hInstance, pszResName, "PNG");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_png_load_mem(lpData, dwSize);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_png_load_res_w(HMODULE hInstance, const wchar_t *pszResName)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    HBITMAP hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceW(hInstance, pszResName, L"PNG");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_png_load_mem(lpData, dwSize);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API bool IIAPI
ii_png_save_common(FILE *outf, HBITMAP hbm, float dpi)
{
    png_structp png;
    png_infop info;
    png_color_8 sBIT;
    HDC hMemDC;
    BITMAPINFO bi;
    BITMAP bm;
    uint32_t rowbytes, cbBits;
    LPBYTE pbBits;
    int y, nDepth;
    png_bytep *lines = NULL;
    bool ok = false;

    assert(outf);
    if (outf == NULL)
        return false;

    if (!ii_get_info(hbm, &bm))
    {
        fclose(outf);
        return false;
    }

    nDepth = (bm.bmBitsPixel == 32 ? 32 : 24);
    rowbytes = II_WIDTHBYTES(bm.bmWidth * nDepth);
    cbBits = rowbytes * bm.bmHeight;

    do
    {
        pbBits = (LPBYTE)malloc(cbBits);
        if (pbBits == NULL)
            break;

        ok = false;
        hMemDC = CreateCompatibleDC(NULL);
        if (hMemDC != NULL)
        {
            ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
            bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth    = bm.bmWidth;
            bi.bmiHeader.biHeight   = bm.bmHeight;
            bi.bmiHeader.biPlanes   = 1;
            bi.bmiHeader.biBitCount = (WORD)nDepth;
            ok = GetDIBits(hMemDC, hbm, 0, bm.bmHeight, pbBits, &bi,
                           DIB_RGB_COLORS);
            DeleteDC(hMemDC);
        }
        if (!ok)
            break;
        ok = false;

        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
            break;

        if (setjmp(png_jmpbuf(png)))
            break;

        png_init_io(png, outf);
        png_set_IHDR(png, info, bm.bmWidth, bm.bmHeight, 8,
            (nDepth == 32 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB),
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

        sBIT.red = 8;
        sBIT.green = 8;
        sBIT.blue = 8;
        sBIT.alpha = (png_byte)(nDepth == 32 ? 8 : 0);
        png_set_sBIT(png, info, &sBIT);

        if (dpi != 0.0)
        {
            png_uint_32 res = (png_uint_32)(dpi * 100 / 2.54 + 0.5);
            png_set_pHYs(png, info, res, res, PNG_RESOLUTION_METER);
        }

        png_write_info(png, info);
        png_set_bgr(png);

        lines = (png_bytep *)malloc(sizeof(png_bytep *) * bm.bmHeight);
        if (lines == NULL)
            break;
        for (y = 0; y < bm.bmHeight; y++)
        {
            lines[y] = (png_bytep)&pbBits[rowbytes * (bm.bmHeight - y - 1)];
        }

        png_write_image(png, lines);
        png_write_end(png, info);
        ok = true;
    } while (0);

    png_destroy_write_struct(&png, &info);

    free(lines);
    free(pbBits);
    fclose(outf);

    return ok;
}

IMAIO_API bool IIAPI
ii_png_save_a(const char *pszFileName, HBITMAP hbm, float dpi)
{
    FILE *outf;
    outf = fopen(pszFileName, "wb");
    if (outf)
    {
        if (ii_png_save_common(outf, hbm, dpi))
            return true;
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_png_save_w(const wchar_t *pszFileName, HBITMAP hbm, float dpi)
{
    FILE *outf;
    outf = _wfopen(pszFileName, L"wb");
    if (outf)
    {
        if (ii_png_save_common(outf, hbm, dpi))
            return true;
        DeleteFileW(pszFileName);
    }
    return false;
}

#ifdef PNG_APNG_SUPPORTED
    IMAIO_API HBITMAP IIAPI
    ii_image_from_32bpp_rows(int width, int height, png_bytepp rows)
    {
        HBITMAP hbm;
        LPDWORD pdw, row;
        int x, y;

        assert(width > 0);
        assert(height > 0);
        assert(rows);

        hbm = ii_create_32bpp(width, height);
        if (hbm)
        {
            pdw = (LPDWORD)ii_get_pixels(hbm);
            for (y = 0; y < height; ++y)
            {
                row = (LPDWORD)rows[height - y - 1 + 0];
                for (x = 0; x < width; ++x)
                {
                    pdw[y * width + x] = row[x];
                }
            }
        }
        return hbm;
    }

    IMAIO_API void IIAPI
    ii_32bpp_rows_from_image(png_bytepp rows, HBITMAP hbmImage)
    {
        BITMAP bm;
        int x, y;
        LPDWORD pdw, row;

        if (ii_get_info(hbmImage, &bm))
        {
            pdw = (LPDWORD)bm.bmBits;
            for (y = 0; y < bm.bmHeight; ++y)
            {
                row = (LPDWORD)rows[bm.bmHeight - y - 1];
                for (x = 0; x < bm.bmWidth; ++x)
                {
                    row[x] = pdw[y * bm.bmWidth + x];
                }
            }
        }
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_from_anigif(II_ANIGIF *anigif)
    {
        II_APNG *apng;
        uint32_t i;

        apng = (II_APNG *)calloc(sizeof(II_APNG), 1);
        if (apng == NULL)
            return NULL;

        if (anigif->flags & II_FLAG_USE_SCREEN)
            apng->flags = II_FLAG_USE_SCREEN;
        else
            apng->flags = 0;

        apng->width = anigif->width;
        apng->height = anigif->height;
        apng->num_frames = anigif->num_frames;
        apng->num_plays = anigif->loop_count;
        apng->frames = calloc(sizeof(II_APNG_FRAME), apng->num_frames);
        if (apng->frames)
        {
            for (i = 0; i < apng->num_frames; ++i)
            {
                II_ANIGIF_FRAME *anigif_frame = &anigif->frames[i];
                II_APNG_FRAME *apng_frame = &apng->frames[i];
                apng_frame->x_offset = anigif_frame->x;
                apng_frame->y_offset = anigif_frame->y;
                apng_frame->width = anigif_frame->width;
                apng_frame->height = anigif_frame->height;
                apng_frame->delay = anigif_frame->delay;
                switch (anigif_frame->disposal)
                {
                case 0:
                case 1:
                    apng_frame->dispose_op = PNG_DISPOSE_OP_NONE;
                    break;
                case 2:
                    apng_frame->dispose_op = PNG_DISPOSE_OP_BACKGROUND;
                    break;
                case 3:
                    apng_frame->dispose_op = PNG_DISPOSE_OP_PREVIOUS;
                    break;
                }
                apng_frame->blend_op = PNG_BLEND_OP_OVER;
                if ((apng->flags & II_FLAG_USE_SCREEN) && anigif_frame->hbmScreen)
                {
                    apng_frame->x_offset = 0;
                    apng_frame->y_offset = 0;
                    apng_frame->width = anigif->width;
                    apng_frame->height = anigif->height;
                    apng_frame->hbmScreen = ii_clone(anigif_frame->hbmScreen);
                }
                if (anigif_frame->hbmPart)
                {
                    apng_frame->hbmPart =
                        ii_32bpp_from_trans_8bpp(
                            anigif_frame->hbmPart, &anigif_frame->iTransparent);
                }
            }
        }
        return apng;
    }

#ifdef HAVE_GIF
    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_from_apng(II_APNG *apng, bool kill_semitrans)
    {
        II_ANIGIF *anigif;
        uint32_t i;
        int iTransparent;

        iTransparent = -1;

        anigif = (II_ANIGIF *)calloc(sizeof(II_ANIGIF), 1);
        if (anigif == NULL)
            return NULL;

        if (apng->flags & II_FLAG_USE_SCREEN)
            anigif->flags = II_FLAG_USE_SCREEN;
        else
            anigif->flags = 0;
        anigif->width = apng->width;
        anigif->height = apng->height;
        anigif->num_frames = apng->num_frames;
        anigif->loop_count = apng->num_plays;
        anigif->frames = (II_ANIGIF_FRAME *)
                calloc(sizeof(II_ANIGIF_FRAME), anigif->num_frames);
        if (anigif->frames)
        {
            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *apng_frame = &apng->frames[i];
                II_ANIGIF_FRAME *anigif_frame = &anigif->frames[i];

                anigif_frame->iTransparent = -1;
                anigif_frame->delay = apng_frame->delay;

                anigif_frame->x = apng_frame->x_offset;
                anigif_frame->y = apng_frame->y_offset;
                anigif_frame->width = apng_frame->width;
                anigif_frame->height = apng_frame->height;
                anigif_frame->hbmScreen = NULL;

                if ((apng->flags & II_FLAG_USE_SCREEN) && apng_frame->hbmScreen)
                {
                    anigif_frame->hbmScreen = ii_clone(apng_frame->hbmScreen);
                    anigif_frame->disposal = 0;
                }
                else
                {
                    anigif_frame->hbmScreen = ii_clone(apng_frame->hbmPart);
                }
                if (kill_semitrans)
                {
                    ii_erase_semitrans(anigif_frame->hbmScreen);
                }
            }

            anigif->global_palette = ii_palette_for_anigif(anigif, 255);
            iTransparent = anigif->global_palette->num_colors;
            anigif->global_palette->num_colors++;

            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *apng_frame = &apng->frames[i];
                II_ANIGIF_FRAME *anigif_frame = &anigif->frames[i];
                if ((apng->flags & II_FLAG_USE_SCREEN) == 0 ||
                    apng_frame->hbmScreen == NULL)
                {
                    ii_destroy(anigif_frame->hbmScreen);
                    anigif_frame->hbmScreen = NULL;
                }
                if (apng_frame->hbmPart)
                {
                    anigif_frame->hbmPart =
                        ii_reduce_colors(
                            apng_frame->hbmPart,
                            anigif->global_palette, &iTransparent);
                }
                anigif_frame->iTransparent = iTransparent;
            }
        }
        else
        {
            ii_anigif_destroy(anigif);
            anigif = NULL;
        }

        return anigif;
    }
#endif  /* def HAVE_GIF */

    IMAIO_API void IIAPI
    ii_apng_destroy(II_APNG *apng)
    {
        uint32_t i;

        if (apng)
        {
            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *frame = &apng->frames[i];
                if (frame->hbmScreen)
                {
                    ii_destroy(frame->hbmScreen);
                    frame->hbmScreen = NULL;
                }
                if (frame->hbmPart)
                {
                    ii_destroy(frame->hbmPart);
                    frame->hbmPart = NULL;
                }
            }
            if (apng->hbmDefault)
            {
                ii_destroy(apng->hbmDefault);
                apng->hbmDefault = NULL;
            }
            free(apng->frames);
            free(apng);
        }
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_fp(FILE *fp, II_FLAGS flags)
    {
        png_byte sig[8];
        II_APNG *apng = NULL;
        png_structp png;
        png_infop info;
        png_bytepp rows = NULL;
        png_uint_32 i, k, rowbytes, y;
        bool ok = false;
        HBITMAP hbm, hbmScreen = NULL;
        II_APNG_FRAME *frame, *old_frame;
        double gamma;

        if (fp == NULL)
            return NULL;

        /* check signature */
        if (!fread(sig, 8, 1, fp) || !png_check_sig(sig, 8))
        {
            fclose(fp);
            return NULL;
        }

        png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
        {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(fp);
            return NULL;
        }

        png_init_io(png, fp);
        png_set_sig_bytes(png, 8);

        png_set_strip_16(png);
        png_set_gray_to_rgb(png);
        png_set_palette_to_rgb(png);
        png_set_bgr(png);
        png_set_packing(png);
        png_set_interlace_handling(png);
        png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
        if (png_get_gAMA(png, info, &gamma))
            png_set_gamma(png, 2.2, gamma);
        else
            png_set_gamma(png, 2.2, 0.45455);

        do
        {
            if (setjmp(png_jmpbuf(png)))
                break;

            png_read_info(png, info);

            /* is it an APNG? */
            if (!png_get_valid(png, info, PNG_INFO_acTL))
            {
                break;
            }

            apng = (II_APNG *)calloc(1, sizeof(II_APNG));
            if (apng == NULL)
                break;

            /* store info */
            apng->width = png_get_image_width(png, info);
            apng->height = png_get_image_height(png, info);
            apng->num_plays = png_get_num_plays(png, info);
            if (flags & II_FLAG_USE_SCREEN)
                apng->flags = II_FLAG_USE_SCREEN;
            else
                apng->flags = 0;

            /* get resolution */
            {
                png_uint_32 res_x, res_y;
                int unit_type;

                apng->dpi = 0.0;
                if (png_get_pHYs(png, info, &res_x, &res_y, &unit_type))
                {
                    if (unit_type == PNG_RESOLUTION_METER)
                        apng->dpi = (float)(res_x * 2.54 / 100.0);
                }
            }

            /* create screen */
            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
            if (hbmScreen == NULL)
                break;

            /* allocate rows */
            rows = (png_bytepp)malloc(sizeof(png_bytep) * apng->height);
            if (rows == NULL)
                break;
            rowbytes = (png_uint_32)png_get_rowbytes(png, info);
            for (y = 0; y < apng->height; ++y)
            {
                rows[y] = (png_bytep)malloc(rowbytes);
                if (rows[y] == NULL)
                {
                    for (--y; y < apng->height; --y)
                        free(rows[y]);
                    free(rows);
                    rows = NULL;
                    break;
                }
            }
            if (rows == NULL)
                break;

            /* allocate for frames */
            apng->num_frames = png_get_num_frames(png, info);
            apng->frames = (II_APNG_FRAME *)
                calloc(apng->num_frames, sizeof(II_APNG_FRAME));
            if (apng->frames == NULL)
                break;

            /* for each frame */
            k = 0;
            old_frame = NULL;
            for (i = 0; i < apng->num_frames; ++i)
            {
                frame = &apng->frames[k];

                png_read_frame_head(png, info);

                if (png_get_valid(png, info, PNG_INFO_fcTL))
                {
                    /* frame */
                    uint16_t delay_num, delay_den;

                    png_get_next_frame_fcTL(png, info,
                                            (png_uint_32 *)&frame->width,
                                            (png_uint_32 *)&frame->height,
                                            (png_uint_32 *)&frame->x_offset,
                                            (png_uint_32 *)&frame->y_offset,
                                            &delay_num, &delay_den,
                                            &frame->dispose_op, &frame->blend_op);

                    /* calculate delay time */
                    if (delay_den == 0)
                        delay_den = 100;
                    frame->delay = (delay_num * 1000) / delay_den;

                    /* create a part */
                    png_read_image(png, rows);
                    hbm = ii_image_from_32bpp_rows(frame->width, frame->height, rows);
                    frame->hbmPart = hbm;

                    /* blending */
                    switch (frame->blend_op)
                    {
                    case PNG_BLEND_OP_SOURCE:
                        {
                            HDC hdc1, hdc2;
                            HGDIOBJ hbm1Old, hbm2Old;
                            hdc1 = CreateCompatibleDC(NULL);
                            hdc2 = CreateCompatibleDC(NULL);
                            hbm1Old = SelectObject(hdc1, hbmScreen);
                            hbm2Old = SelectObject(hdc2, hbm);
                            BitBlt(hdc1,
                                frame->x_offset, frame->y_offset,
                                frame->width, frame->height,
                                hdc2, 0, 0, SRCCOPY);
                            SelectObject(hdc1, hbm1Old);
                            SelectObject(hdc1, hbm2Old);
                            DeleteDC(hdc1);
                            DeleteDC(hdc2);
                        }
                        break;
                    case PNG_BLEND_OP_OVER:
                        ii_stamp(hbmScreen, frame->x_offset, frame->y_offset,
                                 hbm, NULL, 255);
                        break;
                    }

                    /* create a screen image */
                    if (flags & II_FLAG_USE_SCREEN)
                    {
                        frame->hbmScreen = ii_clone(hbmScreen);

                        /* dispose */
                        switch (frame->dispose_op)
                        {
                        case PNG_DISPOSE_OP_BACKGROUND:
                            ii_destroy(hbmScreen);
                            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
                            break;
                        case PNG_DISPOSE_OP_PREVIOUS:
                            if (old_frame)
                            {
                                ii_destroy(hbmScreen);
                                hbmScreen = ii_clone(old_frame->hbmScreen);
                            }
                            break;
                        }
                    }

                    old_frame = frame;
                    ++k;
                }
                else
                {
                    /* default image */
                    png_read_image(png, rows);
                    apng->flags |= II_FLAG_DEFAULT_PRESENT;
                    hbm = ii_image_from_32bpp_rows(apng->width, apng->height, rows);
                    apng->hbmDefault = hbm;
                }
            }
            ok = true;
            png_read_end(png, info);
        } while (0);

        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);

        if (apng && rows)
        {
            for (y = 0; y < apng->height; ++y)
            {
                free(rows[y]);
            }
            free(rows);
        }

        if (apng)
        {
            if (apng->flags & II_FLAG_DEFAULT_PRESENT)
                apng->num_frames -= 1;

            if (ok && apng->num_frames > 0)
            {
                if (apng->hbmDefault == NULL)
                {
                    apng->hbmDefault = ii_clone(apng->frames[0].hbmScreen);
                }
            }
            else
            {
                ii_apng_destroy(apng);
                apng = NULL;
            }
        }

        if (hbmScreen)
        {
            ii_destroy(hbmScreen);
        }

        return apng;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_mem(const void *pv, uint32_t cb, II_FLAGS flags)
    {
        II_APNG *apng = NULL;
        png_structp png;
        png_infop info;
        png_bytepp rows = NULL;
        png_uint_32 i, k, rowbytes, y;
        bool ok = false;
        HBITMAP hbm, hbmScreen = NULL;
        II_APNG_FRAME *frame, *old_frame;
        double gamma;
        II_MEMORY       memory;

        memory.m_pb = (const uint8_t *)pv;
        memory.m_i = 0;
        memory.m_size = cb;

        /* check signature */
        if (cb < 8 || !png_check_sig(memory.m_pb, 8))
        {
            return NULL;
        }
        memory.m_i = 8;

        png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
        {
            png_destroy_read_struct(&png, &info, NULL);
            return NULL;
        }

        png_set_read_fn(png, &memory, ii_png_mem_read);
        png_set_sig_bytes(png, 8);

        png_set_strip_16(png);
        png_set_gray_to_rgb(png);
        png_set_palette_to_rgb(png);
        png_set_bgr(png);
        png_set_packing(png);
        png_set_interlace_handling(png);
        png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
        if (png_get_gAMA(png, info, &gamma))
            png_set_gamma(png, 2.2, gamma);
        else
            png_set_gamma(png, 2.2, 0.45455);

        do
        {
            if (setjmp(png_jmpbuf(png)))
                break;

            png_read_info(png, info);

            /* is it an APNG? */
            if (!png_get_valid(png, info, PNG_INFO_acTL))
                break;

            apng = (II_APNG *)calloc(1, sizeof(II_APNG));
            if (apng == NULL)
                break;

            /* store info */
            apng->width = png_get_image_width(png, info);
            apng->height = png_get_image_height(png, info);
            apng->num_plays = png_get_num_plays(png, info);
            if (flags & II_FLAG_USE_SCREEN)
                apng->flags = II_FLAG_USE_SCREEN;
            else
                apng->flags = 0;

            /* get resolution */
            {
                png_uint_32 res_x, res_y;
                int unit_type;

                apng->dpi = 0.0;
                if (png_get_pHYs(png, info, &res_x, &res_y, &unit_type))
                {
                    if (unit_type == PNG_RESOLUTION_METER)
                        apng->dpi = (float)(res_x * 2.54 / 100.0);
                }
            }

            /* create screen */
            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
            if (hbmScreen == NULL)
                break;

            /* allocate rows */
            rows = (png_bytepp)malloc(sizeof(png_bytep) * apng->height);
            if (rows == NULL)
                break;
			rowbytes = (png_uint_32)png_get_rowbytes(png, info);
            for (y = 0; y < apng->height; ++y)
            {
                rows[y] = (png_bytep)malloc(rowbytes);
                if (rows[y] == NULL)
                {
                    for (--y; y < apng->height; --y)
                        free(rows[y]);
                    free(rows);
                    rows = NULL;
                    break;
                }
            }
            if (rows == NULL)
                break;

            /* allocate for frames */
            apng->num_frames = png_get_num_frames(png, info);
            apng->frames = (II_APNG_FRAME *)
                calloc(apng->num_frames, sizeof(II_APNG_FRAME));
            if (apng->frames == NULL)
                break;

            /* for each frame */
            k = 0;
            old_frame = NULL;
            for (i = 0; i < apng->num_frames; ++i)
            {
                frame = &apng->frames[k];

                png_read_frame_head(png, info);

                if (png_get_valid(png, info, PNG_INFO_fcTL))
                {
                    /* frame */
                    uint16_t delay_num, delay_den;

                    png_get_next_frame_fcTL(png, info,
                                            (png_uint_32 *)&frame->width,
                                            (png_uint_32 *)&frame->height,
                                            (png_uint_32 *)&frame->x_offset,
                                            (png_uint_32 *)&frame->y_offset,
                                            &delay_num, &delay_den,
                                            &frame->dispose_op, &frame->blend_op);

                    /* calculate delay time */
                    if (delay_den == 0)
                        delay_den = 100;
                    frame->delay = (delay_num * 1000) / delay_den;

                    /* create a part */
                    png_read_image(png, rows);
                    hbm = ii_image_from_32bpp_rows(frame->width, frame->height, rows);
                    frame->hbmPart = hbm;

                    /* blending */
                    switch (frame->blend_op)
                    {
                    case PNG_BLEND_OP_SOURCE:
                        {
                            HDC hdc1, hdc2;
                            HGDIOBJ hbm1Old, hbm2Old;
                            hdc1 = CreateCompatibleDC(NULL);
                            hdc2 = CreateCompatibleDC(NULL);
                            hbm1Old = SelectObject(hdc1, hbmScreen);
                            hbm2Old = SelectObject(hdc2, hbm);
                            BitBlt(hdc1,
                                frame->x_offset, frame->y_offset,
                                frame->width, frame->height,
                                hdc2, 0, 0, SRCCOPY);
                            SelectObject(hdc1, hbm1Old);
                            SelectObject(hdc1, hbm2Old);
                            DeleteDC(hdc1);
                            DeleteDC(hdc2);
                        }
                        break;
                    case PNG_BLEND_OP_OVER:
                        ii_stamp(hbmScreen, frame->x_offset, frame->y_offset,
                                 hbm, NULL, 255);
                        break;
                    }

                    /* create a screen image */
                    if (flags & II_FLAG_USE_SCREEN)
                    {
                        frame->hbmScreen = ii_clone(hbmScreen);

                        /* dispose */
                        switch (frame->dispose_op)
                        {
                        case PNG_DISPOSE_OP_BACKGROUND:
                            ii_destroy(hbmScreen);
                            hbmScreen = ii_create_32bpp_trans(apng->width, apng->height);
                            break;
                        case PNG_DISPOSE_OP_PREVIOUS:
                            if (old_frame)
                            {
                                ii_destroy(hbmScreen);
                                hbmScreen = ii_clone(old_frame->hbmScreen);
                            }
                            break;
                        }
                    }

                    old_frame = frame;
                    ++k;
                }
                else
                {
                    /* default image */
                    png_read_image(png, rows);
                    apng->flags |= II_FLAG_DEFAULT_PRESENT;
                    hbm = ii_image_from_32bpp_rows(apng->width, apng->height, rows);
                    apng->hbmDefault = hbm;
                }
            }
            ok = true;
            png_read_end(png, info);
        } while (0);

        png_destroy_read_struct(&png, &info, NULL);

        if (apng && rows)
        {
            for (y = 0; y < apng->height; ++y)
            {
                free(rows[y]);
            }
            free(rows);
        }

        if (apng)
        {
            if (apng->flags & II_FLAG_DEFAULT_PRESENT)
                apng->num_frames -= 1;

            if (ok && apng->num_frames > 0)
            {
                if (apng->hbmDefault == NULL)
                {
                    apng->hbmDefault = ii_clone(apng->frames[0].hbmScreen);
                }
            }
            else
            {
                ii_apng_destroy(apng);
                apng = NULL;
            }
        }

        if (hbmScreen)
        {
            ii_destroy(hbmScreen);
        }

        return apng;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_a(const char *pszFileName, II_FLAGS flags)
    {
        FILE *fp;
        fp = fopen(pszFileName, "rb");
        if (fp)
            return ii_apng_load_fp(fp, flags);
        return NULL;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_w(const wchar_t *pszFileName, II_FLAGS flags)
    {
        FILE *fp;
        fp = _wfopen(pszFileName, L"rb");
        if (fp)
            return ii_apng_load_fp(fp, flags);
        return NULL;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_res_a(HMODULE hInstance, const char *pszResName, II_FLAGS flags)
    {
        II_APNG *apng;
        HGLOBAL hGlobal;
        uint32_t dwSize;
        LPVOID lpData;
        HRSRC hRsrc;

        assert(pszResName);
        hRsrc = FindResourceA(hInstance, pszResName, "PNG");
        if (hRsrc == NULL)
        {
            hRsrc = FindResourceA(hInstance, pszResName, "APNG");
            if (hRsrc == NULL)
            {
                return NULL;
            }
        }

        dwSize = SizeofResource(hInstance, hRsrc);
        hGlobal = LoadResource(hInstance, hRsrc);
        if (hGlobal == NULL)
            return NULL;

        lpData = LockResource(hGlobal);
        apng = ii_apng_load_mem(lpData, dwSize, flags);

    #ifdef WIN16
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    #endif

        return apng;
    }

    IMAIO_API II_APNG * IIAPI
    ii_apng_load_res_w(HMODULE hInstance, const wchar_t *pszResName, II_FLAGS flags)
    {
        II_APNG *apng;
        HGLOBAL hGlobal;
        uint32_t dwSize;
        LPVOID lpData;
        HRSRC hRsrc;

        assert(pszResName);
        hRsrc = FindResourceW(hInstance, pszResName, L"PNG");
        if (hRsrc == NULL)
        {
            hRsrc = FindResourceW(hInstance, pszResName, L"APNG");
            if (hRsrc == NULL)
            {
                return NULL;
            }
        }

        dwSize = SizeofResource(hInstance, hRsrc);
        hGlobal = LoadResource(hInstance, hRsrc);
        if (hGlobal == NULL)
            return NULL;

        lpData = LockResource(hGlobal);
        apng = ii_apng_load_mem(lpData, dwSize, flags);

    #ifdef WIN16
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    #endif

        return apng;
    }

    IMAIO_API bool IIAPI
    ii_apng_save_fp(FILE *fp, II_APNG *apng)
    {
        png_structp png;
        png_infop info;
        png_bytepp rows = NULL;
        uint32_t y, i, k;
        png_color_8 sBIT;
        bool ok = false;
        png_uint_32 rowbytes;

        if (apng == NULL)
        {
            fclose(fp);
            return false;
        }

        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        info = png_create_info_struct(png);
        if (png == NULL || info == NULL)
        {
            png_destroy_write_struct(&png, &info);
            fclose(fp);
            return false;
        }

        png_init_io(png, fp);

        do
        {
            if (setjmp(png_jmpbuf(png)))
            {
                break;
            }

            png_set_IHDR(png, info, apng->width, apng->height,
                         8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

            sBIT.red = 8;
            sBIT.green = 8;
            sBIT.blue = 8;
            sBIT.alpha = 8;
            png_set_sBIT(png, info, &sBIT);

            if (apng->dpi != 0.0)
            {
                png_uint_32 res = (png_uint_32)(apng->dpi * 100 / 2.54 + 0.5);
                png_set_pHYs(png, info, res, res, PNG_RESOLUTION_METER);
            }

            png_set_bgr(png);
            if ((apng->flags & II_FLAG_DEFAULT_PRESENT) && apng->hbmDefault)
                png_set_acTL(png, info, apng->num_frames + 1, apng->num_plays);
            else
                png_set_acTL(png, info, apng->num_frames, apng->num_plays);

            /* allocate rows */
            rows = (png_bytepp)calloc(sizeof(png_bytep), apng->height);
            if (rows == NULL)
                break;
			rowbytes = (png_uint_32)png_get_rowbytes(png, info);
            for (y = 0; y < apng->height; ++y)
            {
                rows[y] = (png_bytep)malloc(rowbytes);
                if (rows[y] == NULL)
                {
                    for (k = y - 1; k < apng->height; --k)
                        free(rows[y]);
                    free(rows);
                    rows = NULL;
                    break;
                }
            }
            if (rows == NULL)
                break;

            if ((apng->flags & II_FLAG_DEFAULT_PRESENT) && apng->hbmDefault)
            {
                /* write the default image */
                png_set_first_frame_is_hidden(png, info, 1);

                png_write_info(png, info);

                ii_32bpp_rows_from_image(rows, apng->hbmDefault);
                png_write_frame_head(
                    png, info, rows, 
                    apng->width, apng->height, 0, 0,
                    0, 0,
                    PNG_DISPOSE_OP_NONE,
                    PNG_BLEND_OP_OVER
                );

                png_write_image(png, rows);
                png_write_frame_tail(png, info);
            }

            for (i = 0; i < apng->num_frames; ++i)
            {
                II_APNG_FRAME *frame = &apng->frames[i];

                png_write_info(png, info);
                if (apng->flags & II_FLAG_USE_SCREEN)
                {
                    ii_32bpp_rows_from_image(rows, frame->hbmScreen);
                    png_write_frame_head(
                        png, info, rows, 
                        apng->width, apng->height, 0, 0,
                        frame->delay, 1000, 
                        frame->dispose_op,
                        frame->blend_op
                    );
                }
                else
                {
                    ii_32bpp_rows_from_image(rows, frame->hbmPart);
                    png_write_frame_head(
                        png, info, rows, 
                        frame->width, frame->height,
                        frame->x_offset, frame->y_offset,
                        frame->delay, 1000, 
                        frame->dispose_op,
                        frame->blend_op
                    );
                }

                png_write_image(png, rows);
                png_write_frame_tail(png, info);
            }
            ok = true;
            png_write_end(png, NULL);
        } while (0);

        if (rows)
        {
            for (y = 0; y < apng->height; ++y)
            {
                free(rows[y]);
            }
            free(rows);
        }

        png_destroy_write_struct(&png, &info);
        fclose(fp);

        return ok;
    }

    IMAIO_API bool IIAPI
    ii_apng_save_a(const char *pszFileName, II_APNG *apng)
    {
        FILE *fp;
        fp = fopen(pszFileName, "wb");
        if (fp)
        {
            if (ii_apng_save_fp(fp, apng))
                return true;
        }
        DeleteFileA(pszFileName);
        return false;
    }

    IMAIO_API bool IIAPI
    ii_apng_save_w(const wchar_t *pszFileName, II_APNG *apng)
    {
        FILE *fp;
        fp = _wfopen(pszFileName, L"wb");
        if (fp)
        {
            if (ii_apng_save_fp(fp, apng))
                return true;
        }
        DeleteFileW(pszFileName);
        return false;
    }
#endif  /* def PNG_APNG_SUPPORTED */

/*****************************************************************************/

#endif  /* def HAVE_PNG */
