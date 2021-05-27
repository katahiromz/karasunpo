//////////////////////////////////////////////////////////////////////////////
// katahiromz_pdfium.h --- PDFium wrapper for Windows
// Copyright (C) 2015-2021 Katayama Hirofumi MZ. All Rights Reserved.
// See License.txt about licensing.
//////////////////////////////////////////////////////////////////////////////

#ifndef KATAHIROMZ_PDFIUM_H
#define KATAHIROMZ_PDFIUM_H

#include "fpdfview.h"

struct katahiromz_pdfium {
    HINSTANCE m_hInstance;

    // pdfium functions:
    typedef void (*FN_FPDF_InitLibrary)(VOID);
    typedef void (*FN_FPDF_InitLibraryWithConfig)(const FPDF_LIBRARY_CONFIG* config);
    typedef void (*FN_FPDF_DestroyLibrary)(VOID);
    typedef FPDF_DOCUMENT (*FN_FPDF_LoadDocument)(FPDF_STRING, FPDF_BYTESTRING);
    typedef FPDF_PAGE (*FN_FPDF_LoadPage)(FPDF_DOCUMENT, int);
    typedef double (*FN_FPDF_GetPageWidth)(FPDF_PAGE);
    typedef double (*FN_FPDF_GetPageHeight)(FPDF_PAGE);
    typedef void (*FN_FPDF_RenderPage)(HDC, FPDF_PAGE, int, int, int, int, int, int);
    typedef void (*FN_FPDF_ClosePage)(FPDF_PAGE);
    typedef void (*FN_FPDF_CloseDocument)(FPDF_DOCUMENT);
    typedef int (*FN_FPDF_GetPageCount)(FPDF_DOCUMENT);
    typedef unsigned long (*FN_FPDF_GetLastError)(VOID);
    FN_FPDF_InitLibrary         FPDF_InitLibrary;
    FN_FPDF_InitLibraryWithConfig FPDF_InitLibraryWithConfig;
    FN_FPDF_DestroyLibrary      FPDF_DestroyLibrary;
    FN_FPDF_LoadDocument        FPDF_LoadDocument;
    FN_FPDF_LoadPage            FPDF_LoadPage;
    FN_FPDF_GetPageWidth        FPDF_GetPageWidth;
    FN_FPDF_GetPageHeight       FPDF_GetPageHeight;
    FN_FPDF_RenderPage          FPDF_RenderPage;
    FN_FPDF_ClosePage           FPDF_ClosePage;
    FN_FPDF_CloseDocument       FPDF_CloseDocument;
    FN_FPDF_GetPageCount        FPDF_GetPageCount;
    FN_FPDF_GetLastError        FPDF_GetLastError;

    katahiromz_pdfium(bool do_load = false) : m_hInstance(NULL) {
        #ifndef NDEBUG
            FPDF_InitLibrary = NULL;
            FPDF_InitLibraryWithConfig = NULL;
            FPDF_DestroyLibrary = NULL;
            FPDF_LoadDocument = NULL;
            FPDF_LoadPage = NULL;
            FPDF_GetPageWidth = NULL;
            FPDF_GetPageHeight = NULL;
            FPDF_RenderPage = NULL;
            FPDF_ClosePage = NULL;
            FPDF_CloseDocument = NULL;
            FPDF_GetPageCount = NULL;
            FPDF_GetLastError = NULL;
        #endif
        if (do_load) {
            load();
        }
    }

    bool is_loaded() const {
        return m_hInstance != NULL;
    }

    ~katahiromz_pdfium() {
        release();
    }

    template <typename T_FN>
    T_FN get_proc(LPCSTR name) {
        return reinterpret_cast<T_FN>(::GetProcAddress(m_hInstance, name));
    }

    bool load(const TCHAR *dll_name = NULL) {
        release();
        if (dll_name)
        {
            m_hInstance = ::LoadLibrary(dll_name);
        }
        else
        {
            #ifdef _WIN64
                m_hInstance = ::LoadLibrary(TEXT("x64\\pdfium.dll"));
                if (m_hInstance == NULL) {
                    m_hInstance = ::LoadLibrary(TEXT("x64\\pdfium64.dll"));
                    if (m_hInstance == NULL) {
                        m_hInstance = ::LoadLibrary(TEXT("pdfium64.dll"));
                        if (m_hInstance == NULL) {
                            m_hInstance = ::LoadLibrary(TEXT("pdfium.dll"));
                        }
                    }
                }
            #else
                m_hInstance = ::LoadLibrary(TEXT("x86\\pdfium.dll"));
                if (m_hInstance == NULL) {
                    m_hInstance = ::LoadLibrary(TEXT("pdfium.dll"));
                }
            #endif
        }

        if (m_hInstance) {
            FPDF_InitLibrary = get_proc<FN_FPDF_InitLibrary>("FPDF_InitLibrary");
            FPDF_InitLibraryWithConfig = get_proc<FN_FPDF_InitLibraryWithConfig>("FPDF_InitLibraryWithConfig");
            FPDF_DestroyLibrary = get_proc<FN_FPDF_DestroyLibrary>("FPDF_DestroyLibrary");
            FPDF_LoadDocument = get_proc<FN_FPDF_LoadDocument>("FPDF_LoadDocument");
            FPDF_LoadPage = get_proc<FN_FPDF_LoadPage>("FPDF_LoadPage");
            FPDF_GetPageWidth = get_proc<FN_FPDF_GetPageWidth>("FPDF_GetPageWidth");
            FPDF_GetPageHeight = get_proc<FN_FPDF_GetPageHeight>("FPDF_GetPageHeight");
            FPDF_RenderPage = get_proc<FN_FPDF_RenderPage>("FPDF_RenderPage");
            FPDF_ClosePage = get_proc<FN_FPDF_ClosePage>("FPDF_ClosePage");
            FPDF_CloseDocument = get_proc<FN_FPDF_CloseDocument>("FPDF_CloseDocument");
            FPDF_GetPageCount = get_proc<FN_FPDF_GetPageCount>("FPDF_GetPageCount");
            FPDF_GetLastError = get_proc<FN_FPDF_GetLastError>("FPDF_GetLastError");
            if (FPDF_DestroyLibrary &&
                FPDF_LoadDocument &&
                FPDF_LoadPage &&
                FPDF_GetPageWidth &&
                FPDF_GetPageHeight &&
                FPDF_RenderPage &&
                FPDF_ClosePage &&
                FPDF_CloseDocument &&
                FPDF_GetPageCount &&
                FPDF_GetLastError)
            {
                if (FPDF_InitLibraryWithConfig)
                {
                    FPDF_LIBRARY_CONFIG config = { 2 };
                    FPDF_InitLibraryWithConfig(&config);
                    if (FPDF_GetLastError() == 0)
                        return true;
                }
                else if (FPDF_InitLibrary)
                {
                    FPDF_InitLibrary();
                    if (FPDF_GetLastError() == 0)
                        return true;
                }
            }
            release();
        }
        return false;
    }

    void release() {
        if (m_hInstance) {
            FPDF_DestroyLibrary();
            ::FreeLibrary(m_hInstance);
            m_hInstance = NULL;
        }
    }
}; // struct katahiromz_pdfium

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef KATAHIROMZ_PDFIUM_H
