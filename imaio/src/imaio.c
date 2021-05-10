/*****************************************************************************/
/* imaio.c --- imaio                                                         */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#include "imaio_impl.h"

/*****************************************************************************/

IMAIO_API HBITMAP IIAPI
ii_create(int width, int height, int bpp, const II_PALETTE *table)
{
    II_BITMAPINFOEX bi;
    HBITMAP hbmNew;
    LPVOID pvBits;
    int i;
    HDC hdc;

    assert(width > 0);
    assert(height > 0);
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = bpp;
    if (table)
    {
        for (i = 0; i < table->num_colors; ++i)
        {
            bi.bmiColors[i].rgbBlue = table->colors[i].value[0];
            bi.bmiColors[i].rgbGreen = table->colors[i].value[1];
            bi.bmiColors[i].rgbRed = table->colors[i].value[2];
            bi.bmiColors[i].rgbReserved = 0;
        }
    }
    hdc = CreateCompatibleDC(NULL);
    hbmNew = CreateDIBSection(hdc, (LPBITMAPINFO)&bi, DIB_RGB_COLORS,
                              &pvBits, NULL, 0);
    DeleteDC(hdc);
    assert(hbmNew);
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_create_8bpp_solid(int cx, int cy, const II_PALETTE *table, int iColorIndex)
{
    HBITMAP hbmNew;
    BITMAP bm;

    assert(table);
    hbmNew = ii_create(cx, cy, 8, table);
    assert(hbmNew);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        FillMemory(bm.bmBits, bm.bmWidthBytes * cy, (uint8_t)iColorIndex);
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_create_8bpp_grayscale(int cx, int cy)
{
    II_PALETTE *table;
    int i;
    HBITMAP hbmNew;

    table = ii_palette_create(256, NULL);
    if (table == NULL)
        return NULL;

    for (i = 0; i < 256; ++i)
    {
        table->colors[i].value[0] = (uint8_t)i;
        table->colors[i].value[1] = (uint8_t)i;
        table->colors[i].value[2] = (uint8_t)i;
    }

    hbmNew = ii_create(cx, cy, 8, table);
    ii_palette_destroy(table);

    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_create_24bpp(int cx, int cy)
{
    HBITMAP hbmNew;
    hbmNew = ii_create(cx, cy, 24, NULL);
    assert(hbmNew);
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_create_24bpp_solid(int cx, int cy, const II_COLOR8 *color)
{
    HBITMAP hbmNew;
    LPBYTE pbBits;
    BITMAP bm;
    int x, y;

    hbmNew = ii_create(cx, cy, 24, NULL);
    assert(hbmNew);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;
        for (y = 0; y < cy; ++y)
        {
            for (x = 0; x < cx; ++x)
            {
                pbBits[x * 3 + y * bm.bmWidthBytes + 0] = color->value[0];
                pbBits[x * 3 + y * bm.bmWidthBytes + 1] = color->value[1];
                pbBits[x * 3 + y * bm.bmWidthBytes + 2] = color->value[2];
            }
        }
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_create_32bpp(int cx, int cy)
{
    HBITMAP hbm;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create(cx, cy, 32, NULL);
    assert(hbm);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_32bpp_trans(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create(cx, cy, 32, NULL);
    assert(hbm);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    ZeroMemory(bm.bmBits, cb);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_32bpp_solid(int cx, int cy, const II_COLOR8 *color)
{
    HBITMAP hbmNew;
    BITMAP bm;
    DWORD dw, cdw;
    LPDWORD pdw;

    hbmNew = ii_create(cx, cy, 32, NULL);
    assert(hbmNew);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pdw = (LPDWORD)bm.bmBits;
        cdw = cx * cy;
        while (cdw--)
        {
            dw = color->value[3];
            dw <<= 8;
            dw |= color->value[2];
            dw <<= 8;
            dw |= color->value[1];
            dw <<= 8;
            dw |= color->value[0];
            *pdw++ = dw;
        }
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_create_24bpp_black(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_24bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    ZeroMemory(bm.bmBits, cb);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_32bpp_black_opaque(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    LPBYTE pb;
    uint32_t cb, i;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_32bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    pb = (LPBYTE)bm.bmBits;
    for (i = 0; i < cb; i += 4)
    {
        pb++; pb++; pb++;
        *pb++ = 0xFF;
    }
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_24bpp_white(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_24bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    FillMemory(bm.bmBits, cb, 0xFF);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_32bpp_white(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    uint32_t cb;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_32bpp(cx, cy);
    if (hbm == NULL)
        return hbm;

    ii_get_info(hbm, &bm);
    cb = bm.bmWidthBytes * bm.bmHeight;
    FillMemory(bm.bmBits, cb, 0xFF);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_24bpp_checker(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    LPBYTE pbBits, pb;
    int x, y;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_24bpp(cx, cy);
    if (hbm == NULL)
        return hbm;
    ii_get_info(hbm, &bm);
    pbBits = (LPBYTE)bm.bmBits;
    for (y = 0; y < cy; y++)
    {
        pb = &pbBits[y * bm.bmWidthBytes];
        for (x = 0; x < cx; x++)
        {
            if (((x >> 3) & 1) ^ (((bm.bmHeight - y - 1) >> 3) & 1))
                pb[x * 3 + 0] = pb[x * 3 + 1] = pb[x * 3 + 2] = 0x88;
            else
                pb[x * 3 + 0] = pb[x * 3 + 1] = pb[x * 3 + 2] = 0xFF;
        }
    }
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_create_32bpp_checker(int cx, int cy)
{
    HBITMAP hbm;
    BITMAP bm;
    LPBYTE pbBits, pb;
    int x, y;

    assert(cx > 0);
    assert(cy > 0);
    hbm = ii_create_32bpp(cx, cy);
    if (hbm == NULL)
        return hbm;
    ii_get_info(hbm, &bm);

    pbBits = (LPBYTE)bm.bmBits;
    for (y = 0; y < cy; y++)
    {
        pb = &pbBits[y * bm.bmWidthBytes];
        for (x = 0; x < cx; x++)
        {
            if (((x >> 3) & 1) ^ (((bm.bmHeight - y - 1) >> 3) & 1))
                pb[(x << 2) + 0] = pb[(x << 2) + 1] = pb[(x << 2) + 2] = 0x88;
            else
                pb[(x << 2) + 0] = pb[(x << 2) + 1] = pb[(x << 2) + 2] = 0xFF;
            pb[(x << 2) + 3] = 0xFF;
        }
    }
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_subimage_8bpp_minus(HBITMAP hbm, int x, int y, int cx, int cy, int bpp)
{
    HBITMAP hbmNew;
    II_PALETTE *table;
    int n;

    assert(hbm);
    table = ii_get_palette(hbm);

    assert(0 < bpp && bpp <= 8);
    if (table)
    {
        n = (1 << bpp);
        if (n < table->num_colors)
            table->num_colors = n;
    }

    hbmNew = ii_create(cx, cy, bpp, table);
    assert(hbmNew);
    ii_palette_destroy(table);

    ii_put(hbmNew, 0, 0, hbm, x, y, cx, cy, NULL, 255);

    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_subimage_24bpp(HBITMAP hbm, int x, int y, int cx, int cy)
{
    HBITMAP hbmNew;

    assert(hbm);
    hbmNew = ii_create_24bpp(cx, cy);
    ii_put(hbmNew, 0, 0, hbm, x, y, cx, cy, NULL, 255);

    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_subimage_32bpp(HBITMAP hbm, int x, int y, int cx, int cy)
{
    HBITMAP hbmNew, hbmSrc;

    assert(hbm);
    hbmNew = NULL;
    hbmSrc = ii_clone(hbm);
    if (hbmSrc)
    {
        ii_premultiply(hbmSrc);
        hbmNew = ii_create_32bpp(cx, cy);
        ii_put(hbmNew, 0, 0, hbmSrc, x, y, cx, cy, NULL, 255);
        ii_destroy(hbmSrc);
    }

    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_subimage(HBITMAP hbm, int x, int y, int cx, int cy)
{
    HBITMAP hbmNew;
    BITMAP bm;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    switch (bm.bmBitsPixel)
    {
    case 32:
        hbmNew = ii_subimage_32bpp(hbm, x, y, cx, cy);
        break;
    case 24:
        hbmNew = ii_subimage_24bpp(hbm, x, y, cx, cy);
        break;
    default:
        hbmNew = ii_subimage_8bpp_minus(hbm, x, y, cx, cy, bm.bmBitsPixel);
    }
    assert(hbmNew);
    return hbmNew;
}

IMAIO_API void IIAPI
ii_destroy(HBITMAP hbm)
{
    if (hbm)
    {
        DeleteObject(hbm);
    }
}

IMAIO_API bool IIAPI
ii_get_info(HBITMAP hbm, BITMAP *pbm)
{
    assert(hbm);
    assert(pbm);
    return GetObject(hbm, sizeof(BITMAP), pbm) == sizeof(BITMAP);
}

IMAIO_API uint32_t IIAPI
ii_get_bpp(HBITMAP hbm)
{
    BITMAP bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return bm.bmBitsPixel;
    return 0;
}

IMAIO_API uint8_t * IIAPI
ii_get_pixels(HBITMAP hbm)
{
    BITMAP bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return (LPBYTE)bm.bmBits;
    return NULL;
}

IMAIO_API int IIAPI
ii_get_width(HBITMAP hbm)
{
    BITMAP bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return bm.bmWidth;
    return 0;
}

IMAIO_API int IIAPI
ii_get_height(HBITMAP hbm)
{
    BITMAP bm;
    assert(hbm);
    if (ii_get_info(hbm, &bm))
        return bm.bmHeight;
    return 0;
}

IMAIO_API II_PALETTE * IIAPI
ii_get_palette(HBITMAP hbm)
{
    HDC hdc;
    HGDIOBJ hbmOld;
    RGBQUAD colors[256];
    int i, num_colors;

    assert(hbm);
    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbm);
    num_colors = GetDIBColorTable(hdc, 0, 256, colors);
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);
    if (num_colors)
    {
        II_PALETTE *table = ii_palette_create(num_colors, NULL);
        if (table)
        {
            for (i = 0; i < num_colors; ++i)
            {
                table->colors[i].value[0] = colors[i].rgbBlue;
                table->colors[i].value[1] = colors[i].rgbGreen;
                table->colors[i].value[2] = colors[i].rgbRed;
            }
        }
        return table;
    }
    return NULL;
}

IMAIO_API bool IIAPI
ii_is_opaque(HBITMAP hbm)
{
    uint32_t cdw;
    uint8_t *pb;
    BITMAP bm;

    assert(hbm);
    if (!ii_get_info(hbm, &bm))
        return false;

    if (bm.bmBitsPixel <= 24)
        return true;

    cdw = bm.bmWidth * bm.bmHeight;
    pb = (uint8_t *)bm.bmBits;
    while (cdw--)
    {
        pb++; pb++; pb++;
        if (*pb++ != 0xFF)
            return false;
    }
    return true;
}

IMAIO_API void IIAPI
ii_erase_semitrans(HBITMAP hbm)
{
    uint32_t cdw;
    uint8_t *pb;
    BITMAP bm;

    assert(hbm);
    if (!ii_get_info(hbm, &bm) || bm.bmBitsPixel <= 24)
        return;

    cdw = bm.bmWidth * bm.bmHeight;
    pb = (uint8_t *)bm.bmBits;
    while (cdw--)
    {
        pb++; pb++; pb++;
        if (*pb != 0xFF)
            *pb = 0;
        pb++;
    }
}

IMAIO_API HBITMAP IIAPI
ii_clone(HBITMAP hbm)
{
    assert(hbm);
    hbm = (HBITMAP)CopyImage(hbm, IMAGE_BITMAP,
        0, 0, LR_CREATEDIBSECTION | LR_COPYRETURNORG);
    assert(hbm);
    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_stretched(HBITMAP hbm, int cxNew, int cyNew)
{
    BITMAP bm;

    assert(hbm);
    if (!ii_get_info(hbm, &bm))
        return NULL;

    if (bm.bmBitsPixel <= 24)
        return ii_stretched_24bpp(hbm, cxNew, cyNew);
    else
        return ii_stretched_32bpp(hbm, cxNew, cyNew);
}

IMAIO_API HBITMAP IIAPI
ii_stretched_24bpp(HBITMAP hbm, int cxNew, int cyNew)
{
    HBITMAP hbmNew;
    BITMAP bm;
    HDC hdc1, hdc2;
    HGDIOBJ hbm1Old, hbm2Old;

    assert(cxNew > 0);
    assert(cyNew > 0);
    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = ii_create_24bpp(cxNew, cyNew);
    if (hbmNew)
    {
        hdc1 = CreateCompatibleDC(NULL);
        hdc2 = CreateCompatibleDC(NULL);
        if (hdc1 && hdc2)
        {
            hbm1Old = SelectObject(hdc1, hbm);
            hbm2Old = SelectObject(hdc2, hbmNew);

            SetStretchBltMode(hdc2, STRETCH_HALFTONE);
            StretchBlt(hdc2, 0, 0, cxNew, cyNew,
                hdc1, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

            SelectObject(hdc1, hbm1Old);
            SelectObject(hdc2, hbm2Old);
            DeleteDC(hdc1);
            DeleteDC(hdc1);
            return hbmNew;
        }
        ii_destroy(hbmNew);
    }
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_stretched_32bpp(HBITMAP hbm, int cxNew, int cyNew)
{
    BITMAP bm;
    HBITMAP hbmNew;
    HDC hdc;
    BITMAPINFO bi;
    BYTE *pbNewBits, *pbBits, *pbNewLine, *pbLine0, *pbLine1;
    LONG nWidthBytes, nWidthBytesNew;
    BOOL fAlpha;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = NULL;
    hdc = CreateCompatibleDC(NULL);
    nWidthBytes = bm.bmWidth * 4;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = bm.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    fAlpha = (bm.bmBitsPixel == 32);
    pbBits = (BYTE *)malloc(nWidthBytes * bm.bmHeight);
    if (pbBits == NULL)
    {
        DeleteDC(hdc);
        return NULL;
    }

    GetDIBits(hdc, hbm, 0, bm.bmHeight, pbBits, &bi, DIB_RGB_COLORS);
    bi.bmiHeader.biWidth = cxNew;
    bi.bmiHeader.biHeight = cyNew;
    hbmNew = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS,
                              (VOID **)&pbNewBits, NULL, 0);
    if (hbmNew)
    {
        int ix, iy, x0, y0, x1, y1;
        DWORD x, y;
        DWORD wfactor, hfactor;
        BYTE r0, g0, b0, a0, r1, g1, b1, a1;
        DWORD c00, c01, c10, c11;
        DWORD ex0, ey0, ex1, ey1;

        nWidthBytesNew = cxNew * 4;
        wfactor = (bm.bmWidth << 8) / cxNew;
        hfactor = (bm.bmHeight << 8) / cyNew;
        if (!fAlpha)
            a0 = 255;
        for (iy = 0; iy < cyNew; iy++)
        {
            y = hfactor * iy;
            y0 = y >> 8;
            y1 = min(y0 + 1, (int)bm.bmHeight - 1);
            ey1 = y & 0xFF;
            ey0 = 0x100 - ey1;
            pbNewLine = pbNewBits + iy * nWidthBytesNew;
            pbLine0 = pbBits + y0 * nWidthBytes;
            pbLine1 = pbBits + y1 * nWidthBytes;
            for (ix = 0; ix < cxNew; ix++)
            {
                x = wfactor * ix;
                x0 = x >> 8;
                x1 = min(x0 + 1, (int)bm.bmWidth - 1);
                ex1 = x & 0xFF;
                ex0 = 0x100 - ex1;
                c00 = ((LPDWORD)pbLine0)[x0];
                c01 = ((LPDWORD)pbLine1)[x0];
                c10 = ((LPDWORD)pbLine0)[x1];
                c11 = ((LPDWORD)pbLine1)[x1];

                b0 = (BYTE)(((ex0 * (c00 & 0xFF)) + 
                            (ex1 * (c10 & 0xFF))) >> 8);
                b1 = (BYTE)(((ex0 * (c01 & 0xFF)) + 
                            (ex1 * (c11 & 0xFF))) >> 8);
                g0 = (BYTE)(((ex0 * ((c00 >> 8) & 0xFF)) + 
                            (ex1 * ((c10 >> 8) & 0xFF))) >> 8);
                g1 = (BYTE)(((ex0 * ((c01 >> 8) & 0xFF)) + 
                            (ex1 * ((c11 >> 8) & 0xFF))) >> 8);
                r0 = (BYTE)(((ex0 * ((c00 >> 16) & 0xFF)) + 
                            (ex1 * ((c10 >> 16) & 0xFF))) >> 8);
                r1 = (BYTE)(((ex0 * ((c01 >> 16) & 0xFF)) + 
                            (ex1 * ((c11 >> 16) & 0xFF))) >> 8);
                b0 = (BYTE)((ey0 * b0 + ey1 * b1) >> 8);
                g0 = (BYTE)((ey0 * g0 + ey1 * g1) >> 8);
                r0 = (BYTE)((ey0 * r0 + ey1 * r1) >> 8);

                if (fAlpha)
                {
                    a0 = (BYTE)(((ex0 * ((c00 >> 24) & 0xFF)) + 
                                (ex1 * ((c10 >> 24) & 0xFF))) >> 8);
                    a1 = (BYTE)(((ex0 * ((c01 >> 24) & 0xFF)) + 
                                (ex1 * ((c11 >> 24) & 0xFF))) >> 8);
                    a0 = (BYTE)((ey0 * a0 + ey1 * a1) >> 8);
                }
                ((LPDWORD)pbNewLine)[ix] = 
                    MAKELONG(MAKEWORD(b0, g0), MAKEWORD(r0, a0));
            }
        }
    }
    free(pbBits);
    DeleteDC(hdc);
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_24bpp(HBITMAP hbm)
{
    BITMAP bm;
    HBITMAP hbmNew;
    HDC hdc1, hdc2;
    HGDIOBJ hbm1Old, hbm2Old;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    if (bm.bmBitsPixel == 24)
        return ii_clone(hbm);

    hdc1 = CreateCompatibleDC(NULL);
    hdc2 = CreateCompatibleDC(NULL);
    hbmNew = ii_create_24bpp(bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        hbm1Old = SelectObject(hdc1, hbm);
        hbm2Old = SelectObject(hdc2, hbmNew);
        BitBlt(hdc2, 0, 0, bm.bmWidth, bm.bmHeight,
               hdc1, 0, 0, SRCCOPY);
        SelectObject(hdc1, hbm1Old);
        SelectObject(hdc2, hbm2Old);
    }

    DeleteDC(hdc2);
    DeleteDC(hdc1);

    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_32bpp(HBITMAP hbm)
{
    BITMAP bm;
    HBITMAP hbmNew;
    HDC hdc1, hdc2;
    HGDIOBJ hbm1Old, hbm2Old;
    LPBYTE pb;
    uint32_t cdw;

    hbmNew = NULL;
    if (!ii_get_info(hbm, &bm))
        return hbmNew;

    if (bm.bmBitsPixel == 32)
    {
        return ii_clone(hbm);
    }

    hdc1 = CreateCompatibleDC(NULL);
    hdc2 = CreateCompatibleDC(NULL);
    hbmNew = ii_create_32bpp(bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);

        hbm1Old = SelectObject(hdc1, hbm);
        hbm2Old = SelectObject(hdc2, hbmNew);
        BitBlt(hdc2, 0, 0, bm.bmWidth, bm.bmHeight,
               hdc1, 0, 0, SRCCOPY);
        SelectObject(hdc1, hbm1Old);
        SelectObject(hdc2, hbm2Old);

        pb = (LPBYTE)bm.bmBits;
        cdw = bm.bmWidth * bm.bmHeight;
        while (cdw--)
        {
            pb++; pb++; pb++;
            *pb++ = 0xFF;
        }
    }
    DeleteDC(hdc2);
    DeleteDC(hdc1);

    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_32bpp_from_trans_8bpp(HBITMAP hbm8bpp, const int *pi_trans)
{
    HBITMAP hbmNew;
    BITMAP bm, bmNew;
    LPBYTE pb, pbNew;
    int x, y;

    assert(hbm8bpp);
    if (pi_trans == NULL || *pi_trans == -1)
    {
        return ii_32bpp(hbm8bpp);
    }

    hbmNew = ii_32bpp(hbm8bpp);
    if (hbmNew)
    {
        ii_get_info(hbm8bpp, &bm);
        pb = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel != 8)
        {
            /* not 8bpp */
            DeleteObject(hbmNew);
            return NULL;
        }

        ii_get_info(hbmNew, &bmNew);
        pbNew = (LPBYTE)bmNew.bmBits;
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                if (pb[x + y * bm.bmWidthBytes] == *pi_trans)
                {
                    pbNew[((x + y * bm.bmWidth) << 2) + 3] = 0;
                }
            }
        }
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_24bpp_or_32bpp(HBITMAP hbm)
{
    BITMAP bm;

    if (ii_get_info(hbm, &bm))
    {
        if (bm.bmBitsPixel == 32)
            return ii_clone(hbm);
        else
            return ii_24bpp(hbm);
    }
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_grayscale_8bpp(HBITMAP hbm)
{
    BITMAP bm;
    int x, y;
    LPBYTE pbBits;
    HBITMAP hbmNew;
    HDC hdc;
    HGDIOBJ hbmOld;
    COLORREF rgb;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = ii_create_8bpp_grayscale(bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;

        hdc = CreateCompatibleDC(NULL);
        hbmOld = SelectObject(hdc, hbm);
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                rgb = GetPixel(hdc, x, bm.bmHeight - y - 1);
                pbBits[x + y * bm.bmWidthBytes] = (uint8_t)(
                    (
                        GetRValue(rgb) +
                        GetGValue(rgb) +
                        GetBValue(rgb)
                    ) / 3
                );
            }
        }
        SelectObject(hdc, hbmOld);
        DeleteDC(hdc);
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_grayscale_32bpp(HBITMAP hbm)
{
    BITMAP bm;
    HBITMAP hbmNew;
    HDC hdc;
    HGDIOBJ hbmOld;
    uint8_t b, alpha;
    uint32_t dw;
    LPDWORD pdw;
    int x, y;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbmNew = ii_32bpp(hbm);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pdw = (LPDWORD)bm.bmBits;

        hdc = CreateCompatibleDC(NULL);
        hbmOld = SelectObject(hdc, hbm);
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                dw = pdw[x + y * bm.bmWidth];
                b = (uint8_t)(
                    (
                        GetRValue(dw) +
                        GetGValue(dw) +
                        GetBValue(dw)
                    ) / 3
                );
                alpha = (dw >> 24);
                pdw[x + y * bm.bmWidth] = (
                    b | (b << 8) | (b << 16) | (alpha << 24)
                );
            }
        }
        SelectObject(hdc, hbmOld);
        DeleteDC(hdc);
    }
    return hbmNew;
}

IMAIO_API uint8_t IIAPI
ii_bound(int value)
{
    if (value > 255)
        return 255;
    if (value < 0)
        return 0;
    return value;
}

IMAIO_API void IIAPI
ii_premultiply(HBITMAP hbm32bpp)
{
    BITMAP bm;
    uint32_t cdw;
    LPBYTE pb;
    uint8_t alpha;
    ii_get_info(hbm32bpp, &bm);
    if (bm.bmBitsPixel == 32)
    {
        cdw = bm.bmWidth * bm.bmHeight;
        pb = (LPBYTE) bm.bmBits;
        while (cdw--)
        {
            alpha = pb[3];
            pb[0] = (uint8_t) ((uint32_t) pb[0] * alpha / 255);
            pb[1] = (uint8_t) ((uint32_t) pb[1] * alpha / 255);
            pb[2] = (uint8_t) ((uint32_t) pb[2] * alpha / 255);
            pb += 4;
        }
    }
}

IMAIO_API HBITMAP IIAPI
ii_rotated_32bpp(HBITMAP hbmSrc, double angle, bool fGrow)
{
    HDC hdc;
    HBITMAP hbm;
    BITMAP bm;
    BITMAPINFO bi;
    LPBYTE pbBits, pbBitsSrc;
    int32_t widthbytes, widthbytesSrc;
    int cost, sint;
    int cx, cy, x0, x1, y0, y1, px, py, qx, qy;
    uint8_t r0, g0, b0, a0, r1, g1, b1, a1;
    int mx, my;
    int x, y, ex0, ey0, ex1, ey1;

    if (!ii_get_info(hbmSrc, &bm))
        return NULL;

    if (fGrow)
    {
        cx = (int)(fabs(bm.bmWidth * cos(angle)) + fabs(bm.bmHeight * sin(angle)) + 0.5);
        cy = (int)(fabs(bm.bmWidth * sin(angle)) + fabs(bm.bmHeight * cos(angle)) + 0.5);
    }
    else
    {
        cx = bm.bmWidth;
        cy = bm.bmHeight;
    }

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = cx;
    bi.bmiHeader.biHeight   = cy;
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 32;

    widthbytesSrc = (bm.bmWidth << 2);
    widthbytes = (cx << 2);

    do
    {
        hdc = CreateCompatibleDC(NULL);
        if (hdc != NULL)
        {
            hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (VOID **)&pbBits,
                                   NULL, 0);
            if (hbm != NULL)
            {
                pbBitsSrc = (LPBYTE)malloc(widthbytesSrc * bm.bmHeight);
                if (pbBitsSrc != NULL)
                    break;
                DeleteObject(hbm);
            }
            DeleteDC(hdc);
        }
        return NULL;
    } while (0);

    px = (bm.bmWidth - 1) << 15;
    py = (bm.bmHeight - 1) << 15;
    qx = (cx - 1) << 7;
    qy = (cy - 1) << 7;
    cost = (int)(cos(angle) * 256);
    sint = (int)(sin(angle) * 256);
    bi.bmiHeader.biWidth    = bm.bmWidth;
    bi.bmiHeader.biHeight   = bm.bmHeight;
    GetDIBits(hdc, hbmSrc, 0, bm.bmHeight, pbBitsSrc, &bi, DIB_RGB_COLORS);
    if (bm.bmBitsPixel < 32)
    {
        UINT cdw = bm.bmWidth * bm.bmHeight;
        LPBYTE pb = pbBitsSrc;
        while (cdw--)
        {
            pb++;
            pb++;
            pb++;
            *pb++ = 0xFF;
        }
    }
    ZeroMemory(pbBits, widthbytes * cy);

    x = (0 - qx) * cost + (0 - qy) * sint + px;
    y = -(0 - qx) * sint + (0 - qy) * cost + py;
    for (my = 0; my < cy; my++)
    {
        /* x = (0 - qx) * cost + ((my << 8) - qy) * sint + px; */
        /* y = -(0 - qx) * sint + ((my << 8) - qy) * cost + py; */
        for (mx = 0; mx < cx; mx++)
        {
            /* x = ((mx << 8) - qx) * cost + ((my << 8) - qy) * sint + px; */
            /* y = -((mx << 8) - qx) * sint + ((my << 8) - qy) * cost + py; */
            x0 = x >> 16;
            x1 = min(x0 + 1, (int)bm.bmWidth - 1);
            ex1 = x & 0xFFFF;
            ex0 = 0x10000 - ex1;
            y0 = y >> 16;
            y1 = min(y0 + 1, (int)bm.bmHeight - 1);
            ey1 = y & 0xFFFF;
            ey0 = 0x10000 - ey1;
            if (0 <= x0 && x0 < bm.bmWidth && 0 <= y0 && y0 < bm.bmHeight)
            {
                uint32_t c00 = *(uint32_t *)&pbBitsSrc[(x0 << 2) + y0 * widthbytesSrc];
                uint32_t c01 = *(uint32_t *)&pbBitsSrc[(x0 << 2) + y1 * widthbytesSrc];
                uint32_t c10 = *(uint32_t *)&pbBitsSrc[(x1 << 2) + y0 * widthbytesSrc];
                uint32_t c11 = *(uint32_t *)&pbBitsSrc[(x1 << 2) + y1 * widthbytesSrc];
                b0 = (uint8_t)(((ex0 * (c00 & 0xFF)) + (ex1 * (c10 & 0xFF))) >> 16);
                b1 = (uint8_t)(((ex0 * (c01 & 0xFF)) + (ex1 * (c11 & 0xFF))) >> 16);
                g0 = (uint8_t)(((ex0 * ((c00 >> 8) & 0xFF)) + (ex1 * ((c10 >> 8) & 0xFF))) >> 16);
                g1 = (uint8_t)(((ex0 * ((c01 >> 8) & 0xFF)) + (ex1 * ((c11 >> 8) & 0xFF))) >> 16);
                r0 = (uint8_t)(((ex0 * ((c00 >> 16) & 0xFF)) + (ex1 * ((c10 >> 16) & 0xFF))) >> 16);
                r1 = (uint8_t)(((ex0 * ((c01 >> 16) & 0xFF)) + (ex1 * ((c11 >> 16) & 0xFF))) >> 16);
                a0 = (uint8_t)(((ex0 * ((c00 >> 24) & 0xFF)) + (ex1 * ((c10 >> 24) & 0xFF))) >> 16);
                a1 = (uint8_t)(((ex0 * ((c01 >> 24) & 0xFF)) + (ex1 * ((c11 >> 24) & 0xFF))) >> 16);
                b0 = (ey0 * b0 + ey1 * b1) >> 16;
                g0 = (ey0 * g0 + ey1 * g1) >> 16;
                r0 = (ey0 * r0 + ey1 * r1) >> 16;
                a0 = (ey0 * a0 + ey1 * a1) >> 16;
                *(uint32_t *)&pbBits[(mx << 2) + my * widthbytes] =
                    MAKELONG(MAKEWORD(b0, g0), MAKEWORD(r0, a0));
            }
            x += cost << 8;
            y -= sint << 8;
        }
        x -= cx * cost << 8;
        x += sint << 8;
        y -= -cx * sint << 8;
        y += cost << 8;
    }
    free(pbBitsSrc);
    DeleteDC(hdc);
    return hbm;
}

/* three bytes */
typedef struct II_TRIBYTE
{
    uint8_t value[3];
} II_TRIBYTE;

IMAIO_API HBITMAP IIAPI
ii_flipped_horizontal(HBITMAP hbmSrc)
{
    BITMAP bm;
    HBITMAP hbmNew;
    LPBYTE pbBits;
    int x, y;
    uint32_t dw;
    II_TRIBYTE tribytes;

    assert(hbmSrc);
    hbmNew = ii_24bpp_or_32bpp(hbmSrc);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel == 32)
        {
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < (bm.bmWidth >> 1); ++x)
                {
#define GETDWPIX(x, y) *(LPDWORD)&pbBits[((x) + (y) * bm.bmWidth) << 2]
                    dw = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(bm.bmWidth - x - 1, y);
                    GETDWPIX(bm.bmWidth - x - 1, y) = dw;
#undef GETDWPIX
                }
            }
        }
        else
        {
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < (bm.bmWidth >> 1); ++x)
                {
#define GETDWPIX(x, y) *(II_TRIBYTE *)&pbBits[(x) * 3 + (y) * bm.bmWidthBytes]
                    tribytes = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(bm.bmWidth - x - 1, y);
                    GETDWPIX(bm.bmWidth - x - 1, y) = tribytes;
#undef GETDWPIX
                }
            }
        }
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_flipped_vertical(HBITMAP hbmSrc)
{
    BITMAP bm;
    HBITMAP hbmNew;
    LPBYTE pbBits;
    int x, y;
    uint32_t dw;
    II_TRIBYTE tribytes;

    assert(hbmSrc);
    hbmNew = ii_24bpp_or_32bpp(hbmSrc);
    if (hbmNew)
    {
        ii_get_info(hbmNew, &bm);
        pbBits = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel == 32)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                for (y = 0; y < (bm.bmHeight >> 1); ++y)
                {
#define GETDWPIX(x, y) *(LPDWORD)&pbBits[((x) + (y) * bm.bmWidth) << 2]
                    dw = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(x, bm.bmHeight - y - 1);
                    GETDWPIX(x, bm.bmHeight - y - 1) = dw;
#undef GETDWPIX
                }
            }
        }
        else
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
                for (y = 0; y < (bm.bmHeight >> 1); ++y)
                {
#define GETDWPIX(x, y) *(II_TRIBYTE *)&pbBits[(x) * 3 + (y) * bm.bmWidthBytes]
                    tribytes = GETDWPIX(x, y);
                    GETDWPIX(x, y) = GETDWPIX(x, bm.bmHeight - y - 1);
                    GETDWPIX(x, bm.bmHeight - y - 1) = tribytes;
#undef GETDWPIX
                }
            }
        }
    }
    return hbmNew;
}

IMAIO_API void IIAPI
ii_make_opaque(HBITMAP hbm32bpp, int x, int y, int cx, int cy)
{
    BITMAP bm;
    DWORD dw;
    LPDWORD pdw;
    int xx, yy;

    if (!ii_get_info(hbm32bpp, &bm) || bm.bmBitsPixel != 32)
    {
        return;
    }

    pdw = (LPDWORD)bm.bmBits;
    for (yy = y; yy < y + cy; ++yy)
    {
        for (xx = x; xx < x + cx; ++xx)
        {
            dw = pdw[xx + yy * cx];
            dw |= 0xFF000000;
            pdw[xx + yy * cx] = dw;
        }
    }
}

IMAIO_API void IIAPI
ii_draw(
    HDC hdc, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    BITMAP bmSrc;
    HDC hdc2;
    HBITMAP hbmNewSrc;
    HGDIOBJ hbm2Old;
    BLENDFUNCTION bf;

    if (!ii_get_info(hbmSrc, &bmSrc))
        return;

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = bSCA;
    bf.AlphaFormat = AC_SRC_ALPHA;

    if (bmSrc.bmBitsPixel >= 24)
    {
        assert(pi_trans == NULL || *pi_trans == -1);
        hbmNewSrc = ii_32bpp(hbmSrc);
        ii_premultiply(hbmNewSrc);

        hdc2 = CreateCompatibleDC(NULL);
        hbm2Old = SelectObject(hdc2, hbmNewSrc);
        GdiAlphaBlend(hdc, x, y, cxSrc, cySrc,
                      hdc2, xSrc, ySrc, cxSrc, cySrc, bf);
        SelectObject(hdc2, hbm2Old);
        DeleteDC(hdc2);

        ii_destroy(hbmNewSrc);
    }
    else if (bmSrc.bmBitsPixel == 8)
    {
        hbmNewSrc = ii_32bpp_from_trans_8bpp(hbmSrc, pi_trans);
        ii_premultiply(hbmNewSrc);

        hdc2 = CreateCompatibleDC(NULL);
        hbm2Old = SelectObject(hdc2, hbmNewSrc);
        GdiAlphaBlend(hdc, x, y, cxSrc, cySrc,
                      hdc2, xSrc, ySrc, cxSrc, cySrc, bf);
        SelectObject(hdc2, hbm2Old);
        DeleteDC(hdc2);

        ii_destroy(hbmNewSrc);
    }
    else
    {
        /* other bpps are not supported yet */
        assert(0);
    }
}

IMAIO_API void IIAPI
ii_draw_center(
    HDC hdc, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    x -= cxSrc / 2;
    y -= cySrc / 2;

    ii_draw(hdc, x, y, hbmSrc, xSrc, ySrc, cxSrc, cySrc, pi_trans, bSCA);
}

IMAIO_API void IIAPI
ii_put(
    HBITMAP hbm, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    HDC hdc;
    HGDIOBJ hbmOld;

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbm);
    ii_draw(hdc, x, y, hbmSrc, xSrc, ySrc, cxSrc, cySrc, pi_trans, bSCA);
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);
}

IMAIO_API void IIAPI
ii_put_center(
    HBITMAP hbm, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans, BYTE bSCA)
{
    x -= cxSrc / 2;
    y -= cySrc / 2;

    ii_put(hbm, x, y, hbmSrc, xSrc, ySrc, cxSrc, cySrc, pi_trans, bSCA);
}

IMAIO_API void IIAPI
ii_stamp(HBITMAP hbm, int x, int y, HBITMAP hbmSrc,
         const int *pi_trans, BYTE bSCA)
{
    BITMAP bmSrc;
    ii_get_info(hbmSrc, &bmSrc);
    ii_put(hbm, x, y, hbmSrc, 0, 0, bmSrc.bmWidth, bmSrc.bmHeight,
           pi_trans, bSCA);
}

IMAIO_API void IIAPI
ii_stamp_center(
    HBITMAP hbm, int x, int y, HBITMAP hbmSrc,
    const int *pi_trans, BYTE bSCA)
{
    BITMAP bmSrc;
    ii_get_info(hbmSrc, &bmSrc);
    ii_put_center(
        hbm, x, y, hbmSrc, 0, 0, bmSrc.bmWidth, bmSrc.bmHeight,
        pi_trans, bSCA);
}

/*****************************************************************************/
/* screenshot */

IMAIO_API HBITMAP IIAPI
ii_screenshot(HWND window, const RECT *position)
{
    HBITMAP hbmNew;
    HDC hDC, hMemDC;
    HGDIOBJ hbmOld;
    RECT rc;
    int x, y, cx, cy;

    if (window == NULL)
        window = GetDesktopWindow();

    if (position)
    {
        x = position->left;
        y = position->top;
        cx = position->right - position->left;
        cy = position->bottom - position->top;
    }
    else
    {
        GetWindowRect(window, &rc);
        x = rc.left;
        y = rc.top;
        cx = rc.right - rc.left;
        cy = rc.bottom - rc.top;
    }

    hbmNew = NULL;
    hDC = GetWindowDC(window);
    if (hDC != NULL)
    {
        hMemDC = CreateCompatibleDC(hDC);
        if (hMemDC)
        {
            hbmNew = ii_create_24bpp(cx, cy);
            if (hbmNew)
            {
                hbmOld = SelectObject(hMemDC, hbmNew);
                BitBlt(hMemDC, 0, 0, cx, cy,
                       hDC, x, y, SRCCOPY | CAPTUREBLT);
                SelectObject(hMemDC, hbmOld);
            }
            DeleteDC(hMemDC);
        }
        ReleaseDC(window, hDC);
    }
    return hbmNew;
}

/*****************************************************************************/
/* colors */

IMAIO_API int32_t IIAPI
ii_color_distance(const II_COLOR8 *c1, const II_COLOR8 *c2)
{
    int32_t diff, sum = 0;
    diff = c1->value[0] - c2->value[0];
    sum += diff * diff;
    diff = c1->value[1] - c2->value[1];
    sum += diff * diff;
    diff = c1->value[2] - c2->value[2];
    sum += diff * diff;
    return sum;
}

IMAIO_API int32_t IIAPI
ii_color_distance_alpha(const II_COLOR8 *c1, const II_COLOR8 *c2)
{
    int32_t diff, sum = 0;
    diff = c1->value[0] - c2->value[0];
    sum += diff * diff;
    diff = c1->value[1] - c2->value[1];
    sum += diff * diff;
    diff = c1->value[2] - c2->value[2];
    sum += diff * diff;
    diff = c1->value[3] - c2->value[3];
    sum += diff * diff;
    return sum;
}

IMAIO_API II_PALETTE * IIAPI
ii_palette_create(int num_colors, const II_COLOR8 *colors)
{
    II_PALETTE *table;
    table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
    if (table == NULL)
        return NULL;

    table->num_colors = num_colors;
    if (colors)
        CopyMemory(table->colors, colors, sizeof(II_COLOR8) * num_colors);
    return table;
}

IMAIO_API II_PALETTE * IIAPI
ii_palette_fixed(bool web_safe)
{
    II_PALETTE *    table;
    int i = 0, j, rr, gg, bb;

    table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
    if (table == NULL)
        return NULL;
    for (rr = 0; rr <= 0xFF; rr += 0x33)
    {
        for (gg = 0; gg <= 0xFF; gg += 0x33)
        {
            for (bb = 0; bb <= 0xFF; bb += 0x33)
            {
                table->colors[i].value[0] = bb;
                table->colors[i].value[1] = gg;
                table->colors[i].value[2] = rr;
                ++i;
            }
        }
    }
    assert(i == 216);
    if (!web_safe)
    {
        for (j = 0; j < 40; ++j, ++i)
        {
            table->colors[i].value[0] = j * 0xFF / 40;
            table->colors[i].value[1] = j * 0xFF / 40;
            table->colors[i].value[2] = j * 0xFF / 40;
        }
    }
    table->num_colors = i;
    assert(i == 216 || i == 256);
    return table;
}

IMAIO_API void IIAPI
ii_palette_destroy(II_PALETTE *palette)
{
    free(palette);
}

/*****************************************************************************/
/* k-means */

/* entry of k-means */
typedef struct II_KMEANS_ENTRY
{
    II_COLOR8   true_color;
    II_COLOR8   trimmed_color;
    int32_t     count;
    int32_t     i_cluster;
} II_KMEANS_ENTRY;

/* cluster of k-means */
typedef struct II_KMEANS_CLUSTER
{
    II_COLOR32 centroid;
    int         count;
} II_KMEANS_CLUSTER;

/* maximum number of clusters */
#define II_KMEANS_MAX_CLUSTER 256

/* the k-means structure */
typedef struct II_KMEANS
{
    II_KMEANS_ENTRY *   entries;
    int                 num_entries;
    II_KMEANS_CLUSTER   clusters[II_KMEANS_MAX_CLUSTER];
} II_KMEANS;

/* trim a color */
IMAIO_API II_COLOR8 IIAPI
ii_color_trim(II_COLOR8 color)
{
    II_COLOR8 trimmed_color;
    trimmed_color.value[0] = (color.value[0] & 0xF8);
    trimmed_color.value[1] = (color.value[1] & 0xF8);
    trimmed_color.value[2] = (color.value[2] & 0xF8);
    trimmed_color.value[3] = 0;
    return trimmed_color;
}

/* compare two entries */
static ii_inline int IIAPI
ii_kmeans_equal_entry(const II_KMEANS_ENTRY *e1, const II_KMEANS_ENTRY *e2)
{
    return (
        e1->trimmed_color.value[0] == e2->trimmed_color.value[0] &&
        e1->trimmed_color.value[1] == e2->trimmed_color.value[1] &&
        e1->trimmed_color.value[2] == e2->trimmed_color.value[2]
    );
}

/* get distance */
static ii_inline int IIAPI
ii_kmeans_distance(const II_KMEANS_ENTRY *e1, const II_COLOR32 *c2)
{
    int diff, sum = 0;
    diff = e1->trimmed_color.value[0] - c2->value[0];
    sum += diff * diff;
    diff = e1->trimmed_color.value[1] - c2->value[1];
    sum += diff * diff;
    diff = e1->trimmed_color.value[2] - c2->value[2];
    sum += diff * diff;
    return sum;
}

/* add entry to the k-means structure */
static int IIAPI
ii_kmeans_add_entry(II_KMEANS *kms, II_KMEANS_ENTRY *entry)
{
    int i;
    II_KMEANS_ENTRY *entries;

    for (i = 0; i < kms->num_entries; ++i)
    {
        if (ii_kmeans_equal_entry(&kms->entries[i], entry))
        {
            kms->entries[i].true_color = entry->true_color;
            kms->entries[i].count++;
            return i;
        }
    }

    entries = (II_KMEANS_ENTRY *)realloc(
        kms->entries,
        (kms->num_entries + 1) * sizeof(II_KMEANS_ENTRY));
    if (entries)
    {
        entries[kms->num_entries] = *entry;
        kms->entries = entries;
        return kms->num_entries++;
    }
    return -1;
}

/* add color to the k-means structure */
static ii_inline int IIAPI
ii_kmeans_add_color(II_KMEANS *kms, II_COLOR8 color)
{
    II_KMEANS_ENTRY entry;
    entry.true_color = color;
    entry.trimmed_color = ii_color_trim(color);
    entry.count = 1;
    return ii_kmeans_add_entry(kms, &entry);
}

/* do the k-means */
static void IIAPI
ii_kmeans(II_KMEANS *kms, int num_colors)
{
    int i, j, m;
    const int times = 2;

    assert(num_colors > 0);
    if (num_colors > 256)
        num_colors = 256;

    for (m = 0; m < times; ++m)
    {
        for (i = 0; i < kms->num_entries; ++i)
        {
            int dist, min_dist = 0x7FFFFFFF, min_j = -1;
            for (j = 0; j < num_colors; ++j)
            {
                dist = ii_kmeans_distance(
                    &kms->entries[i], &kms->clusters[j].centroid);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    min_j = j;
                }
            }
            kms->entries[i].i_cluster = min_j;
            assert(min_j != -1);
        }

        for (j = 0; j < num_colors; ++j)
        {
            kms->clusters[j].centroid.value[0] = 0;
            kms->clusters[j].centroid.value[1] = 0;
            kms->clusters[j].centroid.value[2] = 0;
            kms->clusters[j].count = 0;
            for (i = 0; i < kms->num_entries; ++i)
            {
                if (j == kms->entries[i].i_cluster)
                {
                    kms->clusters[j].centroid.value[0] +=
                        kms->entries[i].true_color.value[0] *
                            kms->entries[i].count;
                    kms->clusters[j].centroid.value[1] +=
                        kms->entries[i].true_color.value[1] *
                            kms->entries[i].count;
                    kms->clusters[j].centroid.value[2] +=
                        kms->entries[i].true_color.value[2] *
                            kms->entries[i].count;
                    kms->clusters[j].count += kms->entries[i].count;
                }
            }
            if (kms->clusters[j].count)
            {
                kms->clusters[j].centroid.value[0] /= kms->clusters[j].count;
                kms->clusters[j].centroid.value[1] /= kms->clusters[j].count;
                kms->clusters[j].centroid.value[2] /= kms->clusters[j].count;
            }
            else
            {
                kms->clusters[j].centroid.value[0] = 0;
                kms->clusters[j].centroid.value[1] = 0;
                kms->clusters[j].centroid.value[2] = 0;
            }
        }
    }
}

/*****************************************************************************/
/* colors */

IMAIO_API II_PALETTE * IIAPI
ii_palette_for_pixels(int num_pixels, const uint32_t *pixels, int num_colors)
{
    int i;
    DWORD dw;
    II_COLOR8 color;
    II_PALETTE *table;
    II_KMEANS kms;

    if (num_colors < 0 || 256 < num_colors)
        num_colors = 256;
    
    if (num_pixels <= 1)
    {
        if (num_pixels == 1)
            dw = *pixels;
        else
            dw = 0;
        table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
        table->num_colors = 2;
        table->colors[0].value[0] = (uint8_t)(dw >> 0);
        table->colors[0].value[1] = (uint8_t)(dw >> 8);
        table->colors[0].value[2] = (uint8_t)(dw >> 16);
        table->colors[1].value[0] = 0xFF;
        table->colors[1].value[1] = 0xFF;
        table->colors[1].value[2] = 0xFF;
        return table;
    }

    /* initialize the k-means structure */
    ZeroMemory(&kms, sizeof(kms));

    for (i = 0; i < num_colors; ++i)
    {
        dw = pixels[rand() % num_pixels];
        kms.clusters[i].centroid.value[0] = (uint8_t)(dw >> 0);
        kms.clusters[i].centroid.value[1] = (uint8_t)(dw >> 8);
        kms.clusters[i].centroid.value[2] = (uint8_t)(dw >> 16);
    }

    /* store colors to the k-means structure */
    for (i = 0; i < num_pixels; ++i)
    {
        dw = pixels[i];
        color.value[0] = (uint8_t)(dw >> 0);
        color.value[1] = (uint8_t)(dw >> 8);
        color.value[2] = (uint8_t)(dw >> 16);
        ii_kmeans_add_color(&kms, color);
    }

    /* store colors to the k-means structure */
    for (i = 0; i < num_pixels; ++i)
    {
        dw = pixels[i];
        color.value[0] = (uint8_t)(dw >> 0);
        color.value[1] = (uint8_t)(dw >> 8);
        color.value[2] = (uint8_t)(dw >> 16);
        ii_kmeans_add_color(&kms, color);
    }

    /* just do it */
    ii_kmeans(&kms, num_colors);

    /* store colors to table */
    table = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
    if (table)
    {
        table->num_colors = num_colors;
        for (i = 0; i < num_colors; ++i)
        {
            table->colors[i].value[0] =
                (uint8_t)kms.clusters[i].centroid.value[0];
            table->colors[i].value[1] =
                (uint8_t)kms.clusters[i].centroid.value[1];
            table->colors[i].value[2] =
                (uint8_t)kms.clusters[i].centroid.value[2];
        }
    }

    /* release allocated memory */
    free(kms.entries);

    return table;
}

IMAIO_API II_PALETTE * IIAPI
ii_palette_optimized(HBITMAP hbm, int num_colors)
{
    HBITMAP hbmNew;
    BITMAP bm;
    LPDWORD pdw;
    DWORD dw, cdw;
    uint32_t *pixels;
    int num_pixels;
    II_PALETTE *table = NULL;

    if (num_colors < 0 || 256 < num_colors)
        num_colors = 256;

    if (!ii_get_info(hbm, &bm))
        return NULL;

    /* get smaller one */
    if (bm.bmWidth > 256)
        bm.bmWidth = 256;
    if (bm.bmHeight > 256)
        bm.bmHeight = 256;
    hbmNew = ii_stretched_32bpp(hbm, bm.bmWidth, bm.bmHeight);
    if (hbmNew)
    {
        /* get pixels */
        ii_get_info(hbmNew, &bm);
        pdw = (LPDWORD)bm.bmBits;
        cdw = bm.bmWidth * bm.bmHeight;
        pixels = calloc(sizeof(DWORD), cdw);
        if (pixels)
        {
            num_pixels = 0;
            while (cdw--)
            {
                dw = *pdw++;
                if (dw >> 24)
                {
                    pixels[num_pixels++] = (dw & 0xFFFFFF);
                }
            }

            table = ii_palette_for_pixels(num_pixels, pixels, num_colors);

            free(pixels);
        }

        ii_destroy(hbmNew);
    }

    return table;
}

IMAIO_API void IIAPI 
ii_palette_shrink(II_PALETTE *table, const int *pi_trans)
{
    int i;
    for (i = table->num_colors - 1; i > 0; --i)
    {
        II_COLOR8 *pcolor1 = &table->colors[i];
        II_COLOR8 *pcolor2 = &table->colors[i - 1];
        if (pcolor1->value[0] == pcolor2->value[0] &&
            pcolor1->value[1] == pcolor2->value[1] &&
            pcolor1->value[2] == pcolor2->value[2])
        {
            if (pi_trans && i == *pi_trans)
                break;
            --table->num_colors;
        }
        else
        {
            break;
        }
    }
}

IMAIO_API HBITMAP IIAPI 
ii_reduce_colors(
    HBITMAP hbm,
    const II_PALETTE *table,
    const int *pi_trans)
{
    BITMAP bm, bm32bpp;
    LPBYTE pb8bpp, pb32bpp;
    int x, y, index;
    HBITMAP hbm8bpp, hbm32bpp;

    assert(table);
    if (!ii_get_info(hbm, &bm))
        return NULL;

    hbm32bpp = ii_32bpp(hbm);
    if (hbm32bpp == NULL)
        return NULL;
    ii_get_info(hbm32bpp, &bm32bpp);
    pb32bpp = (LPBYTE)bm32bpp.bmBits;

    hbm8bpp = ii_create(bm.bmWidth, bm.bmHeight, 8, table);
    if (hbm8bpp)
    {
        uint8_t v0[4];
        uint8_t v1[4];
        int value[4];
        int k;

        ii_get_info(hbm8bpp, &bm);
        pb8bpp = (LPBYTE)bm.bmBits;

        /* Floyd-Steinberg dithering */
        for (y = 0; y < bm.bmHeight; ++y)
        {
            for (x = 0; x < bm.bmWidth; ++x)
            {
#define GETPX(x,y,i) pb32bpp[((x) << 2) + (y) * bm32bpp.bmWidthBytes + (i)]
                v0[0] = GETPX(x, y, 0);
                v0[1] = GETPX(x, y, 1);
                v0[2] = GETPX(x, y, 2);
                v0[3] = GETPX(x, y, 3);
                if (v0[3] == 0 && pi_trans && *pi_trans != -1)
                {
                    pb8bpp[x + y * bm.bmWidthBytes] = *pi_trans;
                    continue;
                }

                index = ii_color_nearest_index(table,
                    (II_COLOR8 *)
                        &pb32bpp[(x << 2) + y * bm32bpp.bmWidthBytes]);
                pb8bpp[x + y * bm.bmWidthBytes] = (uint8_t)index;

                v1[0] = table->colors[index].value[0];
                v1[1] = table->colors[index].value[1];
                v1[2] = table->colors[index].value[2];

                value[0] = (int)v0[0] - (int)v1[0];
                value[1] = (int)v0[1] - (int)v1[1];
                value[2] = (int)v0[2] - (int)v1[2];
                GETPX(x, y, 0) = v1[0];
                GETPX(x, y, 1) = v1[1];
                GETPX(x, y, 2) = v1[2];

                for (k = 0; k < 3; ++k)
                {
                    if (x + 1 < bm.bmWidth)
                    {
                        GETPX(x + 1, y + 0, k) =
                            ii_bound(GETPX(x + 1, y + 0, k) + value[k] * 7 / 16);
                    }
                    if (x > 0 && y + 1 < bm.bmHeight)
                    {
                        GETPX(x - 1, y + 1, k) =
                            ii_bound(GETPX(x - 1, y + 1, k) + value[k] * 3 / 16);
                    }
                    if (y + 1 < bm.bmHeight)
                    {
                        GETPX(x + 0, y + 1, k) =
                            ii_bound(GETPX(x + 0, y + 1, k) + value[k] * 5 / 16);
                    }
                    if (x + 1 < bm.bmWidth && y + 1 < bm.bmHeight)
                    {
                        GETPX(x + 1, y + 1, k) =
                            ii_bound(GETPX(x + 1, y + 1, k) + value[k] * 1 / 16);
                    }
                }
#undef GETPX
            }
        }
    }

    ii_destroy(hbm32bpp);
    return hbm8bpp;
}

IMAIO_API HBITMAP IIAPI
ii_alpha_channel_from_32bpp(HBITMAP hbm32bpp)
{
    BITMAP  bm;
    LPDWORD     pdw;
    HBITMAP   hbm8bpp;
    LPBYTE      pb;
    int         x, y;
    DWORD       dw;

    if (!ii_get_info(hbm32bpp, &bm))
        return NULL;

    pdw = (LPDWORD)bm.bmBits;

    hbm8bpp = ii_create_8bpp_grayscale(bm.bmWidth, bm.bmHeight);
    if (hbm8bpp)
    {
        ii_get_info(hbm8bpp, &bm);
        pb = (LPBYTE)bm.bmBits;
        if (bm.bmBitsPixel == 32)
        {
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < bm.bmWidth; ++x)
                {
                    dw = pdw[x + y * bm.bmWidth];
                    dw >>= 24;
                    pb[x + y * bm.bmWidthBytes] = (uint8_t)dw;
                }
            }
        }
        else
        {
            FillMemory(pb, bm.bmWidthBytes * bm.bmHeight, 0xFF);
        }
    }
    return hbm8bpp;
}

IMAIO_API HBITMAP IIAPI
ii_add_alpha_channel(HBITMAP hbmAlpha, HBITMAP hbm)
{
    HBITMAP hbm32bpp;

    hbm32bpp = ii_32bpp(hbm);
    if (hbm32bpp == NULL)
        return NULL;

    ii_store_alpha_channel(hbmAlpha, hbm32bpp);
    return hbm32bpp;
}

IMAIO_API void IIAPI 
ii_store_alpha_channel(HBITMAP hbmAlpha, HBITMAP hbm32bpp)
{
    BITMAP  bm;
    LPDWORD     pdw;
    int         x, y;
    DWORD       dw;
    HDC         hdc;
    HGDIOBJ     hbmOld;
    COLORREF    rgb;
    BYTE        r, g, b;

    if (!ii_get_info(hbm32bpp, &bm) || bm.bmBitsPixel != 32)
        return;

    pdw = (LPDWORD)bm.bmBits;

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbmAlpha);
    for (y = 0; y < bm.bmHeight; ++y)
    {
        for (x = 0; x < bm.bmWidth; ++x)
        {
            rgb = GetPixel(hdc, x, y);
            r = GetRValue(rgb);
            g = GetGValue(rgb);
            b = GetBValue(rgb);
            dw = pdw[x + y * bm.bmWidth];
            dw |= (((uint8_t)((r + g + b) / 3)) << 24);
            pdw[x + y * bm.bmWidth] = dw;
        }
    }
    SelectObject(hdc, hbmOld);
}

