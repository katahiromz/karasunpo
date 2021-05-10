/*****************************************************************************/
/* tif.c --- imaio for TIFF                                                  */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#include "imaio_impl.h"

#ifdef HAVE_TIFF

/*****************************************************************************/

IMAIO_API HBITMAP IIAPI
ii_tif_load_common(TIFF *tif, float *dpi)
{
    BITMAPINFO bi;
    HBITMAP hbm, hbm32Bpp, hbm24Bpp;
    uint32_t *pdwBits, *pdw;
    void *pvBits;
    HDC hdc1, hdc2;
    uint16 resunit;
    uint32 w, h;
    size_t cPixels;
    uint8 r, g, b, a;
    bool fOpaque;

    assert(tif);
    if (tif == NULL)
        return NULL;

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 32;

    hdc1 = CreateCompatibleDC(NULL);
    hdc2 = CreateCompatibleDC(NULL);
    if (hdc1 == NULL || hdc2 == NULL)
    {
        DeleteDC(hdc1);
        DeleteDC(hdc2);
        TIFFClose(tif);
        return NULL;
    }

    hbm = NULL;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &resunit);

    if (dpi)
    {
        if (!TIFFGetField(tif, TIFFTAG_XRESOLUTION, dpi))
        {
            *dpi = 0.0f;
        }
        else if (resunit == RESUNIT_CENTIMETER)
        {
            *dpi *= 2.54f;
        }
        else
        {
            *dpi = 0.0f;
        }
    }
    bi.bmiHeader.biWidth    = w;
    bi.bmiHeader.biHeight   = h;
    cPixels = w * h;
    hbm32Bpp = CreateDIBSection(hdc1, &bi, DIB_RGB_COLORS,
        (void **)&pdwBits, NULL, 0);
    if (hbm32Bpp)
    {
        if (TIFFReadRGBAImageOriented(tif, w, h, (uint32 *)pdwBits,
            ORIENTATION_BOTLEFT, 0))
        {
            pdw = pdwBits;
            fOpaque = true;
            while (cPixels--)
            {
                r = (uint8)TIFFGetR(*pdw);
                g = (uint8)TIFFGetG(*pdw);
                b = (uint8)TIFFGetB(*pdw);
                a = (uint8)TIFFGetA(*pdw);
                if (a != 255)
                {
                    fOpaque = false;
                }
                *pdw++ = (uint32)b | (((uint32)g) << 8) |
                    (((uint32)r) << 16) | (((uint32)a) << 24);
            }
            if (fOpaque)
            {
                bi.bmiHeader.biBitCount = 24;
                hbm24Bpp = CreateDIBSection(hdc2, &bi, DIB_RGB_COLORS,
                    &pvBits, NULL, 0);
                if (hbm24Bpp)
                {
                    HGDIOBJ hbmOld1 = SelectObject(hdc1, hbm32Bpp);
                    HGDIOBJ hbmOld2 = SelectObject(hdc2, hbm24Bpp);
                    BitBlt(hdc2, 0, 0, w, h, hdc1, 0, 0, SRCCOPY);
                    SelectObject(hdc1, hbmOld1);
                    SelectObject(hdc2, hbmOld2);
                    hbm = hbm24Bpp;
                    DeleteObject(hbm32Bpp);
                }
            }
            else
            {
                hbm = hbm32Bpp;
            }
        }
    }

    TIFFClose(tif);
    DeleteDC(hdc1);
    DeleteDC(hdc2);

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_tif_load_a(const char *pszFileName, float *dpi)
{
    TIFF* tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpen(pszFileName, "r");
    if (tif)
        return ii_tif_load_common(tif, dpi);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_tif_load_w(const wchar_t *pszFileName, float *dpi)
{
    TIFF* tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpenW(pszFileName, "r");
    if (tif)
        return ii_tif_load_common(tif, dpi);
    return NULL;
}

IMAIO_API bool IIAPI
ii_tif_save_common(TIFF *tif, HBITMAP hbm, float dpi)
{
    BITMAP bm;
    bool no_alpha;
    BITMAPINFO bi;
    int32_t widthbytes;
    uint8_t *pbBits, *pb;
    int c, y;
    bool f;
    HDC hdc;

    assert(tif);
    if (tif == NULL)
        return false;

    if (!ii_get_info(hbm, &bm))
    {
        TIFFClose(tif);
        return false;
    }

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = bm.bmWidth;
    bi.bmiHeader.biHeight   = bm.bmHeight;
    bi.bmiHeader.biPlanes   = 1;

    no_alpha = (bm.bmBitsPixel <= 24 || ii_is_opaque(hbm));
    bi.bmiHeader.biBitCount = (WORD)(no_alpha ? 24 : 32);
    widthbytes = II_WIDTHBYTES(bm.bmWidth * bi.bmiHeader.biBitCount);
    pbBits = (uint8_t *)malloc(widthbytes * bm.bmHeight);
    if (pbBits == NULL)
    {
        TIFFClose(tif);
        return false;
    }

    hdc = CreateCompatibleDC(NULL);
    if (!GetDIBits(hdc, hbm, 0, bm.bmHeight, pbBits, &bi, DIB_RGB_COLORS))
    {
        DeleteDC(hdc);
        free(pbBits);
        TIFFClose(tif);
        return false;
    }
    DeleteDC(hdc);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, bm.bmWidth);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, bm.bmHeight);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, no_alpha ? 3 : 4);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    if (dpi != 0.0)
    {
        TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
        TIFFSetField(tif, TIFFTAG_XRESOLUTION, dpi);
        TIFFSetField(tif, TIFFTAG_YRESOLUTION, dpi);
    }
    TIFFSetField(tif, TIFFTAG_SOFTWARE, "katayama_hirofumi_mz's software");
    f = true;
    for (y = 0; y < bm.bmHeight; y++)
    {
        pb = &pbBits[(bm.bmHeight - 1 - y) * widthbytes];
        c = bm.bmWidth;
        if (no_alpha)
        {
            uint8_t b;
            while (c--)
            {
                b = pb[2];
                pb[2] = *pb;
                *pb = b;
                pb++; pb++; pb++;
            }
        }
        else
        {
            uint8_t b;
            while (c--)
            {
                b = pb[2];
                pb[2] = *pb;
                *pb = b;
                pb++; pb++; pb++; pb++;
            }
        }
        if (TIFFWriteScanline(tif,
            &pbBits[(bm.bmHeight - 1 - y) * widthbytes], y, 0) < 0)
        {
            f = false;
            break;
        }
    }
    TIFFClose(tif);

    free(pbBits);
    return f;
}

IMAIO_API bool IIAPI
ii_tif_save_a(const char *pszFileName, HBITMAP hbm, float dpi)
{
    TIFF *tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpen(pszFileName, "w");
    if (tif)
    {
        if (ii_tif_save_common(tif, hbm, dpi))
            return true;
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_tif_save_w(const wchar_t *pszFileName, HBITMAP hbm, float dpi)
{
    TIFF *tif;
    TIFFSetWarningHandler(NULL);
    TIFFSetWarningHandlerExt(NULL);
    tif = TIFFOpenW(pszFileName, "w");
    if (tif)
    {
        if (ii_tif_save_common(tif, hbm, dpi))
            return true;
        DeleteFileW(pszFileName);
    }
    return false;
}

#endif  /* def HAVE_TIFF */
