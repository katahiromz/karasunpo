/*****************************************************************************/
/* imaio.h --- image file I/O libraries collection                           */
/* Copyright (C) 2015-2019 katahiromz. All Rights Reserved.                  */
/*****************************************************************************/

#ifndef KATAHIROMZ_IMAIO_H_
#define KATAHIROMZ_IMAIO_H_ 0x006   /* Version 0.6 */

/*****************************************************************************/

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef __cplusplus
    #include <cstdlib>
    #include <cstdio>
#else
    #include <stdlib.h>
    #include <stdio.h>
#endif

#if __cplusplus >= 201103L
    #include <cstdint>
#elif __STDC_VERSION__ >= 199901L
    #include <stdint.h>
#else
    #include "pstdint.h"     /* int??_t, uint??_t */
#endif

#ifndef __cplusplus
    #include "pstdbool.h"    /* bool, true, false */
#endif

#if defined(_WIN32) && !defined(WONVER)
    #ifndef _INC_WINDOWS
        #include <windows.h>
    #endif
#else
    #include "wonver.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

#ifdef _WIN32
    #ifdef IMAIO_DLL
        #ifdef IMAIO_BUILDING
            #define IMAIO_API   __declspec(dllexport)
        #else
            #define IMAIO_API   __declspec(dllimport)
        #endif
    #else
        #define IMAIO_API   /* empty */
    #endif
#else
    #define IMAIO_API       /* empty */
#endif  /* def _WIN32 */

/*****************************************************************************/
/* inline keyword wrapper */

#ifndef ii_inline
    #define ii_inline  __inline
#endif

/*****************************************************************************/
/* for optional parameters */

#ifdef __cplusplus
    #define ii_optional        = 0
    #define ii_optional_(def)  = def
#else
    #define ii_optional
    #define ii_optional_(def)
#endif

/*****************************************************************************/
/* IIAPI --- imaio API calling convention */

#ifdef _WIN32
    #define IIAPI   __stdcall
    #define IICAPI  __cdecl
#else
    #define IIAPI   /* empty */
    #define IICAPI  /* empty */
#endif

/*****************************************************************************/
/* flags */

typedef unsigned int II_FLAGS;

/* NOTE: II_FLAG_USE_SCREEN indicates the function uses screen image. */
#define II_FLAG_USE_SCREEN          1
/* NOTE: II_FLAG_DEFAULT_PRESENT indicates the default image of APNG exists. */
#define II_FLAG_DEFAULT_PRESENT     2

/*****************************************************************************/
/* structures */

/* color */
typedef struct II_COLOR8
{
    /* value[0: blue, 1: green, 2: red, 3:alpha] */
    uint8_t value[4];
} II_COLOR8;
/* NOTE: II_COLOR8 is compatible to RGBQUAD and COLORREF. */

/* wide color */
typedef struct II_COLOR32
{
    /* value[0: blue, 1: green, 2: red, 3:alpha] */
    int32_t value[4];
} II_COLOR32;

/* palette (color table) */
typedef struct II_PALETTE
{
    II_COLOR8  colors[256];
    int         num_colors;
} II_PALETTE;

/* animated gif frame */
typedef struct II_ANIGIF_FRAME
{
    int                 x, y;           /* position */
    int                 width, height;  /* pixel size */
    int                 iTransparent;   /* -1 if not transparent */
    int                 disposal;       /* gif disposal method */
    int                 delay;          /* in milliseconds */
    II_PALETTE *        local_palette;  /* local color table (can be NULL) */
    HBITMAP             hbmPart;        /* 8bpp part image */
    HBITMAP             hbmScreen;      /* must be a 32bpp or NULL */
    void *              p_user;         /* user data pointer */
    size_t              i_user;         /* user data integer */
} II_ANIGIF_FRAME;

