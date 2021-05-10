/*****************************************************************************/
/* imaio_test.c --- test program for imaio                                   */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#include "imaio.h"
#include <tchar.h>
#include <assert.h>

int main(void)
{
    int i, i_trans;

#ifdef HAVE_GIF
    puts("anigif --> gif");
    {
        II_ANIGIF *anigif = ii_anigif_load(_T("tests/images/earth.gif"), II_FLAG_USE_SCREEN);
        assert(anigif);
        printf("num_frames: %d\n", anigif->num_frames);
        printf("flags: %d\n", anigif->flags);
        printf("width: %d\n", anigif->width);
        printf("height: %d\n", anigif->height);
        printf("global_palette: %s\n", (anigif->global_palette? "present" : "none"));
        printf("iBackground: %d\n", anigif->iBackground);
        printf("loop_count: %d\n", anigif->loop_count);
        for (i = 0; i < anigif->num_frames; ++i)
        {
            char fname[64];
            II_ANIGIF_FRAME *frame = &anigif->frames[i];
            printf("\n[ anigif frame #%d ]\n", i);
            printf("x, y: %d, %d\n", frame->x, frame->y);
            printf("width, height: %d, %d\n", frame->width, frame->height);
            printf("iTransparent: %d\n", frame->iTransparent);
            printf("disposal: %d\n", frame->disposal);
            printf("delay: %d\n", frame->delay);
            printf("local_palette: %s\n", (frame->local_palette ? "present" : "none"));
            sprintf(fname, "tests/images/earth-frame-%02d-save.gif", i);
            ii_gif_save_a(fname, frame->hbmPart, &frame->iTransparent);
        }
        puts("anigif --> anigif");
        anigif->loop_count = 3;
        anigif->flags = II_FLAG_USE_SCREEN;
        ii_anigif_save(_T("tests/images/earth-save.gif"), anigif);

# if defined(HAVE_PNG) && defined(PNG_APNG_SUPPORTED)
        puts("anigif --> apng");
        {
            II_APNG *apng = ii_apng_from_anigif(anigif);
            ii_apng_save(_T("tests/images/earth-save.png"), apng);
            ii_apng_destroy(apng);
        }
# endif

        ii_anigif_destroy(anigif);
    }
#endif

#if defined(HAVE_PNG) && defined(PNG_APNG_SUPPORTED)
    puts("apng --> png");
    {
        II_APNG *apng = ii_apng_load(_T("tests/images/apng.png"), II_FLAG_USE_SCREEN);
        printf("width: %d\n", apng->width);
        printf("height: %d\n", apng->height);
        printf("num_frames: %d\n", apng->num_frames);
        for (i = 0; i < apng->num_frames; ++i)
        {
            char fname[64];
            II_APNG_FRAME *frame = &apng->frames[i];
            printf("\n[ apng frame #%d ]\n", i);
            printf("x_offset: %d\n", frame->x_offset);
            printf("y_offset: %d\n", frame->y_offset);
            printf("width: %d\n", frame->width);
            printf("height: %d\n", frame->height);
            if (frame->hbmScreen)
            {
                sprintf(fname, "tests/images/apng-screen-%03d-save.png", i);
                ii_png_save_a(fname, frame->hbmScreen, 0);
            }
            sprintf(fname, "tests/images/part-%03d-save.png", i);
            ii_png_save_a(fname, frame->hbmPart, 0);
        }

        puts("apng --> apng");
        ii_apng_save(_T("tests/images/apng-save.png"), apng);

# ifdef HAVE_GIF
        puts("apng --> anigif");
        {
            II_ANIGIF *anigif = ii_anigif_from_apng(apng, true);
            ii_anigif_save(_T("tests/images/apng-save.gif"), anigif);
            ii_anigif_destroy(anigif);
        }
# endif
        ii_apng_destroy(apng);
    }
#endif

#ifdef HAVE_JPEG
    puts("jpeg --> jpeg");
    {
        HBITMAP hbm = ii_jpg_load(_T("tests/images/flower.jpg"), NULL);
        ii_jpg_save(_T("tests/images/flower-save.jpg"), hbm, 100, false, 0);
        ii_destroy(hbm);
    }
    puts("jpeg --> bmp");
    {
        HBITMAP hbm = ii_jpg_load(_T("tests/images/flower.jpg"), NULL);
        ii_bmp_save(_T("tests/images/flower-save.bmp"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("bmp --> jpeg");
    {
        HBITMAP hbm = ii_bmp_load(_T("tests/images/lena.bmp"), NULL);
        ii_jpg_save(_T("tests/images/lena-save.jpg"), hbm, 50, false, 0);
        ii_destroy(hbm);
    }
#endif

#ifdef HAVE_GIF
    puts("gif --> gif");
    {
        HBITMAP hbm = ii_gif_load_8bpp(_T("tests/images/sunflower.gif"), &i_trans);
        ii_gif_save(_T("tests/images/sunflower-save.gif"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("gif --> bmp");
    {
        HBITMAP hbm = ii_gif_load_8bpp(_T("tests/images/sunflower.gif"), &i_trans);
        ii_bmp_save(_T("tests/images/sunflower-save.bmp"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("gif --> gif");
    {
        HBITMAP hbm = ii_gif_load_8bpp(_T("tests/images/sunflower.gif"), &i_trans);
        ii_gif_save(_T("tests/images/sunflower-save.gif"), hbm, &i_trans);
        ii_destroy(hbm);
    }
#endif

#ifdef HAVE_PNG
    puts("png --> png");
    {
        HBITMAP hbm = ii_png_load(_T("tests/images/glasses.png"), NULL);
        ii_png_save(_T("tests/images/glasses-save.png"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("png --> bmp");
    {
        HBITMAP hbm = ii_png_load(_T("tests/images/glasses.png"), NULL);
        ii_bmp_save(_T("tests/images/glasses-save.bmp"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("bmp --> png");
    {
        HBITMAP hbm = ii_bmp_load(_T("tests/images/lena.bmp"), NULL);
        ii_png_save(_T("tests/images/lena-save.png"), hbm, 0);
        ii_destroy(hbm);
    }
#endif

#ifdef HAVE_TIFF
    puts("tiff --> tiff");
    {
        HBITMAP hbm = ii_tif_load(_T("tests/images/lena.tif"), NULL);
        ii_tif_save(_T("tests/images/lena-save.tif"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("tif --> bmp");
    {
        HBITMAP hbm = ii_tif_load(_T("tests/images/lena.tif"), NULL);
        ii_bmp_save(_T("tests/images/lena-save.bmp"), hbm, 0);
        ii_destroy(hbm);
    }
    puts("bmp --> tif");
    {
        HBITMAP hbm = ii_bmp_load(_T("tests/images/lena.bmp"), NULL);
        ii_tif_save(_T("tests/images/lena-save.tif"), hbm, 0);
        ii_destroy(hbm);
    }
#endif

#if defined(HAVE_GIF)
    puts("generate gif anime");
    {
        HDC hDC = NULL;
        int num_frames = 9;
        II_ANIGIF *anigif = calloc(1, sizeof(II_ANIGIF));
        anigif->flags = 0;
        anigif->width = 100;
        anigif->height = 100;
        anigif->global_palette = ii_palette_fixed(false);
        anigif->iBackground = -1;
        anigif->num_frames = num_frames;
        anigif->frames = calloc(sizeof(II_ANIGIF_FRAME), num_frames);
        anigif->loop_count = 0;
        hDC = CreateCompatibleDC(NULL);
        if (hDC)
        {
            int i;
            TCHAR szText[32];
            for (i = 0; i < num_frames; ++i)
            {
                HBITMAP hbm = ii_create_8bpp_solid(100, 100, anigif->global_palette, 0);
                if (hbm)
                {
                    RECT rc = { 0, 0, 100, 100 };
                    HGDIOBJ hbmOld = SelectObject(hDC, hbm);
                    wsprintf(szText, TEXT("%u"), i + 1);
                    SetTextColor(hDC, 0);
                    SetBkColor(hDC, RGB(255, 255, 255));
                    SetBkMode(hDC, OPAQUE);
                    DrawText(hDC, szText, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    SelectObject(hDC, hbmOld);

                    anigif->frames[i].x = 0;
                    anigif->frames[i].y = 0;
                    anigif->frames[i].width = anigif->width;
                    anigif->frames[i].height = anigif->height;
                    anigif->frames[i].iTransparent = -1;
                    anigif->frames[i].disposal = 0;
                    anigif->frames[i].delay = 1000;
                    anigif->frames[i].hbmPart = hbm;
                }
            }
            DeleteDC(hDC);
        }
        ii_anigif_save(_T("tests/images/generated-save.gif"), anigif);
        ii_anigif_destroy(anigif);
    }
#endif

    return 0;
}