IMAIO_API HBITMAP IIAPI
ii_8bpp(HBITMAP hbm, int num_colors)
{
    HBITMAP hbmNew = NULL;
    II_PALETTE *table = ii_palette_optimized(hbm, num_colors);
    if (table)
    {
        hbmNew = ii_reduce_colors(hbm, table, NULL);
        free(table);
    }
    return hbmNew;
}

IMAIO_API HBITMAP IIAPI
ii_trans_8bpp(HBITMAP hbm, int *pi_trans)
{
    HBITMAP hbm8bpp, hbm32bpp;
    hbm32bpp = ii_32bpp(hbm);
    hbm8bpp = ii_trans_8bpp_from_32bpp(hbm32bpp, pi_trans);
    ii_destroy(hbm32bpp);
    return hbm8bpp;
}

IMAIO_API HBITMAP IIAPI
ii_trans_8bpp_from_32bpp(HBITMAP hbm32bpp, int *pi_trans)
{
    II_PALETTE *table;
    BITMAP bm32bpp, bm8bpp;
    LPBYTE pb;
    LPDWORD pdw;
    uint32_t dw;
    int x, y;
    HBITMAP hbm8bpp = NULL;

    if (pi_trans)
        table = ii_palette_optimized(hbm32bpp, 255);
    else
        table = ii_palette_optimized(hbm32bpp, 256);

    if (table)
    {
        if (pi_trans)
        {
            *pi_trans = table->num_colors;
            table->num_colors++;
        }
        hbm8bpp = ii_reduce_colors(hbm32bpp, table, pi_trans);
        free(table);
        if (hbm8bpp && pi_trans)
        {
            ii_get_info(hbm8bpp, &bm8bpp);
            pb = (LPBYTE)bm8bpp.bmBits;
            ii_get_info(hbm32bpp, &bm32bpp);
            pdw = (LPDWORD)bm32bpp.bmBits;
            for (y = 0; y < bm32bpp.bmHeight; ++y)
            {
                for (x = 0; x < bm32bpp.bmWidth; ++x)
                {
                    dw = *pdw++;
                    if ((dw >> 24) == 0)
                    {
                        pb[x + y * bm8bpp.bmWidthBytes] = (uint8_t)*pi_trans;
                    }
                }
            }
        }
    }
    return hbm8bpp;
}