/* animated gif */
typedef struct II_ANIGIF
{
    II_FLAGS            flags;          /* II_FLAG_* */
    int                 width, height;  /* pixel size */
    II_PALETTE *        global_palette; /* global color table */
    int                 iBackground;    /* background color index */
    int                 num_frames;     /* the number of frames */
    II_ANIGIF_FRAME *   frames;         /* malloc'ed */
    int                 loop_count;     /* loop count (0 for infinity) */
    void *              p_user;         /* user data pointer */
    size_t              i_user;         /* user data integer */
} II_ANIGIF;

typedef struct II_APNG_FRAME
{
    HBITMAP         hbmScreen;      /* must be 32bpp or NULL */
    HBITMAP         hbmPart;        /* must be 32bpp */
    uint32_t        x_offset;       /* horizontal position */
    uint32_t        y_offset;       /* vertical position */
    uint32_t        width, height;  /* pixel size */
    uint32_t        delay;          /* in milliseconds */
    uint8_t         dispose_op;     /* PNG_DISPOSE_OP_* */
    uint8_t         blend_op;       /* PNG_BLEND_OP_* */
    void *          p_user;         /* user data pointer */
    size_t          i_user;         /* user data integer */
} II_APNG_FRAME;

typedef struct II_APNG
{
    II_APNG_FRAME * frames;         /* malloc'ed */
    HBITMAP         hbmDefault;     /* must be 32bpp or NULL */
    uint32_t        width, height;  /* pixel size */
    uint32_t        num_frames;     /* number of frames */
    uint32_t        num_plays;      /* number of plays */
    void *          p_user;         /* user data pointer */
    size_t          i_user;         /* user data integer */
    II_FLAGS        flags;          /* II_FLAG_* */
    float           dpi;            /* can be zero */
} II_APNG;

/*****************************************************************************/
/* bitmap image manipulation */

/*
 * creation and destroying
 */
IMAIO_API HBITMAP IIAPI
ii_create(int width, int height,
          int bpp ii_optional_(24), const II_PALETTE *table ii_optional);

IMAIO_API HBITMAP IIAPI
ii_create_8bpp_solid(int cx, int cy, const II_PALETTE *table, int iColorIndex);

IMAIO_API HBITMAP IIAPI ii_create_8bpp_grayscale(int cx, int cy);

