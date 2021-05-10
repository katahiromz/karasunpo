/*****************************************************************************/
/* jpg.c --- imaio for JPEG                                                  */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#include "imaio_impl.h"

#ifdef HAVE_JPEG

/*****************************************************************************/

IMAIO_API HBITMAP IIAPI
ii_jpg_load_common(FILE *fp, float *dpi)
{
    struct jpeg_decompress_struct decomp;
    struct jpeg_error_mgr jerror;
    BITMAPINFO bi;
    uint8_t *lpBuf, *pb;
    HBITMAP hbm;
    JSAMPARRAY buffer;
    int row;
    HDC hdc;

    assert(fp);
    if (fp == NULL)
        return NULL;

    decomp.err = jpeg_std_error(&jerror);

    jpeg_create_decompress(&decomp);
    jpeg_stdio_src(&decomp, fp);

    jpeg_read_header(&decomp, true);
    jpeg_start_decompress(&decomp);

    if (dpi)
    {
        switch(decomp.density_unit)
        {
        case 1: /* dots/inch */
            *dpi = decomp.X_density;
            break;

        case 2: /* dots/cm */
            *dpi = (float)(decomp.X_density * 2.54);
            break;

        default:
            *dpi = 0.0;
        }
    }

    row = ((decomp.output_width * 3 + 3) & ~3);
    buffer = (*decomp.mem->alloc_sarray)((j_common_ptr)&decomp, JPOOL_IMAGE,
                                         row, 1);

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth        = decomp.output_width;
    bi.bmiHeader.biHeight       = decomp.output_height;
    bi.bmiHeader.biPlanes       = 1;
    bi.bmiHeader.biBitCount     = 24;
    bi.bmiHeader.biCompression  = BI_RGB;
    bi.bmiHeader.biSizeImage    = row * decomp.output_height;

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&lpBuf, NULL, 0);
    DeleteDC(hdc);
    if (hbm == NULL)
    {
        jpeg_destroy_decompress(&decomp);
        fclose(fp);
        return NULL;
    }

    pb = lpBuf + row * decomp.output_height;
    while (decomp.output_scanline < decomp.output_height)
    {
        pb -= row;
        jpeg_read_scanlines(&decomp, buffer, 1);

        if (decomp.out_color_components == 1)
        {
            UINT i;
            uint8_t *p = (uint8_t *)buffer[0];
            for (i = 0; i < decomp.output_width; i++)
            {
                pb[3 * i + 0] = p[i];
                pb[3 * i + 1] = p[i];
                pb[3 * i + 2] = p[i];
            }
        }
        else if (decomp.out_color_components == 3)
        {
            int i;
            for (i = 0; i < row; i += 3)
            {
                pb[i + 0] = buffer[0][i + 2];
                pb[i + 1] = buffer[0][i + 1];
                pb[i + 2] = buffer[0][i + 0];
            }
        }
        else
        {
            jpeg_destroy_decompress(&decomp);
            fclose(fp);
            DeleteObject(hbm);
            return NULL;
        }
    }

    SetDIBits(NULL, hbm, 0, decomp.output_height, lpBuf, &bi, DIB_RGB_COLORS);

    jpeg_finish_decompress(&decomp);
    jpeg_destroy_decompress(&decomp);

    fclose(fp);

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_jpg_load_a(const char *pszFileName, float *dpi)
{
    FILE *fp;
    fp = fopen(pszFileName, "rb");
    if (fp)
        return ii_jpg_load_common(fp, dpi);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_jpg_load_w(const wchar_t *pszFileName, float *dpi)
{
    FILE *fp;
    fp = _wfopen(pszFileName, L"rb");
    if (fp)
        return ii_jpg_load_common(fp, dpi);
    return NULL;
}

IMAIO_API bool IIAPI
ii_jpg_save_common(FILE *fp, HBITMAP hbm,
                   int quality, bool progression, float dpi)
{
    BITMAP bm;
    struct jpeg_compress_struct comp;
    struct jpeg_error_mgr jerr;
    JSAMPLE * image_buffer;
    BITMAPINFO bi;
    HDC hDC, hMemDC;
    uint8_t *pbBits;
    int nWidthBytes;
    uint32_t cbBits;
    bool f;

    if (fp == NULL)
        return false;

    if (!ii_get_info(hbm, &bm))
    {
        fclose(fp);
        return false;
    }

    comp.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&comp);
    jpeg_stdio_dest(&comp, fp);

    comp.image_width  = bm.bmWidth;
    comp.image_height = bm.bmHeight;
    comp.input_components = 3;
    comp.in_color_space = JCS_RGB;
    jpeg_set_defaults(&comp);
    if (dpi != 0.0)
    {
        comp.density_unit = 1; /* dots/inch */
        comp.X_density = (UINT16)(dpi + 0.5);
        comp.Y_density = (UINT16)(dpi + 0.5);
    }
    jpeg_set_quality(&comp, quality, true);
    if (progression)
        jpeg_simple_progression(&comp);

    jpeg_start_compress(&comp, true);
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = bm.bmWidth;
    bi.bmiHeader.biHeight   = bm.bmHeight;
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 24;

    f = false;
    nWidthBytes = II_WIDTHBYTES(bm.bmWidth * 24);
    cbBits = nWidthBytes * bm.bmHeight;
    pbBits = (uint8_t *)malloc(cbBits);
    if (pbBits != NULL)
    {
        image_buffer = (JSAMPLE *)malloc(nWidthBytes);
        if (image_buffer != NULL)
        {
            hDC = GetDC(NULL);
            if (hDC != NULL)
            {
                hMemDC = CreateCompatibleDC(hDC);
                if (hMemDC != NULL)
                {
                    f = GetDIBits(hMemDC, hbm, 0, bm.bmHeight, pbBits,
                                  (BITMAPINFO*)&bi, DIB_RGB_COLORS);
                    DeleteDC(hMemDC);
                }
                ReleaseDC(NULL, hDC);
            }
            if (f)
            {
                int x, y;
                uint8_t *src, *dest;
                for (y = 0; y < bm.bmHeight; y++)
                {
                    dest = image_buffer;
                    src = &pbBits[(bm.bmHeight - y - 1) * nWidthBytes];
                    for (x = 0; x < bm.bmWidth; x++)
                    {
                        dest[0] = src[2];
                        dest[1] = src[1];
                        dest[2] = src[0];
                        dest += 3;
                        src += 3;
                    }
                    jpeg_write_scanlines(&comp, &image_buffer, 1);
                }
            }
            free(image_buffer);
        }
        free(pbBits);
    }

    jpeg_finish_compress(&comp);
    jpeg_destroy_compress(&comp);

    fclose(fp);
    return f;
}

IMAIO_API bool IIAPI
ii_jpg_save_a(const char *pszFileName, HBITMAP hbm,
              int quality, bool progression, float dpi)
{
    FILE *fp;
    fp = fopen(pszFileName, "wb");
    if (fp)
    {
        if (ii_jpg_save_common(fp, hbm, quality, progression, dpi))
            return true;
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_jpg_save_w(const wchar_t *pszFileName, HBITMAP hbm,
              int quality, bool progression, float dpi)
{
    FILE *fp;
    fp = _wfopen(pszFileName, L"wb");
    if (fp)
    {
        if (ii_jpg_save_common(fp, hbm, quality, progression, dpi))
            return true;
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/

#endif  /* def HAVE_JPEG */