IMAIO_API int IIAPI
ii_color_nearest_index(const II_PALETTE *table, const II_COLOR8 *pcolor)
{
    int i, i_near;
    int32_t norm, norm_near, i_value[3], k_value[3];

    assert(table);
    if (table == NULL)
        return 0;
    i_value[0] = pcolor->value[0];
    i_value[1] = pcolor->value[1];
    i_value[2] = pcolor->value[2];

    i_near = 0;
    norm_near = 255 * 255 * 3 + 1;
    for (i = 0; i < table->num_colors; ++i)
    {
        k_value[0] = (int)table->colors[i].value[0] - i_value[0];
        k_value[1] = (int)table->colors[i].value[1] - i_value[1];
        k_value[2] = (int)table->colors[i].value[2] - i_value[2];
        norm = k_value[0] * k_value[0] +
               k_value[1] * k_value[1] +
               k_value[2] * k_value[2];
        if (norm < norm_near)
        {
            i_near = i;
            norm_near = norm;
            if (norm == 0)
                break;
        }
    }

    assert(0 <= i_near && i_near < table->num_colors);
    return i_near;
}

/*****************************************************************************/

IMAIO_API HBITMAP IIAPI
ii_bmp_load_common(HANDLE hFile, HBITMAP hbm, float *dpi)
{
    BITMAPFILEHEADER bf;
    II_BITMAPINFOEX bi;
    DWORD cb, cbImage;
    LPVOID pBits, pBits2;
    HDC hDC, hMemDC;

    if (!ReadFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL))
    {
        CloseHandle(hFile);
        return hbm;
    }

    pBits = NULL;
    if (bf.bfType == 0x4D42 && bf.bfReserved1 == 0 && bf.bfReserved2 == 0 &&
        bf.bfSize > bf.bfOffBits && bf.bfOffBits > sizeof(BITMAPFILEHEADER) &&
        bf.bfOffBits <= sizeof(BITMAPFILEHEADER) + sizeof(II_BITMAPINFOEX))
    {
        if (ReadFile(hFile, &bi, bf.bfOffBits -
                     sizeof(BITMAPFILEHEADER), &cb, NULL))
        {
            if (dpi)
                *dpi = (float)(bi.bmiHeader.biXPelsPerMeter * 2.54 / 100.0);

            cbImage = bf.bfSize - bf.bfOffBits;
            pBits = malloc(cbImage);
            if (pBits)
            {
                if (ReadFile(hFile, pBits, cbImage, &cb, NULL))
                {
                    ;
                }
                else
                {
                    free(pBits);
                    pBits = NULL;
                }
            }
        }
    }
    CloseHandle(hFile);

    if (hbm)
    {
        free(pBits);
        return hbm;
    }

    hDC = CreateCompatibleDC(NULL);
    hMemDC = CreateCompatibleDC(hDC);
    hbm = CreateDIBSection(hMemDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
                           &pBits2, NULL, 0);
    if (hbm)
    {
        if (SetDIBits(hMemDC, hbm, 0, abs(bi.bmiHeader.biHeight),
                      pBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
        {
            ;
        }
        else
        {
            DeleteObject(hbm);
            hbm = NULL;
        }
    }
    DeleteDC(hMemDC);
    DeleteDC(hDC);

    free(pBits);

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_bmp_load_a(const char *pszFileName, float *dpi)
{
    HANDLE hFile;
    HBITMAP hbm;

    hbm = (HBITMAP)LoadImageA(NULL, pszFileName, IMAGE_BITMAP,
        0, 0, LR_LOADFROMFILE | LR_LOADREALSIZE | LR_CREATEDIBSECTION);

    hFile = CreateFileA(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
        return ii_bmp_load_common(hFile, hbm, dpi);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_bmp_load_w(const wchar_t *pszFileName, float *dpi)
{
    HANDLE hFile;
    HBITMAP hbm;

    hbm = (HBITMAP)LoadImageW(NULL, pszFileName, IMAGE_BITMAP,
        0, 0, LR_LOADFROMFILE | LR_LOADREALSIZE | LR_CREATEDIBSECTION);

    hFile = CreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
        return ii_bmp_load_common(hFile, hbm, dpi);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_bmp_load_res_a(HMODULE hInstance, const char *pszResName)
{
    return LoadImageA(hInstance, pszResName, IMAGE_BITMAP,
                      0, 0, LR_LOADREALSIZE);
}

IMAIO_API HBITMAP IIAPI
ii_bmp_load_res_w(HMODULE hInstance, const wchar_t *pszResName)
{
    return LoadImageW(hInstance, pszResName, IMAGE_BITMAP,
                      0, 0, LR_LOADREALSIZE);
}

IMAIO_API bool IIAPI
ii_bmp_save_common(HANDLE hFile, HBITMAP hbm, float dpi)
{
    BITMAPFILEHEADER bf;
    II_BITMAPINFOEX bi;
    BITMAPINFOHEADER *pbmih;
    DWORD cb;
    uint32_t cColors, cbColors;
    HDC hDC;
    LPVOID pvBits;
    BITMAP bm;
    bool f;

    if (!ii_get_info(hbm, &bm))
    {
        CloseHandle(hFile);
        return false;
    }

    pbmih = &bi.bmiHeader;
    ZeroMemory(pbmih, sizeof(BITMAPINFOHEADER));
    pbmih->biSize             = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth            = bm.bmWidth;
    pbmih->biHeight           = bm.bmHeight;
    pbmih->biPlanes           = 1;
    pbmih->biBitCount         = bm.bmBitsPixel;
    pbmih->biCompression      = BI_RGB;
    pbmih->biSizeImage        = bm.bmWidthBytes * bm.bmHeight;
    if (dpi != 0.0)
    {
        pbmih->biXPelsPerMeter = (int32_t)(dpi * 100 / 2.54 + 0.5);
        pbmih->biYPelsPerMeter = (int32_t)(dpi * 100 / 2.54 + 0.5);
    }

    if (bm.bmBitsPixel < 16)
        cColors = 1 << bm.bmBitsPixel;
    else
        cColors = 0;
    cbColors = cColors * sizeof(RGBQUAD);

    bf.bfType = 0x4d42;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    cb = sizeof(BITMAPFILEHEADER) + pbmih->biSize + cbColors;
    bf.bfOffBits = cb;
    bf.bfSize = cb + pbmih->biSizeImage;

    pvBits = malloc(pbmih->biSizeImage);
    if (pvBits == NULL)
    {
        CloseHandle(hFile);
        return false;
    }

    f = false;
    hDC = CreateCompatibleDC(NULL);
    if (GetDIBits(hDC, hbm, 0, bm.bmHeight, pvBits, (BITMAPINFO*)&bi,
                  DIB_RGB_COLORS))
    {
        f = WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL) &&
            WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &cb, NULL) &&
            WriteFile(hFile, bi.bmiColors, cbColors, &cb, NULL) &&
            WriteFile(hFile, pvBits, pbmih->biSizeImage, &cb, NULL);
    }
    DeleteDC(hDC);
    free(pvBits);
    if (!CloseHandle(hFile))
        f = false;
    return f;
}

IMAIO_API bool IIAPI
ii_bmp_save_a(const char *pszFileName, HBITMAP hbm, float dpi)
{
    HANDLE hFile;
    hFile = CreateFileA(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (ii_bmp_save_common(hFile, hbm, dpi))
        {
            return true;
        }
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_bmp_save_w(const wchar_t *pszFileName, HBITMAP hbm, float dpi)
{
    HANDLE hFile;
    hFile = CreateFileW(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (ii_bmp_save_common(hFile, hbm, dpi))
        {
            return true;
        }
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/
/* image types */

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_path_name_a(const char * pszFileName)
{
    const char *pchDotExt = ii_find_dotext_a(pszFileName);
    return ii_image_type_from_dotext_a(pchDotExt);
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_path_name_w(const wchar_t *pszFileName)
{
    const wchar_t *pchDotExt = ii_find_dotext_w(pszFileName);
    return ii_image_type_from_dotext_w(pchDotExt);
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_dotext_a(const char * pchDotExt)
{
    const char *pch;

    if (pchDotExt == NULL || *pchDotExt == 0)
        return II_IMAGE_TYPE_INVALID;
    if (*pchDotExt == '.')
        pch = pchDotExt + 1;
    else
        pch = pchDotExt;

    if (lstrcmpiA(pch, "jpg") == 0 || lstrcmpiA(pch, "jpe") == 0 ||
        lstrcmpiA(pch, "jpeg") == 0 || lstrcmpiA(pch, "jfif") == 0)
    {
        return II_IMAGE_TYPE_JPG;
    }
    else if (lstrcmpiA(pch, "gif") == 0)
    {
        return II_IMAGE_TYPE_GIF;
    }
    else if (lstrcmpiA(pch, "png") == 0)
    {
        return II_IMAGE_TYPE_PNG;
    }
    else if (lstrcmpiA(pch, "apng") == 0)
    {
        return II_IMAGE_TYPE_APNG;
    }
    else if (lstrcmpiA(pch, "tif") == 0 || lstrcmpiA(pch, "tiff") == 0)
    {
        return II_IMAGE_TYPE_TIF;
    }
    else if (lstrcmpiA(pch, "bmp") == 0 || lstrcmpiA(pch, "dib") == 0)
    {
        return II_IMAGE_TYPE_BMP;
    }
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_dotext_w(const wchar_t *pchDotExt)
{
    const wchar_t *pch;

    if (pchDotExt == NULL || *pchDotExt == 0)
        return II_IMAGE_TYPE_INVALID;
    if (*pchDotExt == L'.')
        pch = pchDotExt + 1;
    else
        pch = pchDotExt;

    if (lstrcmpiW(pch, L"jpg") == 0 || lstrcmpiW(pch, L"jpe") == 0 ||
        lstrcmpiW(pch, L"jpeg") == 0 || lstrcmpiW(pch, L"jfif") == 0)
    {
        return II_IMAGE_TYPE_JPG;
    }
    else if (lstrcmpiW(pch, L"gif") == 0)
    {
        return II_IMAGE_TYPE_GIF;
    }
    else if (lstrcmpiW(pch, L"png") == 0)
    {
        return II_IMAGE_TYPE_PNG;
    }
    else if (lstrcmpiW(pch, L"apng") == 0)
    {
        return II_IMAGE_TYPE_APNG;
    }
    else if (lstrcmpiW(pch, L"tif") == 0 || lstrcmpiW(pch, L"tiff") == 0)
    {
        return II_IMAGE_TYPE_TIF;
    }
    else if (lstrcmpiW(pch, L"bmp") == 0 || lstrcmpiW(pch, L"dib") == 0)
    {
        return II_IMAGE_TYPE_BMP;
    }
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API const char *IIAPI
ii_wildcards_from_image_type_a(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return "*.jpg;*.jpe;*.jpeg;*.jfif";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return "*.gif";
    case II_IMAGE_TYPE_PNG:
        return "*.png";
    case II_IMAGE_TYPE_APNG:
        return "*.png;*.apng";
    case II_IMAGE_TYPE_TIF:
        return "*.tif;*.tiff";
    case II_IMAGE_TYPE_BMP:
        return "*.bmp;*.dib";
    default:
        return "";
    }
}

IMAIO_API const wchar_t *IIAPI
ii_wildcards_from_image_type_w(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return L"*.jpg;*.jpe;*.jpeg;*.jfif";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return L"*.gif";
    case II_IMAGE_TYPE_PNG:
        return L"*.png";
    case II_IMAGE_TYPE_APNG:
        return L"*.png;*.apng";
    case II_IMAGE_TYPE_TIF:
        return L"*.tif;*.tiff";
    case II_IMAGE_TYPE_BMP:
        return L"*.bmp;*.dib";
    default:
        return L"";
    }
}

IMAIO_API const char *IIAPI
ii_find_file_title_a(const char *pszPath)
{
    LPCSTR pch1, pch2;

    assert(pszPath);
    #ifdef ANSI
        pch1 = strrchr(pszPath, '\\');
        pch2 = strrchr(pszPath, '/');
    #else
        pch1 = (LPCSTR)_mbsrchr((const BYTE *)(pszPath), '\\');
        pch2 = (LPCSTR)_mbsrchr((const BYTE *)(pszPath), '/');
    #endif
    if (pch1 == NULL && pch2 == NULL)
        return pszPath;
    if (pch1 == NULL)
        return pch2 + 1;
    if (pch2 == NULL)
        return pch1 + 1;
    if (pch1 < pch2)
        return pch2 + 1;
    else
        return pch1 + 1;
}

IMAIO_API const wchar_t *IIAPI
ii_find_file_title_w(const wchar_t *pszPath)
{
    LPCWSTR pch1, pch2;

    assert(pszPath);
    pch1 = wcsrchr(pszPath, L'\\');
    pch2 = wcsrchr(pszPath, L'/');
    if (pch1 == NULL && pch2 == NULL)
        return pszPath;
    if (pch1 == NULL)
        return pch2 + 1;
    if (pch2 == NULL)
        return pch1 + 1;
    if (pch1 < pch2)
        return pch2 + 1;
    else
        return pch1 + 1;
}

IMAIO_API const char *IIAPI
ii_find_dotext_a(const char *pszPath)
{
    LPCSTR pch;

    assert(pszPath);
    pszPath = ii_find_file_title_a(pszPath);
    #ifdef ANSI
        pch = strrchr(pszPath, '.');
    #else
        pch = (LPCSTR)_mbsrchr((const BYTE *)pszPath, '.');
    #endif
    if (pch)
        return pch;
    else
        return pszPath + lstrlenA(pszPath);
}

IMAIO_API const wchar_t *IIAPI
ii_find_dotext_w(const wchar_t *pszPath)
{
    LPCWSTR pch;

    assert(pszPath);
    pszPath = ii_find_file_title_w(pszPath);
    pch = wcsrchr(pszPath, L'.');
    if (pch)
        return pch;
    else
        return pszPath + lstrlenW(pszPath);
}

IMAIO_API const char *IIAPI
ii_mime_from_path_name_a(const char *pszFileName)
{
    const char *pchDotExt = ii_find_dotext_a(pszFileName);
    return ii_mime_from_dotext_a(pchDotExt);
}

IMAIO_API const wchar_t *IIAPI
ii_mime_from_path_name_w(const wchar_t *pszFileName)
{
    const wchar_t *pchDotExt = ii_find_dotext_w(pszFileName);
    return ii_mime_from_dotext_w(pchDotExt);
}

IMAIO_API const char *IIAPI
ii_mime_from_dotext_a(const char *pchDotExt)
{
    II_IMAGE_TYPE type = ii_image_type_from_dotext_a(pchDotExt);
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return "image/jpeg";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return "image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return "image/png";
    case II_IMAGE_TYPE_TIF:
        return "image/tiff";
    case II_IMAGE_TYPE_BMP:
        return "image/bmp";
    default:
        return "";
    }
}

IMAIO_API const wchar_t *IIAPI
ii_mime_from_dotext_w(const wchar_t *pchDotExt)
{
    II_IMAGE_TYPE type = ii_image_type_from_dotext_w(pchDotExt);
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return L"image/jpeg";
    case II_IMAGE_TYPE_GIF:
    case II_IMAGE_TYPE_ANIGIF:
        return L"image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return L"image/png";
    case II_IMAGE_TYPE_TIF:
        return L"image/tiff";
    case II_IMAGE_TYPE_BMP:
        return L"image/bmp";
    default:
        return L"";
    }
}

IMAIO_API const char *IIAPI
ii_dotext_from_mime_a(const char * pszMIME)
{
    if (lstrcmpiA(pszMIME, "image/jpeg") == 0)
        return ".jpg";
    if (lstrcmpiA(pszMIME, "image/gif") == 0)
        return ".gif";
    if (lstrcmpiA(pszMIME, "image/png") == 0)
        return ".png";
    if (lstrcmpiA(pszMIME, "image/tiff") == 0)
        return ".tif";
    if (lstrcmpiA(pszMIME, "image/bmp") == 0)
        return ".bmp";
    return "";
}

IMAIO_API const wchar_t *IIAPI
ii_dotext_from_mime_w(const wchar_t *pszMIME)
{
    if (lstrcmpiW(pszMIME, L"image/jpeg") == 0)
        return L".jpg";
    if (lstrcmpiW(pszMIME, L"image/gif") == 0)
        return L".gif";
    if (lstrcmpiW(pszMIME, L"image/png") == 0)
        return L".png";
    if (lstrcmpiW(pszMIME, L"image/tiff") == 0)
        return L".tif";
    if (lstrcmpiW(pszMIME, L"image/bmp") == 0)
        return L".bmp";
    return L"";
}

IMAIO_API const char *IIAPI
ii_mime_from_image_type_a(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return "image/jpeg";
    case II_IMAGE_TYPE_GIF:
        return "image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return "image/png";
    case II_IMAGE_TYPE_TIF:
        return "image/tiff";
    case II_IMAGE_TYPE_BMP:
        return "image/bmp";
    default:
        return "";
    }
}

IMAIO_API const wchar_t *IIAPI
ii_mime_from_image_type_w(II_IMAGE_TYPE type)
{
    switch (type)
    {
    case II_IMAGE_TYPE_JPG:
        return L"image/jpeg";
    case II_IMAGE_TYPE_GIF:
        return L"image/gif";
    case II_IMAGE_TYPE_PNG:
    case II_IMAGE_TYPE_APNG:
        return L"image/png";
    case II_IMAGE_TYPE_TIF:
        return L"image/tiff";
    case II_IMAGE_TYPE_BMP:
        return L"image/bmp";
    default:
        return L"";
    }
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_mime_a(const char *pszMIME)
{
    if (lstrcmpiA(pszMIME, "image/jpeg") == 0)
        return II_IMAGE_TYPE_JPG;
    if (lstrcmpiA(pszMIME, "image/gif") == 0)
        return II_IMAGE_TYPE_GIF;
    if (lstrcmpiA(pszMIME, "image/png") == 0)
        return II_IMAGE_TYPE_PNG;
    if (lstrcmpiA(pszMIME, "image/apng") == 0)
        return II_IMAGE_TYPE_APNG;
    if (lstrcmpiA(pszMIME, "image/tiff") == 0)
        return II_IMAGE_TYPE_TIF;
    if (lstrcmpiA(pszMIME, "image/bmp") == 0)
        return II_IMAGE_TYPE_BMP;
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API II_IMAGE_TYPE IIAPI
ii_image_type_from_mime_w(const wchar_t *pszMIME)
{
    if (lstrcmpiW(pszMIME, L"image/jpeg") == 0)
        return II_IMAGE_TYPE_JPG;
    if (lstrcmpiW(pszMIME, L"image/gif") == 0)
        return II_IMAGE_TYPE_GIF;
    if (lstrcmpiW(pszMIME, L"image/png") == 0)
        return II_IMAGE_TYPE_PNG;
    if (lstrcmpiW(pszMIME, L"image/apng") == 0)
        return II_IMAGE_TYPE_APNG;
    if (lstrcmpiW(pszMIME, L"image/tiff") == 0)
        return II_IMAGE_TYPE_TIF;
    if (lstrcmpiW(pszMIME, L"image/bmp") == 0)
        return II_IMAGE_TYPE_BMP;
    return II_IMAGE_TYPE_INVALID;
}

IMAIO_API char * IIAPI
ii_make_filter_a(char *pszFilter)
{
    char *pszSave;

    assert(pszFilter);
    pszSave = pszFilter;
    while (*pszFilter)
    {
        if (*pszFilter == '|')
        {
            *pszFilter = 0;
            pszFilter++;
        }
        else
        {
            pszFilter = CharNextA(pszFilter);
        }
    }
    return pszSave;
}

IMAIO_API wchar_t * IIAPI
ii_make_filter_w(wchar_t *pszFilter)
{
    wchar_t *pszSave;

    assert(pszFilter);
    pszSave = pszFilter;
    while (*pszFilter)
    {
        if (*pszFilter == L'|')
        {
            *pszFilter = 0;
        }
        pszFilter++;
    }
    return pszSave;
}

/*****************************************************************************/