IMAIO_API HBITMAP IIAPI ii_create_24bpp(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_24bpp_solid(int cx, int cy, const II_COLOR8 *color);
IMAIO_API HBITMAP IIAPI ii_create_24bpp_black(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_24bpp_white(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_24bpp_checker(int cx, int cy);

IMAIO_API HBITMAP IIAPI ii_create_32bpp(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_32bpp_trans(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_32bpp_solid(int cx, int cy, const II_COLOR8 *color);
IMAIO_API HBITMAP IIAPI ii_create_32bpp_black_opaque(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_32bpp_white(int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_create_32bpp_checker(int cx, int cy);

IMAIO_API HBITMAP IIAPI ii_clone(HBITMAP hbm);
IMAIO_API void IIAPI    ii_destroy(HBITMAP hbm);

/*
 * stretching size
 */
IMAIO_API HBITMAP IIAPI ii_stretched(HBITMAP hbm, int cxNew, int cyNew);
IMAIO_API HBITMAP IIAPI ii_stretched_24bpp(HBITMAP hbm, int cxNew, int cyNew);
IMAIO_API HBITMAP IIAPI ii_stretched_32bpp(HBITMAP hbm, int cxNew, int cyNew);

/*
 * conversion
 */
IMAIO_API HBITMAP IIAPI ii_8bpp(HBITMAP hbm, int num_colors ii_optional_(256));
IMAIO_API HBITMAP IIAPI ii_trans_8bpp(HBITMAP hbm, int *pi_trans);
IMAIO_API HBITMAP IIAPI ii_trans_8bpp_from_32bpp(HBITMAP hbm32bpp, int *pi_trans);
IMAIO_API HBITMAP IIAPI ii_24bpp(HBITMAP hbm);
IMAIO_API HBITMAP IIAPI ii_32bpp(HBITMAP hbm);
IMAIO_API HBITMAP IIAPI ii_32bpp_from_trans_8bpp(HBITMAP hbm8bpp, const int *pi_trans);
IMAIO_API HBITMAP IIAPI ii_24bpp_or_32bpp(HBITMAP hbm);
IMAIO_API HBITMAP IIAPI ii_grayscale_8bpp(HBITMAP hbm);
IMAIO_API HBITMAP IIAPI ii_grayscale_32bpp(HBITMAP hbm);

/*
 * rotation or flipping
 */
IMAIO_API HBITMAP IIAPI ii_rotated_32bpp(HBITMAP hbmSrc, double angle, bool fGrow);
IMAIO_API HBITMAP IIAPI ii_flipped_horizontal(HBITMAP hbmSrc);
IMAIO_API HBITMAP IIAPI ii_flipped_vertical(HBITMAP hbmSrc);

/*
 * getting info
 */
IMAIO_API bool          IIAPI ii_get_info(HBITMAP hbm, BITMAP *pbm);
IMAIO_API uint32_t      IIAPI ii_get_bpp(HBITMAP hbm);
IMAIO_API uint8_t *     IIAPI ii_get_pixels(HBITMAP hbm);
IMAIO_API int           IIAPI ii_get_width(HBITMAP hbm);
IMAIO_API int           IIAPI ii_get_height(HBITMAP hbm);
IMAIO_API II_PALETTE *  IIAPI ii_get_palette(HBITMAP hbm);
IMAIO_API bool          IIAPI ii_is_opaque(HBITMAP hbm);

/*
 * drawing
 */

/* for AlphaBlend API and/or layered windows */
IMAIO_API VOID IIAPI ii_premultiply(HBITMAP hbm32bpp);

IMAIO_API void IIAPI
ii_make_opaque(HBITMAP hbm32bpp, int x, int y, int cx, int cy);

IMAIO_API
void IIAPI ii_draw(
    HDC hdc, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
IMAIO_API
void IIAPI ii_draw_center(
    HDC hdc, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));

IMAIO_API
void IIAPI ii_put(
    HBITMAP hbm, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
IMAIO_API
void IIAPI ii_put_center(
    HBITMAP hbm, int x, int y,
    HBITMAP hbmSrc, int xSrc, int ySrc, int cxSrc, int cySrc,
    const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));

IMAIO_API void IIAPI
    ii_stamp(HBITMAP hbm, int x, int y, HBITMAP hbmSrc,
        const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
IMAIO_API void IIAPI
    ii_stamp_center(
        HBITMAP hbm, int x, int y, HBITMAP hbmSrc,
        const int *pi_trans ii_optional, BYTE bSCA ii_optional_(255));
/*
 * trimming
 */
IMAIO_API HBITMAP IIAPI ii_subimage(HBITMAP hbm, int x, int y, int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_subimage_24bpp(HBITMAP hbm, int x, int y, int cx, int cy);
IMAIO_API HBITMAP IIAPI ii_subimage_32bpp(HBITMAP hbm, int x, int y, int cx, int cy);

IMAIO_API HBITMAP IIAPI
ii_subimage_8bpp_minus(
    HBITMAP hbm, int x, int y, int cx, int cy, int bpp ii_optional_(8));

/*****************************************************************************/
/* screenshot */

IMAIO_API HBITMAP IIAPI ii_screenshot(
    HWND window ii_optional, const RECT *position ii_optional);

/*****************************************************************************/
/* colors */

IMAIO_API II_COLOR8 IIAPI ii_color_trim(II_COLOR8 color);
IMAIO_API int32_t IIAPI
    ii_color_distance(const II_COLOR8 *c1, const II_COLOR8 *c2);

IMAIO_API int32_t IIAPI
ii_color_distance_alpha(const II_COLOR8 *c1, const II_COLOR8 *c2);

IMAIO_API uint8_t IIAPI ii_bound(int value);

IMAIO_API II_PALETTE * IIAPI
ii_palette_create(int num_colors, const II_COLOR8 *colors ii_optional);

IMAIO_API II_PALETTE * IIAPI ii_palette_fixed(bool web_safe ii_optional);
IMAIO_API II_PALETTE * IIAPI ii_palette_optimized(HBITMAP hbm, int num_colors);

IMAIO_API void IIAPI
ii_palette_shrink(II_PALETTE *table, const int *pi_trans ii_optional);

IMAIO_API II_PALETTE * IIAPI
ii_palette_for_anigif(
    II_ANIGIF *anigif, int32_t num_colors);

IMAIO_API II_PALETTE * IIAPI
ii_palette_for_pixels(int num_pixels, const uint32_t *pixels, int num_colors);

IMAIO_API void IIAPI ii_palette_destroy(II_PALETTE *palette);

IMAIO_API int IIAPI
ii_color_nearest_index(const II_PALETTE *table, const II_COLOR8 *pcolor);

IMAIO_API HBITMAP IIAPI ii_reduce_colors(
    HBITMAP hbm, const II_PALETTE *table, const int *pi_trans ii_optional);

IMAIO_API void IIAPI ii_erase_semitrans(HBITMAP hbm);

/*****************************************************************************/
/* alpha channel */

IMAIO_API HBITMAP IIAPI
    ii_alpha_channel_from_32bpp(HBITMAP hbm32bpp);
IMAIO_API HBITMAP IIAPI
    ii_add_alpha_channel(HBITMAP hbmAlpha, HBITMAP hbm);
IMAIO_API void IIAPI
    ii_store_alpha_channel(HBITMAP hbmAlpha, HBITMAP hbm32bpp);

/*****************************************************************************/
/* Windows bitmap */

IMAIO_API HBITMAP IIAPI ii_bmp_load_a(const char *filename, float *dpi ii_optional);
IMAIO_API HBITMAP IIAPI ii_bmp_load_w(const wchar_t *filename, float *dpi ii_optional);
IMAIO_API HBITMAP IIAPI ii_bmp_load_common(HANDLE hFile, HBITMAP hbm, float *dpi);

IMAIO_API bool IIAPI
ii_bmp_save_a(const char *filename, HBITMAP hbm, float dpi ii_optional);
IMAIO_API bool IIAPI
ii_bmp_save_w(const wchar_t *filename, HBITMAP hbm, float dpi ii_optional);
IMAIO_API bool IIAPI
ii_bmp_save_common(HANDLE hFile, HBITMAP hbm, float dpi);

/* load from resource
 *  ex) HBITMAP hbm = ii_bmp_load_res(hInst, MAKEINTRESOURCE(1));
 *      for resource (1 BITMAP "myfile.bmp")
 */
IMAIO_API HBITMAP IIAPI ii_bmp_load_res_a(HMODULE hInstance, const char *res_name);
IMAIO_API HBITMAP IIAPI ii_bmp_load_res_w(HMODULE hInstance, const wchar_t *res_name);

#ifdef UNICODE
    #define ii_bmp_load ii_bmp_load_w
    #define ii_bmp_save ii_bmp_save_w
    #define ii_bmp_load_res ii_bmp_load_res_w
#else
    #define ii_bmp_load ii_bmp_load_a
    #define ii_bmp_save ii_bmp_save_a
    #define ii_bmp_load_res ii_bmp_load_res_a
#endif

/*****************************************************************************/
/* jpeg */

#ifdef HAVE_JPEG
    #ifndef XMD_H
        #define XMD_H
    #endif
    #define HAVE_BOOLEAN
    typedef char ii_jpeg_boolean;
    #undef boolean
    #define boolean ii_jpeg_boolean
    #include "jpeglib.h"
    #include "jerror.h"
    #undef boolean
    IMAIO_API HBITMAP IIAPI ii_jpg_load_a(const char *filename, float *dpi ii_optional);
    IMAIO_API HBITMAP IIAPI ii_jpg_load_w(const wchar_t *filename, float *dpi ii_optional);

    IMAIO_API bool IIAPI
    ii_jpg_save_a(const char *filename, HBITMAP hbm,
                  int quality ii_optional_(100),
                  bool progression ii_optional,
                  float dpi ii_optional);
    IMAIO_API bool IIAPI
    ii_jpg_save_w(const wchar_t *filename, HBITMAP hbm,
                  int quality ii_optional_(100),
                  bool progression ii_optional,
                  float dpi ii_optional);

    #ifdef UNICODE
        #define ii_jpg_load ii_jpg_load_w
        #define ii_jpg_save ii_jpg_save_w
    #else
        #define ii_jpg_load ii_jpg_load_a
        #define ii_jpg_save ii_jpg_save_a
    #endif

    IMAIO_API HBITMAP IIAPI ii_jpg_load_common(FILE *fp, float *dpi);

    IMAIO_API bool IIAPI
    ii_jpg_save_common(FILE *fp, HBITMAP hbm,
                       int quality, bool progression, float dpi);
#endif

/*****************************************************************************/
/* gif */

#ifdef HAVE_GIF
    #include "gif_lib.h"
    IMAIO_API HBITMAP IIAPI ii_gif_load_8bpp_a(const char *filename, int *pi_trans ii_optional);
    IMAIO_API HBITMAP IIAPI ii_gif_load_8bpp_w(const wchar_t *filename, int *pi_trans ii_optional);
    IMAIO_API HBITMAP IIAPI ii_gif_load_32bpp_a(const char *filename);
    IMAIO_API HBITMAP IIAPI ii_gif_load_32bpp_w(const wchar_t *filename);

    IMAIO_API bool IIAPI
    ii_gif_save_a(const char *filename, HBITMAP hbm8bpp,
                  const int *pi_trans ii_optional);
    IMAIO_API bool IIAPI
    ii_gif_save_w(const wchar_t *filename, HBITMAP hbm8bpp,
                  const int *pi_trans ii_optional);

    /* load from memory */
    IMAIO_API HBITMAP IIAPI
    ii_gif_load_8bpp_mem(const void *pv, uint32_t cb, int *pi_trans ii_optional);

    IMAIO_API HBITMAP IIAPI
    ii_gif_load_32bpp_mem(const void *pv, uint32_t cb);

    /* load from resource
     *  ex) HBITMAP hbm = ii_gif_load_8bpp_res(hInst, MAKEINTRESOURCE(1), NULL);
     *      for resource (1 GIF "myfile.gif")
     */
    IMAIO_API HBITMAP IIAPI
    ii_gif_load_8bpp_res_a(HMODULE hInstance, const char *res_name,
                           int *pi_trans ii_optional);
    IMAIO_API HBITMAP IIAPI
    ii_gif_load_8bpp_res_w(HMODULE hInstance, const wchar_t *res_name,
                           int *pi_trans ii_optional);

    IMAIO_API HBITMAP IIAPI
    ii_gif_load_32bpp_res_a(HMODULE hInstance, const char *res_name);

    IMAIO_API HBITMAP IIAPI
    ii_gif_load_32bpp_res_w(HMODULE hInstance, const wchar_t *res_name);

    #ifdef UNICODE
        #define ii_gif_load_8bpp ii_gif_load_8bpp_w
        #define ii_gif_load_32bpp ii_gif_load_32bpp_w
        #define ii_gif_save ii_gif_save_w
        #define ii_gif_load_8bpp_res ii_gif_load_8bpp_res_w
        #define ii_gif_load_32bpp_res ii_gif_load_32bpp_res_w
    #else
        #define ii_gif_load_8bpp ii_gif_load_8bpp_a
        #define ii_gif_load_32bpp ii_gif_load_32bpp_a
        #define ii_gif_save ii_gif_save_a
        #define ii_gif_load_8bpp_res ii_gif_load_8bpp_res_a
        #define ii_gif_load_32bpp_res ii_gif_load_32bpp_res_a
    #endif

    IMAIO_API void IIAPI
    ii_gif_uninterlace(GifByteType *bits, int width, int height);

    IMAIO_API HBITMAP IIAPI
    ii_gif_load_8bpp_common(GifFileType *gif, int *pi_trans ii_optional);

    IMAIO_API bool IIAPI ii_gif_save_common(
        GifFileType *gif, HBITMAP hbm8bpp, const int *pi_trans ii_optional);
#endif

/*****************************************************************************/
/* animated gif */

#ifdef HAVE_GIF
    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_load_a(const char *filename, II_FLAGS flags);
    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_load_w(const wchar_t *filename, II_FLAGS flags);

    IMAIO_API bool IIAPI
    ii_anigif_save_a(const char *filename, II_ANIGIF *anigif);

    IMAIO_API bool IIAPI
    ii_anigif_save_w(const wchar_t *filename, II_ANIGIF *anigif);

    IMAIO_API II_ANIGIF * IIAPI ii_anigif_load_mem(
        const void *pv, uint32_t cb, II_FLAGS flags);

    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_load_res_a(
        HMODULE hInstance, const char *res_name, II_FLAGS flags);

    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_load_res_w(
        HMODULE hInstance, const wchar_t *res_name, II_FLAGS flags);

    IMAIO_API void IIAPI
    ii_anigif_destroy(II_ANIGIF *anigif);

    #ifdef UNICODE
        #define ii_anigif_load ii_anigif_load_w
        #define ii_anigif_save ii_anigif_save_w
        #define ii_anigif_load_res ii_anigif_load_res_w
    #else
        #define ii_anigif_load ii_anigif_load_a
        #define ii_anigif_save ii_anigif_save_a
        #define ii_anigif_load_res ii_anigif_load_res_a
    #endif

    IMAIO_API II_ANIGIF * IIAPI
    ii_anigif_load_common(GifFileType *gif, II_FLAGS flags);
    IMAIO_API bool IIAPI
    ii_anigif_save_common(GifFileType *gif, II_ANIGIF *anigif);
#endif

/*****************************************************************************/
/* png */

#ifdef HAVE_PNG
    #include "png.h"
    #include "zlib.h"

    IMAIO_API HBITMAP IIAPI
    ii_png_load_a(const char *filename, float *dpi ii_optional);

    IMAIO_API HBITMAP IIAPI
    ii_png_load_w(const wchar_t *filename, float *dpi ii_optional);

    IMAIO_API bool IIAPI
    ii_png_save_a(const char *filename, HBITMAP hbm, float dpi ii_optional);

    IMAIO_API bool IIAPI
    ii_png_save_w(const wchar_t *filename, HBITMAP hbm, float dpi ii_optional);

    /* load from memory */
    IMAIO_API HBITMAP IIAPI ii_png_load_mem(const void *pv, uint32_t cb);

    /* load from resource
     *  ex) HBITMAP hbm = ii_png_load_res(hInst, MAKEINTRESOURCE(1));
     *      for resource (1 PNG "myfile.png")
     */
    IMAIO_API HBITMAP IIAPI
    ii_png_load_res_a(HMODULE hInstance, const char *res_name);

    IMAIO_API HBITMAP IIAPI
     ii_png_load_res_w(HMODULE hInstance, const wchar_t *res_name);

    #ifdef UNICODE
        #define ii_png_load ii_png_load_w
        #define ii_png_save ii_png_save_w
        #define ii_png_load_res ii_png_load_res_w
    #else
        #define ii_png_load ii_png_load_a
        #define ii_png_save ii_png_save_a
        #define ii_png_load_res ii_png_load_res_a
    #endif

    IMAIO_API HBITMAP IIAPI ii_png_load_common(FILE *inf, float *dpi);
    IMAIO_API bool IIAPI ii_png_save_common(FILE *outf, HBITMAP hbm, float dpi);
#endif

/*****************************************************************************/
/* animated PNG (APNG) */

#ifdef HAVE_PNG
    #ifdef PNG_APNG_SUPPORTED
        IMAIO_API II_APNG * IIAPI
        ii_apng_load_a(const char *filename,
                       II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

        IMAIO_API II_APNG * IIAPI
        ii_apng_load_w(const wchar_t *filename,
                       II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

        IMAIO_API II_APNG * IIAPI
        ii_apng_load_res_a(HMODULE hInstance, const char *res_name,
                           II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

        IMAIO_API II_APNG * IIAPI
        ii_apng_load_res_w(HMODULE hInstance, const wchar_t *res_name,
                           II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

        IMAIO_API II_APNG * IIAPI
        ii_apng_load_mem(const void *pv, uint32_t cb,
                         II_FLAGS flags ii_optional_(II_FLAG_USE_SCREEN));

        IMAIO_API bool IIAPI ii_apng_save_a(const char *filename, II_APNG *apng);
        IMAIO_API bool IIAPI ii_apng_save_w(const wchar_t *filename, II_APNG *apng);

        IMAIO_API void IIAPI ii_apng_destroy(II_APNG *apng);

        IMAIO_API II_APNG * IIAPI ii_apng_load_fp(FILE *fp, II_FLAGS flags);
        IMAIO_API bool IIAPI ii_apng_save_fp(FILE *fp, II_APNG *apng);

        #ifdef UNICODE
            #define ii_apng_load ii_apng_load_w
            #define ii_apng_load_res ii_apng_load_res_w
            #define ii_apng_save ii_apng_save_w
        #else
            #define ii_apng_load ii_apng_load_a
            #define ii_apng_load_res ii_apng_load_res_a
            #define ii_apng_save ii_apng_save_a
        #endif

        IMAIO_API HBITMAP IIAPI
        ii_image_from_32bpp_rows(int width, int height, png_bytepp rows);

        IMAIO_API void IIAPI
        ii_32bpp_rows_from_image(png_bytepp rows, HBITMAP hbmImage);

        IMAIO_API II_APNG * IIAPI
        ii_apng_from_anigif(II_ANIGIF *anigif);

        IMAIO_API II_ANIGIF * IIAPI
        ii_anigif_from_apng(II_APNG *apng, bool kill_semitrans ii_optional_(true));
    #endif  /* def PNG_APNG_SUPPORTED */
#endif

/*****************************************************************************/
/* tiff */

#ifdef HAVE_TIFF
    #include "tiffio.h"

    IMAIO_API HBITMAP IIAPI
    ii_tif_load_a(const char *filename, float *dpi ii_optional);

    IMAIO_API HBITMAP IIAPI
    ii_tif_load_w(const wchar_t *filename, float *dpi ii_optional);

    IMAIO_API bool IIAPI
    ii_tif_save_a(const char *filename, HBITMAP hbm, float dpi ii_optional);

    IMAIO_API bool IIAPI
    ii_tif_save_w(const wchar_t *filename, HBITMAP hbm, float dpi ii_optional);

    #ifdef UNICODE
        #define ii_tif_load ii_tif_load_w
        #define ii_tif_save ii_tif_save_w
    #else
        #define ii_tif_load ii_tif_load_a
        #define ii_tif_save ii_tif_save_a
    #endif

    IMAIO_API HBITMAP IIAPI ii_tif_load_common(TIFF *tif, float *dpi);
    IMAIO_API bool IIAPI ii_tif_save_common(TIFF *tif, HBITMAP hbm, float dpi);
#endif

/*****************************************************************************/
/* image types */

typedef enum II_IMAGE_TYPE
{
    II_IMAGE_TYPE_INVALID,  /* not supported */
    II_IMAGE_TYPE_JPG,      /* JPEG */
    II_IMAGE_TYPE_GIF,      /* GIF */
    II_IMAGE_TYPE_ANIGIF,   /* animated GIF */
    II_IMAGE_TYPE_PNG,      /* PNG */
    II_IMAGE_TYPE_APNG,     /* animated PNG */
    II_IMAGE_TYPE_TIF,      /* TIFF */
    II_IMAGE_TYPE_BMP       /* BMP */
} II_IMAGE_TYPE;

/* image type from path name or dotext */
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_path_name_a(const char * filename);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_path_name_w(const wchar_t *filename);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_dotext_a(const char * dot_ext);
IMAIO_API II_IMAGE_TYPE IIAPI ii_image_type_from_dotext_w(const wchar_t *dot_ext);

/* wildcards from image type */
IMAIO_API const char *      IIAPI ii_wildcards_from_image_type_a(II_IMAGE_TYPE type);
IMAIO_API const wchar_t *   IIAPI ii_wildcards_from_image_type_w(II_IMAGE_TYPE type);

/* file title and dotext */
IMAIO_API const char *      IIAPI ii_find_file_title_a(const char * path);
IMAIO_API const wchar_t *   IIAPI ii_find_file_title_w(const wchar_t *path);
IMAIO_API const char *      IIAPI ii_find_dotext_a(const char * path);
IMAIO_API const wchar_t *   IIAPI ii_find_dotext_w(const wchar_t *path);

/* MIME */
IMAIO_API const char *      IIAPI ii_mime_from_path_name_a(const char * filename);
IMAIO_API const wchar_t *   IIAPI ii_mime_from_path_name_w(const wchar_t *filename);
IMAIO_API const char *      IIAPI ii_mime_from_dotext_a(const char * dot_ext);
IMAIO_API const wchar_t *   IIAPI ii_mime_from_dotext_w(const wchar_t *dot_ext);
IMAIO_API const char *      IIAPI ii_dotext_from_mime_a(const char * mime);
IMAIO_API const wchar_t *   IIAPI ii_dotext_from_mime_w(const wchar_t *mime);
IMAIO_API const char *      IIAPI ii_mime_from_image_type_a(II_IMAGE_TYPE type);
IMAIO_API const wchar_t *   IIAPI ii_mime_from_image_type_w(II_IMAGE_TYPE type);
IMAIO_API II_IMAGE_TYPE IIAPI   ii_image_type_from_mime_a(const char * mime);
IMAIO_API II_IMAGE_TYPE IIAPI   ii_image_type_from_mime_w(const wchar_t *mime);

/* make a filter string for "Open" / "Save As" dialog */
IMAIO_API char *        IIAPI ii_make_filter_a(char *     pszFilter);
IMAIO_API wchar_t *     IIAPI ii_make_filter_w(wchar_t *  pszFilter);

#ifdef UNICODE
    #define ii_image_type_from_path_name ii_image_type_from_path_name_w
    #define ii_image_type_from_dotext ii_image_type_from_dotext_w
    #define ii_wildcards_from_image_type ii_wildcards_from_image_type_w
    #define ii_find_file_title ii_find_file_title_w
    #define ii_find_dotext ii_find_dotext_w
    #define ii_mime_from_path_name ii_mime_from_path_name_w
    #define ii_mime_from_dotext ii_mime_from_dotext_w
    #define ii_dotext_from_mime ii_dotext_from_mime_w
    #define ii_mime_from_image_type ii_mime_from_image_type_w
    #define ii_image_type_from_mime ii_image_type_from_mime_w
    #define ii_make_filter ii_make_filter_w
#else
    #define ii_image_type_from_path_name ii_image_type_from_path_name_a
    #define ii_image_type_from_dotext ii_image_type_from_dotext_a
    #define ii_wildcards_from_image_type ii_wildcards_from_image_type_a
    #define ii_find_file_title ii_find_file_title_a
    #define ii_find_dotext ii_find_dotext_a
    #define ii_mime_from_path_name ii_mime_from_path_name_a
    #define ii_mime_from_dotext ii_mime_from_dotext_a
    #define ii_dotext_from_mime ii_dotext_from_mime_a
    #define ii_mime_from_image_type ii_mime_from_image_type_a
    #define ii_image_type_from_mime ii_image_type_from_mime_a
    #define ii_make_filter ii_make_filter_a
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/*****************************************************************************/

#endif  /* ndef KATAHIROMZ_IMAIO_H_ */
