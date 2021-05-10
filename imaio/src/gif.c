/*****************************************************************************/
/* gif.c --- imaio for GIF                                                   */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#include "imaio_impl.h"

#ifdef HAVE_GIF

/*****************************************************************************/

IMAIO_API void IIAPI
ii_gif_uninterlace(GifByteType *bits, int width, int height)
{
    int i, j, k;
    GifByteType *new_bits = malloc(width * height);
    if (new_bits == NULL)
        return;
    k = 0;
    for (i = 0; i < 4; i++)
    {
        static int InterlacedOffset[] = { 0, 4, 2, 1 };
        static int InterlacedJumps[] = { 8, 8, 4, 2 };
        for (j = InterlacedOffset[i]; j < height; j += InterlacedJumps[i])
        {
            CopyMemory(&new_bits[j * width], &bits[k * width], width);
        }
    }
    CopyMemory(bits, new_bits, width * height);
    free(new_bits);
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_8bpp_common(GifFileType *gif, int *pi_trans/* = NULL*/)
{
    GifRowType *ScreenBuffer;
    int i, j, Size;
    int top, left, Width, Height;
    GifRecordType RecordType;
    ColorMapObject *ColorMap;
    II_BITMAPINFOEX bi;
    LPBYTE pbBits;
    HBITMAP hbm;
    HDC hdc;

    if (pi_trans)
        *pi_trans = -1;

    ScreenBuffer = (GifRowType *)calloc(gif->SHeight, sizeof(GifRowType));
    if (ScreenBuffer == NULL)
    {
        DGifCloseFile(gif, NULL);
        return NULL;
    }

    Size = gif->SWidth * sizeof(GifPixelType);
    for (i = 0; i < gif->SHeight; i++)
    {
        ScreenBuffer[i] = (GifRowType) malloc(Size);
        for (j = 0; j < gif->SWidth; j++)
            ScreenBuffer[i][j] = gif->SBackGroundColor;
    }

    do
    {
        DGifGetRecordType(gif, &RecordType);
        switch (RecordType)
        {
        case IMAGE_DESC_RECORD_TYPE:
            DGifGetImageDesc(gif);

            top = gif->Image.Top;
            left = gif->Image.Left;
            Width = gif->Image.Width;
            Height = gif->Image.Height;

            if (gif->Image.Interlace)
            {
                for (i = 0; i < 4; i++)
                {
                    static int InterlacedOffset[] = { 0, 4, 2, 1 };
                    static int InterlacedJumps[] = { 8, 8, 4, 2 };
                    for (j = top + InterlacedOffset[i]; j < top + Height;
                         j += InterlacedJumps[i])
                    {
                        DGifGetLine(gif, &ScreenBuffer[j][left], Width);
                    }
                }
            }
            else
            {
                j = top;
                for (i = 0; i < Height; i++)
                {
                    DGifGetLine(gif, &ScreenBuffer[j++][left], Width);
                }
            }
            goto hell;

        case EXTENSION_RECORD_TYPE:
            {
                GifByteType *Extension;
                int ExtCode;
                DGifGetExtension(gif, &ExtCode, &Extension);
                while (Extension != NULL)
                {
                    if (ExtCode == GRAPHICS_EXT_FUNC_CODE)
                    {
                        /* WORD Delay = Extension[2] | (Extension[3] << 8); */
                        if (Extension[1] & 1)
                        {
                            if (pi_trans)
                                *pi_trans = Extension[4];
                        }
                    }
                    DGifGetExtensionNext(gif, &Extension);
                }
            }
            break;

        default:
            break;
        }
    } while (RecordType != TERMINATE_RECORD_TYPE);

hell:
    if (gif->Image.ColorMap)
        ColorMap = gif->Image.ColorMap;
    else
        ColorMap = gif->SColorMap;

    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = gif->SWidth;
    bi.bmiHeader.biHeight = gif->SHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 8;
    bi.bmiHeader.biClrUsed = ColorMap->ColorCount;
    assert(ColorMap);

    for (i = 0; i < ColorMap->ColorCount; ++i)
    {
        bi.bmiColors[i].rgbBlue = ColorMap->Colors[i].Blue;
        bi.bmiColors[i].rgbGreen = ColorMap->Colors[i].Green;
        bi.bmiColors[i].rgbRed = ColorMap->Colors[i].Red;
        bi.bmiColors[i].rgbReserved = 0;
    }

    DGifCloseFile(gif, NULL);

    hdc = CreateCompatibleDC(NULL);
    hbm = CreateDIBSection(hdc, (LPBITMAPINFO)&bi, DIB_RGB_COLORS,
                           (void **)&pbBits, NULL, 0);
    DeleteDC(hdc);
    if (hbm)
    {
        int widthbytes = II_WIDTHBYTES(bi.bmiHeader.biWidth * 8);
        for (i = 0; i < bi.bmiHeader.biHeight; i++)
        {
            GifRowType row = ScreenBuffer[i];
            for (j = 0; j < bi.bmiHeader.biWidth; j++)
            {
                int y = bi.bmiHeader.biHeight - i - 1;
                pbBits[y * widthbytes + j] = row[j];
            }
        }
    }

    for (i = 0; i < bi.bmiHeader.biHeight; i++)
    {
        free(ScreenBuffer[i]);
    }
    free(ScreenBuffer);

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_8bpp_a(const char *pszFileName, int *pi_trans/* = NULL*/)
{
    GifFileType *gif = DGifOpenFileName(pszFileName, NULL);
    if (gif)
        return ii_gif_load_8bpp_common(gif, pi_trans);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_8bpp_w(const wchar_t *pszFileName, int *pi_trans/* = NULL*/)
{
    GifFileType *gif;
    int fd;

    fd = _wopen(pszFileName, O_RDONLY);
    if (fd != -1) {
        gif = DGifOpenFileHandle(fd, NULL);
        if (gif)
            return ii_gif_load_8bpp_common(gif, pi_trans);
        _close(fd);
    }
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_32bpp_a(const char *pszFileName)
{
    int i_trans;
    HBITMAP hbm8bpp, hbm32bpp;
    assert(pszFileName);
    hbm8bpp = ii_gif_load_8bpp_a(pszFileName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_32bpp_w(const wchar_t *pszFileName)
{
    int i_trans;
    HBITMAP hbm8bpp, hbm32bpp;
    assert(pszFileName);
    hbm8bpp = ii_gif_load_8bpp_w(pszFileName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

static int IICAPI
ii_gif_mem_read(GifFileType *gif, GifByteType *bytes, int length)
{
    II_MEMORY *memory;
    assert(gif);
    memory = (II_MEMORY *)gif->UserData;
    assert(memory);
    assert(bytes);
    if (memory->m_i + length <= memory->m_size)
    {
        CopyMemory(bytes, memory->m_pb + memory->m_i, length);
        memory->m_i += length;
        return length;
    }
    return 0;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_8bpp_mem(const void *pv, uint32_t cb, int *pi_trans)
{
    GifFileType *gif;
    II_MEMORY memory;

    memory.m_pb = (const uint8_t *)pv;
    memory.m_i = 0;
    memory.m_size = cb;

    gif = DGifOpen(&memory, ii_gif_mem_read, NULL);
    assert(gif);
    if (gif)
        return ii_gif_load_8bpp_common(gif, pi_trans);
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_32bpp_mem(const void *pv, uint32_t cb)
{
    int i_trans;
    HBITMAP hbm8bpp, hbm32bpp;
    hbm8bpp = ii_gif_load_8bpp_mem(pv, cb, &i_trans);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_8bpp_res_a(HMODULE hInstance, const char *pszResName, int *pi_trans)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    HBITMAP hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceA(hInstance, pszResName, "GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_gif_load_8bpp_mem(lpData, dwSize, pi_trans);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_32bpp_res_a(HMODULE hInstance, const char *pszResName)
{
    int i_trans;
    HBITMAP hbm8bpp, hbm32bpp;
    assert(pszResName);
    hbm8bpp = ii_gif_load_8bpp_res_a(hInstance, pszResName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_8bpp_res_w(HMODULE hInstance, const wchar_t *pszResName, int *pi_trans)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    HBITMAP hbm;
    LPVOID lpData;
    HRSRC hRsrc;

    assert(pszResName);
    hRsrc = FindResourceW(hInstance, pszResName, L"GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    hbm = ii_gif_load_8bpp_mem(lpData, dwSize, pi_trans);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return hbm;
}

IMAIO_API HBITMAP IIAPI
ii_gif_load_32bpp_res_w(HMODULE hInstance, const wchar_t *pszResName)
{
    int i_trans;
    HBITMAP hbm8bpp, hbm32bpp;
    assert(pszResName);
    hbm8bpp = ii_gif_load_8bpp_res_w(hInstance, pszResName, &i_trans);
    assert(hbm8bpp);
    if (hbm8bpp)
    {
        hbm32bpp = ii_32bpp_from_trans_8bpp(hbm8bpp, &i_trans);
        assert(hbm32bpp);
        ii_destroy(hbm8bpp);
        return hbm32bpp;
    }
    return NULL;
}

IMAIO_API bool IIAPI 
ii_gif_save_common(GifFileType *gif, HBITMAP hbm8bpp, const int *pi_trans)
{
    BITMAP bm;
    RGBQUAD table[256];
    LPBYTE pbBits;
    HDC hdc;
    HGDIOBJ hbmOld;
    int nColorCount;
    ColorMapObject cm;
    GifColorType colors[256];
    int i;
    GifPixelType* ScanLines;

    if (!ii_get_info(hbm8bpp, &bm) || bm.bmBitsPixel != 8)
    {
        assert(0);
        EGifCloseFile(gif, NULL);
        return false;
    }
    pbBits = (LPBYTE)bm.bmBits;
    assert(pbBits);

    ScanLines = (GifPixelType*)calloc(sizeof(GifPixelType), bm.bmWidth);
    if (ScanLines == NULL)
    {
        EGifCloseFile(gif, NULL);
        return false;
    }

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, hbm8bpp);
    nColorCount = GetDIBColorTable(hdc, 0, 256, table);
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);

    cm.ColorCount = nColorCount;
    cm.BitsPerPixel = 8;
    cm.Colors = colors;
    for (i = 0; i < nColorCount; i++)
    {
        cm.Colors[i].Blue = table[i].rgbBlue;
        cm.Colors[i].Green = table[i].rgbGreen;
        cm.Colors[i].Red = table[i].rgbRed;
    }

    EGifPutScreenDesc(gif, bm.bmWidth, bm.bmHeight, 256, 0, &cm);
    if (pi_trans && *pi_trans != -1)
    {
        uint8_t extension[4];
        extension[0] = 1;   /* enable transparency */
        extension[1] = 0;   /* no delay */
        extension[2] = 0;   /* no delay */
        extension[3] = (uint8_t)*pi_trans;     /* tranparent index */
        EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, 4, extension);
    }
    EGifPutImageDesc(gif, 0, 0, bm.bmWidth, bm.bmHeight, false, NULL);
    for (i = 0; i < bm.bmHeight; i++)
    {
        int y = bm.bmHeight - i - 1;
        EGifPutLine(gif, &pbBits[y * bm.bmWidthBytes], bm.bmWidth);
    }

    EGifCloseFile(gif, NULL);
    free(ScanLines);

    return true;
}

IMAIO_API bool IIAPI
ii_gif_save_a(const char *pszFileName, HBITMAP hbm8bpp, const int *pi_trans)
{
    GifFileType *gif;

    gif = EGifOpenFileName(pszFileName, false, NULL);
    assert(gif);
    if (gif)
    {
        if (ii_gif_save_common(gif, hbm8bpp, pi_trans))
        {
            return true;
        }
        DeleteFileA(pszFileName);
    }
    return false;
}

IMAIO_API bool IIAPI
ii_gif_save_w(const wchar_t *pszFileName, HBITMAP hbm8bpp, const int *pi_trans)
{
    GifFileType *gif = NULL;
    int fd = _wopen(pszFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (fd != -1)
    {
        gif = EGifOpenFileHandle(fd, NULL);
        if (gif == NULL)
            _close(fd);
    }
    assert(gif);
    if (gif)
    {
        if (ii_gif_save_common(gif, hbm8bpp, pi_trans))
        {
            return true;
        }
        DeleteFileW(pszFileName);
    }
    return false;
}

/*****************************************************************************/

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_common(GifFileType *gif, II_FLAGS flags)
{
    ColorMapObject *cm;
    II_PALETTE *palette;
    II_ANIGIF *anigif;
    int i, k, n, x, y;
    int ret;
    SavedImage *image;
    II_ANIGIF_FRAME *frame;

    assert(gif);
    if (gif == NULL)
    {
        return NULL;
    }
    ret = DGifSlurp(gif);
    if (ret == GIF_ERROR)
        return NULL;

    anigif = (II_ANIGIF *)calloc(sizeof(II_ANIGIF), 1);
    if (anigif == NULL)
    {
        DGifCloseFile(gif, NULL);
        return NULL;
    }

    if (flags & II_FLAG_USE_SCREEN)
        anigif->flags = II_FLAG_USE_SCREEN;
    else
        anigif->flags = 0;
    anigif->width = gif->SWidth;
    anigif->height = gif->SHeight;
    anigif->num_frames = gif->ImageCount;
    anigif->iBackground = gif->SBackGroundColor;

    anigif->frames =
        (II_ANIGIF_FRAME *)
            calloc(sizeof(II_ANIGIF_FRAME), anigif->num_frames);
    if (anigif->frames == NULL)
    {
        ii_anigif_destroy(anigif);
        DGifCloseFile(gif, NULL);
        return NULL;
    }

    cm = gif->SColorMap;
    if (cm)
    {
        palette = (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
        if (palette == NULL)
        {
            ii_anigif_destroy(anigif);
            DGifCloseFile(gif, NULL);
            return NULL;
        }
        palette->num_colors = cm->ColorCount;
        for (i = 0; i < cm->ColorCount; ++i)
        {
            palette->colors[i].value[0] = cm->Colors[i].Blue;
            palette->colors[i].value[1] = cm->Colors[i].Green;
            palette->colors[i].value[2] = cm->Colors[i].Red;
        }
        anigif->global_palette = palette;
    }
    else
    {
        anigif->global_palette = NULL;
    }

    for (i = 0; i < anigif->num_frames; ++i)
    {
        image = &(gif->SavedImages[i]);
        frame = &anigif->frames[i];

        frame->x = image->ImageDesc.Left;
        frame->y = image->ImageDesc.Top;
        frame->width = image->ImageDesc.Width;
        frame->height = image->ImageDesc.Height;
        frame->iTransparent = -1;
        frame->disposal = 0;
        frame->delay = 0;
        if (image->ImageDesc.ColorMap)
        {
            cm = image->ImageDesc.ColorMap;
            frame->local_palette =
                (II_PALETTE *)calloc(sizeof(II_PALETTE), 1);
            if (frame->local_palette == NULL)
            {
                ii_anigif_destroy(anigif);
                DGifCloseFile(gif, NULL);
                return NULL;
            }
            palette = frame->local_palette;
            palette->num_colors = cm->ColorCount;
            for (n = 0; n < cm->ColorCount; ++n)
            {
                palette->colors[n].value[0] = cm->Colors[n].Blue;
                palette->colors[n].value[1] = cm->Colors[n].Green;
                palette->colors[n].value[2] = cm->Colors[n].Red;
            }
        }
        for (k = 0; k < image->ExtensionBlockCount; ++k)
        {
            ExtensionBlock *block = &image->ExtensionBlocks[k];
            switch (block->Function)
            {
            case GRAPHICS_EXT_FUNC_CODE:
                if (block->ByteCount >= 4)
                {
                    n = block->Bytes[0];
                    if (n & 1)
                        frame->iTransparent = block->Bytes[3];
                    frame->disposal = ((n >> 2) & 0x07);
                    frame->delay = 10 *
                        (block->Bytes[1] | (block->Bytes[2] << 8));
                }
                break;
            case APPLICATION_EXT_FUNC_CODE:
                if (block->ByteCount == 11)
                {
                    if (memcmp(block->Bytes, "NETSCAPE2.0", 11) == 0)
                    {
                        if (i + 1 < image->ExtensionBlockCount)
                        {
                            ++block;
                            if (block->ByteCount == 3 && (block->Bytes[0] & 7) == 1)
                            {
                                anigif->loop_count =
                                    ((block->Bytes[1] & 0xFF) |
                                        ((block->Bytes[2] & 0xFF) << 8));
                            }
                        }
                    }
                }
                break;
            }
        }

        if (frame->local_palette)
            palette = frame->local_palette;
        else
            palette = anigif->global_palette;
        frame->hbmPart = ii_create(frame->width, frame->height, 8, palette);
        assert(frame->hbmPart);
        if (frame->hbmPart == NULL)
        {
            ii_anigif_destroy(anigif);
            DGifCloseFile(gif, NULL);
            return NULL;
        }
        {
            BITMAP bm;
            LPBYTE pb;

            ii_get_info(frame->hbmPart, &bm);
            pb = (LPBYTE)bm.bmBits;
            if (image->ImageDesc.Interlace)
            {
                ii_gif_uninterlace(
                    image->RasterBits, frame->width, frame->height);
            }
            for (y = 0; y < frame->height; ++y)
            {
                for (x = 0; x < frame->width; ++x)
                {
                    pb[x + (bm.bmHeight - y - 1) * bm.bmWidthBytes] =
                       image->RasterBits[x + y * frame->width];
                }
            }
        }
    }

    /* realize 32bpp */
    if (flags & II_FLAG_USE_SCREEN)
    {
        HBITMAP hbmScreen;
        BITMAP bmScreen;
        II_ANIGIF_FRAME *old_frame;

        /* create screen */
        hbmScreen = ii_create_32bpp_trans(anigif->width, anigif->height);
        if (hbmScreen == NULL)
        {
            ii_anigif_destroy(anigif);
            DGifCloseFile(gif, NULL);
            return NULL;
        }
        ii_get_info(hbmScreen, &bmScreen);

        /* realize */
        old_frame = NULL;
        for (i = 0; i < anigif->num_frames; ++i)
        {
            frame = &anigif->frames[i];

            ii_stamp(hbmScreen, frame->x, frame->y, frame->hbmPart,
                     &frame->iTransparent, 255);
            if (frame->hbmScreen)
                ii_destroy(frame->hbmScreen);
            frame->hbmScreen = ii_clone(hbmScreen);
            assert(hbmScreen);
            assert(frame->hbmScreen);

            switch (frame->disposal)
            {
            case 2:
                if (frame->local_palette)
                    palette = frame->local_palette;
                else
                    palette = anigif->global_palette;
                if (anigif->iBackground != -1)
                {
                    int xx, yy;
                    LPDWORD pdw;
                    DWORD dw;

                    dw = *(LPDWORD)(&palette->colors[anigif->iBackground]);
                    dw &= 0xFFFFFF;
                    pdw = (LPDWORD)bmScreen.bmBits;

                    for (yy = frame->y; yy < frame->y + frame->height; ++yy)
                    {
                        for (xx = frame->x; xx < frame->x + frame->width; ++xx)
                        {
                            pdw[xx + yy * frame->width] = dw;
                        }
                    }
                }
                break;
            case 3:
                if (old_frame)
                {
                    ii_destroy(hbmScreen);
                    hbmScreen = ii_clone(old_frame->hbmScreen);
                }
                break;
            }

            old_frame = frame;
        }

        ii_destroy(hbmScreen);
    }

    DGifCloseFile(gif, NULL);
    return anigif;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_a(const char *pszFileName, II_FLAGS flags)
{
    GifFileType *gif = DGifOpenFileName(pszFileName, NULL);
    if (gif)
        return ii_anigif_load_common(gif, flags);
    return NULL;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_w(const wchar_t *pszFileName, II_FLAGS flags)
{
    GifFileType *gif;
    int fd;

    fd = _wopen(pszFileName, O_RDONLY);
    if (fd != -1) {
        gif = DGifOpenFileHandle(fd, NULL);
        if (gif)
            return ii_anigif_load_common(gif, flags);
        _close(fd);
    }
    return NULL;
}

IMAIO_API bool IIAPI
ii_anigif_save_common(GifFileType *gif, II_ANIGIF *anigif)
{
    int i, k;
    II_PALETTE *palette;
    GifColorType colors[256];
    int ret, iTransparent;
    II_COLOR8 bg_color;

    assert(gif);
    assert(anigif);

    if (gif == NULL)
    {
        return false;
    }
    if (anigif == NULL)
    {
        EGifCloseFile(gif, NULL);
        return false;
    }

    assert(anigif->width > 0);
    assert(anigif->height > 0);
    assert(anigif->num_frames > 0);
    if (anigif->iBackground < 0 || 256 <= anigif->iBackground)
        anigif->iBackground = 0;

    /* initialize basic info */
    gif->SWidth = anigif->width;
    gif->SHeight = anigif->height;
    gif->SColorResolution = 8;
    gif->SBackGroundColor = anigif->iBackground;
    gif->ImageCount = anigif->num_frames;
    gif->SavedImages =
        (SavedImage *)calloc(anigif->num_frames, sizeof(SavedImage));
    if (gif->SavedImages == NULL)
    {
        EGifCloseFile(gif, NULL);
        return false;
    }

    palette = anigif->global_palette;
    if (palette && anigif->iBackground < palette->num_colors)
    {
        bg_color = palette->colors[anigif->iBackground];
    }
    else
    {
        ZeroMemory(&bg_color, sizeof(bg_color));
    }

    /* realize 8bpp */
    if (anigif->flags & II_FLAG_USE_SCREEN)
    {
        HBITMAP hbm8bpp;
        iTransparent = -1;
        if (anigif->global_palette == NULL)
        {
            bool fulfilled = true;
            for (i = 0; i < anigif->num_frames; ++i)
            {
                if (anigif->frames[i].local_palette == NULL)
                {
                    fulfilled = false;
                }
            }
            if (!fulfilled)
            {
                palette = ii_palette_for_anigif(anigif, 255);
                assert(palette);
                if (palette == NULL)
                {
                    EGifCloseFile(gif, NULL);
                    return false;
                }
                iTransparent = palette->num_colors;
                palette->num_colors++;
                anigif->global_palette = palette;
            }
        }
        for (i = 0; i < anigif->num_frames; ++i)
        {
            BITMAP info;
            II_ANIGIF_FRAME *frame = &(anigif->frames[i]);
            if (frame->hbmPart)
            {
                ii_destroy(frame->hbmPart);
                frame->hbmPart = NULL;
            }
            if (frame->hbmScreen == NULL)
                continue;

            ii_get_info(frame->hbmScreen, &info);
            if (info.bmWidth != frame->width ||
                info.bmHeight != frame->height)
            {
                frame->x = 0;
                frame->y = 0;
                frame->width = info.bmWidth;
                frame->height = info.bmHeight;
            }

            if (iTransparent != -1)
                frame->iTransparent = iTransparent;

            if (frame->local_palette)
            {
                hbm8bpp = ii_reduce_colors(
                    frame->hbmScreen, frame->local_palette,
                    &frame->iTransparent);
            }
            else
            {
                hbm8bpp = ii_reduce_colors(
                    frame->hbmScreen, anigif->global_palette,
                    &frame->iTransparent);
            }
            assert(hbm8bpp);
            if (hbm8bpp == NULL)
            {
                EGifCloseFile(gif, NULL);
                return false;
            }
            frame->hbmPart = hbm8bpp;
        }
    }

    /* global palette */
    if (anigif->global_palette)
    {
        palette = anigif->global_palette;
        for (i = 0; i < palette->num_colors; ++i)
        {
            II_COLOR8 *pcolor = &palette->colors[i];
            colors[i].Red = pcolor->value[2];
            colors[i].Green = pcolor->value[1];
            colors[i].Blue = pcolor->value[0];
        }
        gif->SColorMap = GifMakeMapObject(palette->num_colors, colors);
        if (gif->SColorMap == NULL)
        {
            EGifCloseFile(gif, NULL);
            return false;
        }
    }
    else
    {
        gif->SColorMap = NULL;
    }

    for (i = 0; i < anigif->num_frames; ++i)
    {
        II_ANIGIF_FRAME *frame = &(anigif->frames[i]);
        SavedImage *image = &(gif->SavedImages[i]);

        /* frame info */
        image->ImageDesc.Left = frame->x;
        image->ImageDesc.Top = frame->y;
        image->ImageDesc.Width = frame->width;
        image->ImageDesc.Height = frame->height;
        image->ImageDesc.Interlace = false;

        /* local palette */
        if (frame->local_palette)
        {
            palette = frame->local_palette;
            for (k = 0; k < palette->num_colors; ++k)
            {
                II_COLOR8 *pcolor = &palette->colors[k];
                colors[k].Red = pcolor->value[2];
                colors[k].Green = pcolor->value[1];
                colors[k].Blue = pcolor->value[0];
            }
            image->ImageDesc.ColorMap =
                GifMakeMapObject(palette->num_colors, colors);
            if (image->ImageDesc.ColorMap == NULL)
            {
                EGifCloseFile(gif, NULL);
                return false;
            }
        }

        /* extension */
        if (image->ExtensionBlocks)
        {
            GifFreeExtensions(
                &image->ExtensionBlockCount, &image->ExtensionBlocks);
        }
        if (i == 0 && anigif->loop_count)
        {
            /* APPLICATION_EXT_FUNC_CODE */
            uint8_t params[3];

            GifAddExtensionBlock(
                &image->ExtensionBlockCount, &image->ExtensionBlocks,
                APPLICATION_EXT_FUNC_CODE, 11, (uint8_t *)"NETSCAPE2.0");

            params[0] = 1;
            params[1] = (uint8_t)(anigif->loop_count);
            params[2] = (uint8_t)(anigif->loop_count >> 8);
            GifAddExtensionBlock(
                &image->ExtensionBlockCount, &image->ExtensionBlocks,
                0, 3, params);
        }
        {
            /* GRAPHICS_EXT_FUNC_CODE */
            uint8_t extension[4];
            uint16_t delay = frame->delay / 10;
            extension[0] = (uint8_t)(frame->iTransparent != -1);
            extension[0] |= (uint8_t)((frame->disposal & 0x07) << 2);
            extension[1] = (uint8_t)delay;
            extension[2] = (uint8_t)(delay >> 8);
            extension[3] = (uint8_t)frame->iTransparent;
            GifAddExtensionBlock(
                &image->ExtensionBlockCount, &image->ExtensionBlocks,
                GRAPHICS_EXT_FUNC_CODE, 4, extension);
        }

        /* image bits */
        if (frame->hbmPart == NULL)
        {
            free(image->RasterBits);
            image->RasterBits = NULL;
            assert(0);
        }
        else
        {
            BITMAP bm;
            LPBYTE pb;
            int x, y;
            BYTE b;

            ii_get_info(frame->hbmPart, &bm);
            pb = (LPBYTE)bm.bmBits;

            /* allocate bits */
            free(image->RasterBits);
            image->RasterBits = (uint8_t *)malloc(bm.bmWidth * bm.bmHeight);
            if (image->RasterBits == NULL)
            {
                EGifCloseFile(gif, NULL);
                return false;
            }

            /* store bits */
            k = 0;
            for (y = 0; y < bm.bmHeight; ++y)
            {
                for (x = 0; x < bm.bmWidth; ++x)
                {
                    b = pb[x + (bm.bmHeight - y - 1) * bm.bmWidthBytes];
                    image->RasterBits[k++] = b;
                }
            }
        }
    }

    ret = EGifSpew(gif);
    assert(ret != GIF_ERROR);

    if (ret == GIF_ERROR)
        EGifCloseFile(gif, NULL);

    return ret != GIF_ERROR;
}

IMAIO_API bool IIAPI
ii_anigif_save_a(const char *pszFileName, II_ANIGIF *anigif)
{
    GifFileType *gif;

    assert(anigif);
    gif = EGifOpenFileName(pszFileName, false, NULL);
    assert(gif);
    if (gif)
    {
        if (ii_anigif_save_common(gif, anigif))
        {
            return true;
        }
    }
    DeleteFileA(pszFileName);
    return false;
}

IMAIO_API bool IIAPI
ii_anigif_save_w(const wchar_t *pszFileName, II_ANIGIF *anigif)
{
    GifFileType *gif = NULL;
    int fd = _wopen(pszFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (fd != -1)
    {
        gif = EGifOpenFileHandle(fd, NULL);
        if (gif == NULL)
            _close(fd);
    }
    assert(gif);
    assert(anigif);
    if (gif)
    {
        if (ii_anigif_save_common(gif, anigif))
        {
            return true;
        }
    }
    DeleteFileW(pszFileName);
    return false;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_mem(const void *pv, uint32_t cb, II_FLAGS flags)
{
    GifFileType *gif;
    II_MEMORY memory;

    memory.m_pb = (const uint8_t *)pv;
    memory.m_i = 0;
    memory.m_size = cb;
    gif = DGifOpen(&memory, ii_gif_mem_read, NULL);
    assert(gif);
    if (gif)
        return ii_anigif_load_common(gif, flags);
    return NULL;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_res_a(
    HMODULE hInstance, const char *pszResName, II_FLAGS flags)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    LPVOID lpData;
    HRSRC hRsrc;
    II_ANIGIF *anigif;

    assert(pszResName);
    hRsrc = FindResourceA(hInstance, pszResName, "GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    anigif = ii_anigif_load_mem(lpData, dwSize, flags);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return anigif;
}

IMAIO_API II_ANIGIF * IIAPI
ii_anigif_load_res_w(
    HMODULE hInstance, const wchar_t *pszResName, II_FLAGS flags)
{
    HGLOBAL hGlobal;
    uint32_t dwSize;
    LPVOID lpData;
    HRSRC hRsrc;
    II_ANIGIF *anigif;

    assert(pszResName);
    hRsrc = FindResourceW(hInstance, pszResName, L"GIF");
    if (hRsrc == NULL)
        return NULL;

    dwSize = SizeofResource(hInstance, hRsrc);
    hGlobal = LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL)
        return NULL;

    lpData = LockResource(hGlobal);
    anigif = ii_anigif_load_mem(lpData, dwSize, flags);

#ifdef WIN16
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
#endif

    return anigif;
}

IMAIO_API void IIAPI
ii_anigif_destroy(II_ANIGIF *anigif)
{
    int i;
    II_ANIGIF_FRAME *frame;

    if (anigif)
    {
        if (anigif->frames)
        {
            for (i = 0; i < anigif->num_frames; ++i)
            {
                frame = &anigif->frames[i];
                if (frame->hbmPart)
                {
                    ii_destroy(frame->hbmPart);
                    frame->hbmPart = NULL;
                }
                if (frame->hbmScreen)
                {
                    ii_destroy(frame->hbmScreen);
                    frame->hbmScreen = NULL;
                }
                if (frame->local_palette)
                {
                    ii_palette_destroy(frame->local_palette);
                    frame->local_palette = NULL;
                }
            }
            free(anigif->frames);
            anigif->frames = NULL;
        }
        if (anigif->global_palette)
        {
            ii_palette_destroy(anigif->global_palette);
            anigif->global_palette = NULL;
        }
        free(anigif);
    }
}

#ifdef HAVE_GIF
    IMAIO_API II_PALETTE * IIAPI 
    ii_palette_for_anigif(II_ANIGIF *anigif, int32_t num_colors)
    {
        HBITMAP hbm, hbmNew;
        BITMAP bm;
        LPDWORD pdw;
        DWORD dw, cdw;
        uint32_t *pixels, *all_pixels, *new_all_pixels;
        int num_pixels, num_all_pixels;
        II_PALETTE *table;
        int i;

        if (num_colors < 0 || 256 < num_colors)
            num_colors = 256;

        all_pixels = NULL;
        num_all_pixels = 0;

        assert(anigif->frames);
        if (anigif->frames == NULL)
            return NULL;

        for (i = 0; i < anigif->num_frames; ++i)
        {
            hbm = anigif->frames[i].hbmScreen;
            if (hbm == NULL)
                continue;

            if (!ii_get_info(hbm, &bm))
                return NULL;

            assert(bm.bmBitsPixel == 32);

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
                assert(bm.bmBitsPixel == 32);

                pdw = (LPDWORD)bm.bmBits;
                cdw = bm.bmWidth * bm.bmHeight;
                num_pixels = 0;
                pixels = (uint32_t *)calloc(sizeof(uint32_t), cdw);
                if (pixels)
                {
                    while (cdw--)
                    {
                        dw = *pdw++;
                        if (dw >> 24)
                        {
                            pixels[num_pixels++] = (dw & 0xFFFFFF);
                        }
                    }
                }
                ii_destroy(hbmNew);

                if (pixels == NULL)
                {
                    free(all_pixels);
                    return NULL;
                }

                new_all_pixels =
                    (uint32_t *)realloc(all_pixels,
                        (num_all_pixels + num_pixels) * sizeof(uint32_t));
                if (new_all_pixels == NULL)
                {
                    free(pixels);
                    free(all_pixels);
                    return NULL;
                }
                all_pixels = new_all_pixels;

                CopyMemory(all_pixels + num_all_pixels, pixels,
                           num_pixels * sizeof(uint32_t));
                num_all_pixels += num_pixels;

                free(pixels);
            }
        }

        table = ii_palette_for_pixels(num_all_pixels, all_pixels, num_colors);
        assert(table);
        free(all_pixels);
        return table;
    }
#endif

/*****************************************************************************/

#endif  /* def HAVE_GIF */
