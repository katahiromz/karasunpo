//////////////////////////////////////////////////////////////////////////////
// karasunpo.cpp --- karasunpo
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
// See License.txt about licensing.
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef M_PI
    #define M_PI  3.14159265358979323846
#endif

// maximum length of resource string plus one
static const INT    s_nMaxLoadString = 512;

// the class name of the main window
static const TCHAR  s_szCompanyName[] = TEXT(COMPANYNAME);

// the class name of the main window
static const TCHAR  s_szSoftware[] = TEXT("Software");

// the class name of the main window
static const TCHAR  s_szClassName[] = TEXT(PROGNAME);

// the class name of the real client window
static const TCHAR  s_szRealClientWndClassName[] =
    TEXT("karasunpo real client window");

// the registry value names
static const TCHAR  s_szGotStarted[] = TEXT("GotStarted");
static const TCHAR  s_szWindowWidth[] = TEXT("WindowWidth");
static const TCHAR  s_szWindowHeight[] = TEXT("WindowHeight");

// the registry key name of recent file list
static const TCHAR  s_szRecentFileList[] = TEXT("Recent File List");
// the registry value name format of recent files
static const TCHAR  s_szFileFormat[] = TEXT("File%u");
// the maximum number of recent files
#define MAX_RECENT      8

static const TCHAR s_szSpaces[] = TEXT(" \t\n\r\f\v");

// the segment color
enum SegColor {
    SC_AUTO,
    SC_RED,
    SC_GREEN,
    SC_BLUE
};

struct EPoint {
    DOUBLE x;
    DOUBLE y;
};

struct ESize {
    DOUBLE cx;
    DOUBLE cy;
};

// For message hook
static HHOOK s_hMsgBoxHook = NULL;

VOID WINAPI CenterDialog(HWND hwnd)
{
    BOOL bChild = !!(GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD);

    HWND hwndOwner;
    if (bChild)
        hwndOwner = GetParent(hwnd);
    else
        hwndOwner = GetWindow(hwnd, GW_OWNER);

    RECT rc, rcOwner;
    if (hwndOwner != NULL)
        GetWindowRect(hwndOwner, &rcOwner);
    else
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcOwner, 0);

    GetWindowRect(hwnd, &rc);

    POINT pt;
    pt.x = rcOwner.left +
        ((rcOwner.right - rcOwner.left) - (rc.right - rc.left)) / 2;
    pt.y = rcOwner.top +
        ((rcOwner.bottom - rcOwner.top) - (rc.bottom - rc.top)) / 2;

    if (bChild && hwndOwner != NULL)
        ScreenToClient(hwndOwner, &pt);

    SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    SendMessage(hwnd, DM_REPOSITION, 0, 0);
} // CenterDialog

LRESULT CALLBACK
MsgBoxCbtProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_ACTIVATE)
    {
        HWND hwnd = (HWND)wParam;

        TCHAR szClassName[MAX_PATH];
        GetClassName(hwnd, szClassName, MAX_PATH);
        if (lstrcmpi(szClassName, TEXT("#32770")) == 0)
        {
            CenterDialog(hwnd);

            if (s_hMsgBoxHook != NULL && UnhookWindowsHookEx(s_hMsgBoxHook))
                s_hMsgBoxHook = NULL;
        }
    }
    // allow the operation
    return 0;
} // MsgBoxCbtProc

INT WINAPI
CenterMessageBox(HWND hwnd, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType)
{
    if (s_hMsgBoxHook != NULL && UnhookWindowsHookEx(s_hMsgBoxHook))
        s_hMsgBoxHook = NULL;

    DWORD dwThreadID = GetCurrentThreadId();
    s_hMsgBoxHook = SetWindowsHookEx(WH_CBT, MsgBoxCbtProc, NULL, dwThreadID);

    INT nID = MessageBox(hwnd, pszText, pszCaption, uType);

    if (s_hMsgBoxHook != NULL && UnhookWindowsHookEx(s_hMsgBoxHook))
        s_hMsgBoxHook = NULL;

    return nID;
} // CenterMessageBox

// the application
struct WinApp {
    INT         m_argc;         // number of command line parameters
    TCHAR **    m_targv;        // command line parameters

    HINSTANCE   m_hInst;        // the instance handle
    HWND        m_hWnd;         // the main window handle
    HICON       m_hIcon;        // the icon handle
    HACCEL      m_hAccel;       // the accelerator handle

    // real client window
    HWND        m_hRealClientWnd;
    // status bar
    HWND        m_hStatusBar;
    // task index
    INT         m_nTaskIndex;
    // task dialog window
    HWND        m_hTaskDialogs[NUM_TASKS];
    // initial size of dialog window
    SIZE        m_sizTaskDlg;
    // file path
    TCHAR       m_szFileName[MAX_PATH];
    // bitmap image
    HBITMAP     m_hbmImage;
    // client image
    HBITMAP     m_hbmClient;
    // zoom rate
    DOUBLE      m_eZoomPercent;
    // image DPI
    DOUBLE      m_eImageDPI;
    // display DPI
    EPoint      m_display_dpi;
    // coordinates for mouse input
    POINT       m_pt0;
    POINT       m_pt1;
    EPoint      m_ept0;
    EPoint      m_ept1;
    // x and y coordinates of the segment
    EPoint      m_eptSegment0;
    EPoint      m_eptSegment1;
    // Is there the segment?
    bool        m_bHasSegment;
    // x and y coordinates of the standard segment
    EPoint      m_eptStd0;
    EPoint      m_eptStd1;
    // Is there the standard segment?
    bool        m_bHasStd;
    // the standard length
    DOUBLE      m_eStdLengthInTheUnits;
    // the standard pixels
    DOUBLE      m_eStdImagePixels;
    // the standard unit
    TCHAR       m_szStdUnit[64];
    enum {
        MODE_NOTHING,
        MODE_SEGMENT,
        MODE_PT0,
        MODE_PT1
    }           m_mode;
    bool        m_bGotStarted;
    INT         m_nWindowWidth;
    INT         m_nWindowHeight;
    enum FitMode {
        FIT_NONE,
        FIT_WHOLE,
        FIT_HORIZONTAL,
        FIT_VERTICAL
    }           m_fit_mode;
    bool        m_bDoesDrawCircle;
    enum MeasureType {
        MEASURE_LENGTH,
        MEASURE_INCLINATION,
        MEASURE_ANGLE
    }           m_measure_type;
    bool        m_in_radian;
    bool        m_two_button_moved;
    katahiromz_pdfium m_pdfium;
    bool        m_is_pdf;
    INT         m_nPageIndex;
    INT         m_nPageCount;
    SegColor    m_seg_color;

    std::deque<std::wstring>    m_recent_files;
    HCURSOR     m_ahCursors[5];

    // constructor
    WinApp(HINSTANCE hInst, int argc, TCHAR **targv) {
        m_argc = argc;
        m_targv = targv;
        m_hInst = hInst;
        m_hWnd = NULL;
        m_hIcon = NULL;
        m_hAccel = NULL;
        m_hRealClientWnd = NULL;
        m_hStatusBar = NULL;
        m_nTaskIndex = DLGINDEX_GETSTARTED;
        ZeroMemory(m_hTaskDialogs, sizeof(m_hTaskDialogs));
        ZeroMemory(&m_sizTaskDlg, sizeof(m_sizTaskDlg));
        m_szFileName[0] = 0;
        m_hbmImage = NULL;
        m_hbmClient = NULL;
        m_eZoomPercent = 100.0;
        m_bHasSegment = false;
        m_bHasStd = false;
        m_ept0.x = 0.0;
        m_ept0.y = 0.0;
        m_ept1.x = 0.0;
        m_ept1.y = 0.0;
        m_eImageDPI = 0.0;
        m_display_dpi.x = 0;
        m_display_dpi.y = 0;
        m_eStdLengthInTheUnits = 0.0;
        m_mode = MODE_NOTHING;
        m_bGotStarted = false;
        m_nWindowWidth = CW_USEDEFAULT;
        m_nWindowHeight = CW_USEDEFAULT;
        m_szStdUnit[0] = 0;
        m_fit_mode = FIT_NONE;
        m_bDoesDrawCircle = false;
        m_measure_type = MEASURE_LENGTH;
        m_in_radian = false;
        m_is_pdf = false;
        m_nPageIndex = 0;
        m_nPageCount = 0;
        m_seg_color = SC_RED;
        for (int i = 0; i < 5; ++i) {
            m_ahCursors[i] = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_PAN + i));
        }
    } // WinApp

    ~WinApp() {
        for (int i = 0; i < 5; ++i) {
            ::DestroyCursor(m_ahCursors[i]);
            m_ahCursors[i] = NULL;
        }
    }

    // NOTE: loadString is not thread-safe!
    LPTSTR loadString(UINT nID) {
        static TCHAR s_sz[s_nMaxLoadString];
        s_sz[0] = 0;
        ::LoadString(m_hInst, nID, s_sz, s_nMaxLoadString);
        return s_sz;
    }

    // NOTE: loadString2 is not thread-safe!
    LPTSTR loadString2(UINT nID) {
        static TCHAR s_sz[s_nMaxLoadString];
        s_sz[0] = 0;
        ::LoadString(m_hInst, nID, s_sz, s_nMaxLoadString);
        return s_sz;
    }

    LPTSTR makeFilter(LPTSTR pszFilter) {
        assert(pszFilter != NULL);
        LPTSTR pszSave = pszFilter;
        while (*pszFilter != 0) {
            if (*pszFilter == TEXT('|')) {
                *pszFilter = 0;
                pszFilter++;
            } else {
                pszFilter = ::CharNext(pszFilter);
            }
        }
        return pszSave;
    }

    // register window classes
    bool registerClasses();

    // start up
    bool startup(INT nCmdShow) {
        if (!m_pdfium.load()) {
            CenterMessageBox(NULL, loadString(15),
                NULL, MB_ICONERROR);
            return false;
        }

        // load accessories
        m_hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
        m_hAccel = ::LoadAccelerators(m_hInst, MAKEINTRESOURCE(1));

        loadSettings();

        // create the main window
        ::CreateWindow(s_szClassName, loadString(1), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, 
            m_nWindowWidth, m_nWindowHeight,
            NULL, NULL, m_hInst, this);
        if (m_hWnd == NULL) {
            CenterMessageBox(NULL,
                TEXT("ERROR: CreateWindow"), NULL, MB_ICONERROR);
            return false;
        }

        // show the window
        ::ShowWindow(m_hWnd, nCmdShow);
        ::UpdateWindow(m_hWnd);

        // process command line
        for (INT i = 1; i < m_argc; ++i) {
            loadFile(m_targv[i]);
            break;
        }
        return true;
    } // startup

    INT run() {
        MSG msg;
        // message loop
        while (::GetMessage(&msg, NULL, 0, 0)) {
            if (!::TranslateAccelerator(m_hWnd, m_hAccel, &msg)) {
                bool flag = false;
                for (int i = 0; i < NUM_TASKS; ++i) {
                    if (::IsDialogMessage(m_hTaskDialogs[i], &msg)) {
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }
        }
        return INT(msg.wParam);
    } // run

    HBITMAP loadPdf(LPCSTR pszFileName, INT nPageIndex = 0) {
        HBITMAP hbm = NULL;
        FPDF_DOCUMENT pdf_doc = m_pdfium.FPDF_LoadDocument(pszFileName, NULL);
        if (pdf_doc != NULL) {
            FPDF_PAGE pdf_page = m_pdfium.FPDF_LoadPage(pdf_doc, nPageIndex);
            if (pdf_page != NULL) {
                double page_width = m_pdfium.FPDF_GetPageWidth(pdf_page);
                double page_height = m_pdfium.FPDF_GetPageHeight(pdf_page);
                HDC hDC = ::GetDC(m_hWnd);
                int logpixelsx = ::GetDeviceCaps(hDC, LOGPIXELSX);
                int logpixelsy = ::GetDeviceCaps(hDC, LOGPIXELSY);
                SIZE sizPixels;
                //sizPixels.cx = LONG(page_width * logpixelsx / 72);
                //sizPixels.cy = LONG(page_height * logpixelsy / 72);
                sizPixels.cx = LONG(page_width * logpixelsx / 72 * 2);
                sizPixels.cy = LONG(page_height * logpixelsy / 72 * 2);
                HDC hMemDC = ::CreateCompatibleDC(hDC);
                hbm = ::CreateCompatibleBitmap(hDC, sizPixels.cx, sizPixels.cy);
                if (hbm != NULL) {
                    HGDIOBJ hbmOld = ::SelectObject(hMemDC, hbm);
                    RECT rc = {0, 0, sizPixels.cx, sizPixels.cy};
                    ::FillRect(hMemDC, &rc,
                        reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));
                    m_pdfium.FPDF_RenderPage(hMemDC, pdf_page,
                        0, 0, sizPixels.cx, sizPixels.cy, 0,
                        FPDF_ANNOT | FPDF_PRINTING
                    );
                    ::SelectObject(hMemDC, hbmOld);
                    // success
                    m_nPageIndex = nPageIndex;
                    m_nPageCount = m_pdfium.FPDF_GetPageCount(pdf_doc);
                    m_is_pdf = true;
                }
                ::DeleteDC(hMemDC);
                ::ReleaseDC(m_hWnd, hDC);
                m_pdfium.FPDF_ClosePage(pdf_page);
            }
            m_pdfium.FPDF_CloseDocument(pdf_doc);
        }
        return hbm;
    }

    HBITMAP loadPdf(LPCWSTR pszFileName, INT nPageIndex = 0) {
        CHAR szFileName[MAX_PATH];
        szFileName[0] = 0;
        ::WideCharToMultiByte(CP_ACP, 0, pszFileName, -1,
            szFileName, MAX_PATH, NULL, NULL);
        return loadPdf(szFileName, nPageIndex);
    }

    HBITMAP loadPdf(INT nPageIndex = 0) {
        return loadPdf(m_szFileName, nPageIndex);
    }

    bool loadFile(LPCTSTR pszFileName) {
        HBITMAP hbm;
        LPCTSTR pszDotExt = ii_find_dotext(pszFileName);
        II_IMAGE_TYPE type = ii_image_type_from_dotext(pszDotExt);
        float dpi = 0;
        switch (type) {
        case II_IMAGE_TYPE_JPG:
            hbm = ii_jpg_load(pszFileName, &dpi);
            break;
        case II_IMAGE_TYPE_GIF:
        case II_IMAGE_TYPE_ANIGIF:
            hbm = ii_gif_load_8bpp(pszFileName);
            break;
        case II_IMAGE_TYPE_PNG:
        case II_IMAGE_TYPE_APNG:
            hbm = ii_png_load(pszFileName, &dpi);
            break;
        case II_IMAGE_TYPE_TIF:
            hbm = ii_tif_load(pszFileName, &dpi);
            break;
        case II_IMAGE_TYPE_BMP:
            hbm = ii_bmp_load(pszFileName, &dpi);
            break;
        default:
            if (lstrcmpi(pszDotExt, TEXT(".pdf")) == 0) {
                hbm = loadPdf(pszFileName);
            } else {
                hbm = NULL;
            }
            break;
        }
        if (hbm == NULL) {
            CenterMessageBox(m_hWnd, loadString(5), loadString2(2),
                MB_ICONERROR);
            return false;
        }
        if (lstrcmpi(pszDotExt, TEXT(".pdf")) == 0) {
            m_is_pdf = true;
        } else {
            m_is_pdf = false;
        }
        if (m_hbmImage != NULL) {
            ::DeleteObject(m_hbmImage);
        }
        m_hbmImage = hbm;
        m_eImageDPI = dpi;
        ::lstrcpyn(m_szFileName, pszFileName, MAX_PATH);
        updateScrollInfo(true);
        updateClientImage();
        onOpened();
        return true;
    } // loadFile

    void setTaskIndex(INT nIndex) {
        if (m_hTaskDialogs[m_nTaskIndex] != NULL) {
            ::ShowWindow(m_hTaskDialogs[m_nTaskIndex], SW_HIDE);
            ::PostMessage(m_hTaskDialogs[m_nTaskIndex], WM_COMMAND, ID_DEACTIVATE, 0);
        }
        if (m_hTaskDialogs[nIndex] != NULL) {
            m_nTaskIndex = nIndex;
            ::ShowWindow(m_hTaskDialogs[nIndex], SW_SHOWNOACTIVATE);
            ::SetFocus(m_hTaskDialogs[nIndex]);
            ::PostMessage(m_hTaskDialogs[nIndex], WM_COMMAND, ID_ACTIVATE, 0);
        }
        updateClientImage();
        ::PostMessage(m_hWnd, WM_SIZE, 0, 0);
    }

    void onOpened() {
        ::EnableWindow(::GetDlgItem(m_hTaskDialogs[DLGINDEX_LOADIMAGE], psh2), TRUE);
        ::SetDlgItemText(m_hTaskDialogs[DLGINDEX_LOADIMAGE], stc1, m_szFileName);

        std::deque<std::wstring> recent_files;
        size_t count = m_recent_files.size();
        for (size_t i = 0; i < count; ++i) {
            if (m_recent_files[i] != m_szFileName) {
                recent_files.emplace_back(m_recent_files[i]);
            }
        }
        recent_files.emplace_front(m_szFileName);
        if (recent_files.size() > MAX_RECENT) {
            recent_files.resize(MAX_RECENT);
        }
        m_recent_files = std::move(recent_files);

        ::SendDlgItemMessage(m_hTaskDialogs[DLGINDEX_LOADIMAGE], stc1, WM_SETFONT,
            reinterpret_cast<WPARAM>(::GetStockObject(DEFAULT_GUI_FONT)),
            TRUE
        );
        m_fit_mode = FIT_WHOLE;
        fitWhile();
    }

    // WM_CREATE
    bool onCreate();

    bool onOpen() {
        OPENFILENAME ofn;
        TCHAR szFile[MAX_PATH] = TEXT("");
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = CDSIZEOF_STRUCT(OPENFILENAME, lpTemplateName);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = makeFilter(loadString(4));
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = loadString2(3);
        ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST |
            OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
        ofn.lpstrDefExt = TEXT("png");
        if (::GetOpenFileName(&ofn)) {
            return loadFile(ofn.lpstrFile);
        }
        return false;
    }

    // WM_DROPFILES
    void onDropFiles(HDROP hDrop) {
        TCHAR szPath[MAX_PATH];
        ::DragQueryFile(hDrop, 0, szPath, MAX_PATH);
        loadFile(szPath);
        ::DragFinish(hDrop);
    } // onDropFiles

    // WM_DESTROY
    void onDestroy() {
        saveSettings();
        ::PostQuitMessage(0);
    } // onDestroy

    DOUBLE getZoomedX(DOUBLE e) const {
        return (e * m_eZoomPercent) / 100.0f;
    }

    DOUBLE getZoomedY(DOUBLE e) const {
        return (e * m_eZoomPercent) / 100.0f;
    }

    DOUBLE getUnzoomedX(DOUBLE e) const {
        return (e * 100.0f) / m_eZoomPercent;
    }

    DOUBLE getUnzoomedY(DOUBLE e) const {
        return (e * 100.0f) / m_eZoomPercent;
    }

    SIZE getImageSize() const {
        SIZE siz = {0, 0};
        if (m_hbmImage != NULL) {
            BITMAP bm;
            if (::GetObject(m_hbmImage, sizeof(BITMAP), &bm)) {
                siz = {bm.bmWidth, bm.bmHeight};
            }
        }
        return siz;
    }

    ESize getZoomedImageSize() const {
        SIZE siz = getImageSize();
        DOUBLE cx = getZoomedX(DOUBLE(siz.cx));
        DOUBLE cy = getZoomedY(DOUBLE(siz.cy));
        ESize esiz = {cx, cy};
        return esiz;
    }

    RECT getRealClientRect() const {
        RECT rcClient;
        ::GetClientRect(m_hRealClientWnd, &rcClient);
        return rcClient;
    }

    SIZE getRealClientSize() const {
        RECT rcClient = getRealClientRect();
        SIZE sizClient = {
            rcClient.right - rcClient.left,
            rcClient.bottom - rcClient.top
        };
        return sizClient;
    }

    void mapClientPixelsToImagePixels(POINT& ptClient, EPoint& eptImage) const {
        if (m_hbmImage == NULL) {
            eptImage.x = eptImage.y = 0.0;
            return;
        }
        ESize sizZoomedImage = getZoomedImageSize();
        SIZE sizClient = getRealClientSize();
        DOUBLE cx = sizZoomedImage.cx;
        DOUBLE cy = sizZoomedImage.cy;

        INT nPos;
        if (cx <= sizClient.cx) {
            nPos = 0;
        } else {
            nPos = ::GetScrollPos(m_hRealClientWnd, SB_HORZ);
        }
        DOUBLE x = ptClient.x + nPos - (sizClient.cx - cx) / 2.0;
        if (cy <= sizClient.cy) {
            nPos = 0;
        } else {
            nPos = ::GetScrollPos(m_hRealClientWnd, SB_VERT);
        }
        DOUBLE y = ptClient.y + nPos - (sizClient.cy - cy) / 2.0;

        eptImage.x = getUnzoomedX(x);
        eptImage.y = getUnzoomedY(y);
    } // mapClientPixelsToImagePixels

    void mapImagePixelsToClientPixels(EPoint& eptImage, POINT& ptClient) const {
        if (m_hbmImage == NULL) {
            ptClient.x = ptClient.y = 0;
            return;
        }
        ESize esizZoomedImage = getZoomedImageSize();
        DOUBLE cx = esizZoomedImage.cx;
        DOUBLE cy = esizZoomedImage.cy;

        DOUBLE x = getZoomedX(eptImage.x);
        DOUBLE y = getZoomedY(eptImage.y);

        RECT rcClient;
        ::GetClientRect(m_hRealClientWnd, &rcClient);
        SIZE sizClient = {
            rcClient.right - rcClient.left,
            rcClient.bottom - rcClient.top
        };

        INT nPos;
        if (cx <= sizClient.cx) {
            nPos = 0;
        } else {
            nPos = ::GetScrollPos(m_hRealClientWnd, SB_HORZ);
        }
        ptClient.x = LONG(x - nPos + (sizClient.cx - cx) / 2.0 + 0.5);
        if (cy <= sizClient.cy) {
            nPos = 0;
        } else {
            nPos = ::GetScrollPos(m_hRealClientWnd, SB_VERT);
        }
        ptClient.y = LONG(y - nPos + (sizClient.cy - cy) / 2.0 + 0.5);
    } // mapImagePixelsToClientPixels

    INT getRounded(DOUBLE e) const {
        return INT(e + 0.5);
    }

    void updateStatusBar() {
        TCHAR szBuf[128];
        if (m_is_pdf) {
            #ifdef UNICODE
                swprintf(szBuf, 128, TEXT("Zoom: %.1f%%, Page: %d / %d"),
                    m_eZoomPercent, m_nPageIndex + 1, m_nPageCount);
            #else
                sprintf_s(szBuf, TEXT("Zoom: %.1f%%, Page: %d / %d"),
                    m_eZoomPercent, m_nPageIndex + 1, m_nPageCount);
            #endif
        } else {
            #ifdef UNICODE
                swprintf(szBuf, 128, TEXT("Zoom: %.1f%%"), m_eZoomPercent);
            #else
                sprintf_s(szBuf, TEXT("Zoom: %.1f%%"), m_eZoomPercent);
            #endif
        }
        ::SendMessage(m_hStatusBar, SB_SETTEXT, 0, LPARAM(szBuf));
    }

    void setPageIndex(INT nIndex) {
        HBITMAP hbm = loadPdf(nIndex);
        if (hbm != NULL) {
            if (m_hbmImage != NULL) {
                ::DeleteObject(m_hbmImage);
            }
            m_hbmImage = hbm;
            m_nPageIndex = nIndex;
            updateScrollInfo(true);
            updateClientImage();
        }
    }

    void onNextPage() {
        if (m_nPageIndex + 1 < m_nPageCount) {
            setPageIndex(m_nPageIndex + 1);
        }
    }
    void onPrevPage() {
        if (m_nPageIndex - 1 >= 0) {
            setPageIndex(m_nPageIndex - 1);
        }
    }
    void onGoToPage();

    void setZoomRate(DOUBLE e) {
        if (e > 0.0) {
            SIZE siz = getImageSize();
            if ((siz.cx > 0) && (siz.cy > 0)) {
                if ((siz.cx * e / 100 < 0x7FFF) && (siz.cy * e / 100 < 0x7FFF)) {
                    DOUBLE x = DOUBLE(::GetScrollPos(m_hRealClientWnd, SB_HORZ));
                    DOUBLE y = DOUBLE(::GetScrollPos(m_hRealClientWnd, SB_VERT));
                    DOUBLE eOldZoom = m_eZoomPercent;
                    m_eZoomPercent = e;
                    updateScrollInfo(true);
                    x *= e / eOldZoom;
                    y *= e / eOldZoom;
                    ::SetScrollPos(m_hRealClientWnd, SB_HORZ, getRounded(x), TRUE);
                    ::SetScrollPos(m_hRealClientWnd, SB_VERT, getRounded(y), TRUE);
                    updateClientImage();
                    updateStatusBar();
                }
            }
        }
    }

    ESize getFitPercents() const {
        ESize esiz = {0, 0};
        SIZE sizImage = getImageSize();
        SIZE sizClient = getRealClientSize();
        if (sizImage.cx > 0 && sizImage.cy > 0) {
            esiz.cx = sizClient.cx * 100.0 / sizImage.cx;
            esiz.cy = sizClient.cy * 100.0 / sizImage.cy;
        }
        return esiz;
    }

    void fitWhile() {
        ESize esiz = getFitPercents();
        if (esiz.cx < esiz.cy) {
            setZoomRate(esiz.cx);
        } else {
            setZoomRate(esiz.cy);
        }
    }

    void fitHorizontal() {
        ESize esiz = getFitPercents();
        setZoomRate(esiz.cx);
    }

    void fitVertical() {
        ESize esiz = getFitPercents();
        setZoomRate(esiz.cy);
    }

    void onZoomRate();

    void updateScrollInfo(bool reset_pos = false) {
        DOUBLE x, y, cx, cy;
        if (m_hbmImage != NULL) {
            ::EnableScrollBar(m_hRealClientWnd, SB_BOTH, ESB_ENABLE_BOTH);

            ESize esizZoomedImage = getZoomedImageSize();
            cx = esizZoomedImage.cx;
            cy = esizZoomedImage.cy;

            RECT rc;
            SIZE sizClient;
            SCROLLINFO si;
            INT nPos;

            ::GetClientRect(m_hRealClientWnd, &rc);
            sizClient = {rc.right - rc.left, rc.bottom - rc.top};

            if (reset_pos) {
                nPos = 0;
            } else {
                nPos = ::GetScrollPos(m_hRealClientWnd, SB_HORZ);
            }
            if (cx <= sizClient.cx) {
                x = (sizClient.cx - cx) / 2;
            } else {
                x = (sizClient.cx - cx) / 2 - nPos;
            }

            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
            si.nMin = getRounded(x);
            si.nMax = getRounded(x + cx);
            si.nPage = sizClient.cx;
            si.nPos = nPos;
            ::SetScrollInfo(m_hRealClientWnd, SB_HORZ, &si, TRUE);

            if (reset_pos) {
                nPos = 0;
            } else {
                nPos = ::GetScrollPos(m_hRealClientWnd, SB_VERT);
            }
            if (cy <= sizClient.cy) {
                y = (sizClient.cy - cy) / 2;
            } else {
                y = (sizClient.cy - cy) / 2 - nPos;
            }

            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
            si.nMin = getRounded(y);
            si.nMax = getRounded(y + cy);
            si.nPage = sizClient.cy;
            si.nPos = nPos;
            ::SetScrollInfo(m_hRealClientWnd, SB_VERT, &si, TRUE);
        }
    } // updateScrollInfo

    bool isGray(COLORREF rgb) const {
        BYTE r = GetRValue(rgb);
        BYTE g = GetGValue(rgb);
        BYTE b = GetBValue(rgb);
        INT dr = abs(r - 128);
        INT dg = abs(g - 128);
        INT db = abs(b - 128);
        return (dr + dg + db < 10);
    }

    void drawTwoPointCircle(HDC hdc, POINT pt0, POINT pt1) {
        DOUBLE cx = (pt0.x + pt1.x) / 2.0;
        DOUBLE cy = (pt0.y + pt1.y) / 2.0;
        INT dx = pt1.x - pt0.x;
        INT dy = pt1.y - pt0.y;
        DOUBLE distance = sqrt(dx * dx + dy * dy);
        DOUBLE radius = distance * 0.5;
        DOUBLE x0 = cx - radius;
        DOUBLE y0 = cy - radius;
        DOUBLE x1 = cx + radius;
        DOUBLE y1 = cy + radius;
        ::Ellipse(hdc,
            getRounded(x0), getRounded(y0),
            getRounded(x1), getRounded(y1));
    }

    void updateClientImage() {
        if (m_hbmClient != NULL) {
            ::DeleteObject(m_hbmClient);
            m_hbmClient = NULL;
        }

        SIZE sizClient = getRealClientSize();

        HDC hdc = ::GetDC(m_hRealClientWnd);
        m_display_dpi.x = DOUBLE(::GetDeviceCaps(hdc, LOGPIXELSX));
        m_display_dpi.y = DOUBLE(::GetDeviceCaps(hdc, LOGPIXELSY));

        HDC hdcMem1 = ::CreateCompatibleDC(hdc);
        m_hbmClient = ::CreateCompatibleBitmap(hdc, sizClient.cx, sizClient.cy);
        RECT rcClient = {0, 0, sizClient.cx, sizClient.cy};

        HGDIOBJ hbm1Old = SelectObject(hdcMem1, m_hbmClient);
        HBRUSH hbr = reinterpret_cast<HBRUSH>(::GetStockObject(GRAY_BRUSH));
        ::FillRect(hdcMem1, &rcClient, hbr);
        if (m_hbmImage != NULL) {
            DOUBLE x, y, cx, cy;

            SIZE sizImage = getImageSize();
            ESize esizImage = getZoomedImageSize();
            cx = esizImage.cx;
            cy = esizImage.cy;
            if (cx <= sizClient.cx) {
                x = (sizClient.cx - cx) / 2;
                ::SetScrollPos(m_hRealClientWnd, SB_HORZ, 0, TRUE);
            } else {
                x = (sizClient.cx - cx) / 2 - ::GetScrollPos(m_hRealClientWnd, SB_HORZ);
            }

            if (cy <= sizClient.cy) {
                y = (sizClient.cy - cy) / 2;
                ::SetScrollPos(m_hRealClientWnd, SB_VERT, 0, TRUE);
            } else {
                y = (sizClient.cy - cy) / 2 - ::GetScrollPos(m_hRealClientWnd, SB_VERT);
            }

            HDC hdcMem2 = ::CreateCompatibleDC(hdc);
            HGDIOBJ hbm2Old = ::SelectObject(hdcMem2, m_hbmImage);

            if (m_eZoomPercent < 200) {
                ::SetStretchBltMode(hdcMem1, STRETCH_HALFTONE);
            } else {
                ::SetStretchBltMode(hdcMem1, STRETCH_DELETESCANS);
            }
            ::StretchBlt(
                hdcMem1, getRounded(x), getRounded(y), getRounded(cx), getRounded(cy),
                hdcMem2, 0, 0, sizImage.cx, sizImage.cy, SRCCOPY
            );

            if (m_mode == MODE_SEGMENT) {
                POINT pt0, pt1;
                mapImagePixelsToClientPixels(m_ept0, pt0);
                mapImagePixelsToClientPixels(m_ept1, pt1);
                COLORREF rgb1 = ::GetPixel(hdcMem1, pt0.x, pt0.y);
                COLORREF rgb2 = ::GetPixel(hdcMem1, pt1.x, pt1.y);
                ::SelectObject(hdcMem1, ::GetStockObject(WHITE_PEN));
                ::SelectObject(hdcMem1, ::GetStockObject(NULL_BRUSH));

                HPEN hPen;
                switch (m_seg_color) {
                case SC_AUTO:
                    if (isGray(rgb1) || isGray(rgb2)) {
                    } else {
                        ::SetROP2(hdcMem1, R2_XORPEN);
                    }
                    hPen = ::CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
                    break;
                case SC_RED:
                    hPen = ::CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
                    break;
                case SC_GREEN:
                    hPen = ::CreatePen(PS_SOLID, 0, RGB(0, 192, 0));
                    break;
                case SC_BLUE:
                    hPen = ::CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
                    break;
                default:
                    assert(0);
                    hPen = reinterpret_cast<HPEN>(::GetStockObject(WHITE_PEN));
                    break;
                }

                HGDIOBJ hPenOld = ::SelectObject(hdcMem1, hPen);
                ::MoveToEx(hdcMem1, pt0.x, pt0.y, NULL);
                ::LineTo(hdcMem1, pt1.x, pt1.y);
                ::SelectObject(hdcMem1, hPenOld);
                ::DeleteObject(hPen);

                if (m_bDoesDrawCircle) {
                    drawTwoPointCircle(hdcMem1, pt0, pt1);
                }
            }

            if (m_bHasSegment) {
                POINT pt0, pt1;
                mapImagePixelsToClientPixels(m_eptSegment0, pt0);
                mapImagePixelsToClientPixels(m_eptSegment1, pt1);
                COLORREF rgb1 = ::GetPixel(hdcMem1, pt0.x, pt0.y);
                COLORREF rgb2 = ::GetPixel(hdcMem1, pt1.x, pt1.y);
                ::SelectObject(hdcMem1, ::GetStockObject(NULL_BRUSH));

                HPEN hPen;
                switch (m_seg_color) {
                case SC_AUTO:
                    if (isGray(rgb1) || isGray(rgb2)) {
                    } else {
                        ::SetROP2(hdcMem1, R2_XORPEN);
                    }
                    hPen = ::CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                    break;
                case SC_RED:
                    hPen = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                    break;
                case SC_GREEN:
                    hPen = ::CreatePen(PS_SOLID, 2, RGB(0, 192, 0));
                    break;
                case SC_BLUE:
                    hPen = ::CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
                    break;
                default:
                    assert(0);
                    hPen = reinterpret_cast<HPEN>(::GetStockObject(WHITE_PEN));
                    break;
                }

                HGDIOBJ hPenOld = ::SelectObject(hdcMem1, hPen);
                ::MoveToEx(hdcMem1, pt0.x, pt0.y, NULL);
                ::LineTo(hdcMem1, pt1.x, pt1.y);
                ::Rectangle(hdcMem1, pt0.x - 1, pt0.y - 1, pt0.x + 2, pt0.y + 2);
                ::Rectangle(hdcMem1, pt1.x - 1, pt1.y - 1, pt1.x + 2, pt1.y + 2);
                if (m_bDoesDrawCircle) {
                    drawTwoPointCircle(hdcMem1, pt0, pt1);
                }
                ::SelectObject(hdcMem1, hPenOld);
                ::DeleteObject(hPen);
            }

            ::SelectObject(hdcMem2, hbm2Old);
            ::DeleteDC(hdcMem2);
        }
        ::SelectObject(hdcMem1, hbm1Old);
        ::DeleteDC(hdcMem1);

        ::ReleaseDC(m_hRealClientWnd, hdc);
        ::InvalidateRect(m_hRealClientWnd, NULL, TRUE);
    } // updateClientImage

    // WM_PAINT
    void onPaint() {
        RECT rcClient = getRealClientRect();
        SIZE sizClient = getRealClientSize();

        BITMAP bmClient;
        if (m_hbmClient != NULL) {
            ::GetObject(m_hbmClient, sizeof(BITMAP), &bmClient);
        }

        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(m_hRealClientWnd, &ps);
        if (hdc != NULL) {
            HDC hdcMem1 = ::CreateCompatibleDC(hdc);
            if (m_hbmClient == NULL) {
                updateClientImage();
            }
            {
                HGDIOBJ hbm1Old = SelectObject(hdcMem1, m_hbmClient);
                ::BitBlt(hdc, rcClient.left, rcClient.top, sizClient.cx, sizClient.cy, hdcMem1, 0, 0, SRCCOPY);
                ::SelectObject(hdcMem1, hbm1Old);
            }
            ::DeleteDC(hdcMem1);
            ::EndPaint(m_hRealClientWnd, &ps);
        }
    } // onPaint

    // WM_LBUTTONDOWN
    void onLButtonDown(UINT fwKeys, INT xPos, INT yPos) {
        switch (m_nTaskIndex) {
        case DLGINDEX_INPUTSTD: case DLGINDEX_MEASURING:
            break;
        default:
            return;
        }
        bool flag = false;
        m_pt0 = {xPos, yPos};
        m_pt1 = {xPos, yPos};
        mapClientPixelsToImagePixels(m_pt0, m_ept0);
        m_mode = MODE_SEGMENT;
        if (m_bHasSegment) {
            INT dx, dy;
            POINT ptStd0, ptStd1;
            mapImagePixelsToClientPixels(m_eptSegment0, ptStd0);
            mapImagePixelsToClientPixels(m_eptSegment1, ptStd1);
            dx = ptStd0.x - xPos;
            dy = ptStd0.y - yPos;
            if (abs(dx) + abs(dy) <= 4) {
                m_mode = MODE_PT0;
                flag = true;
            }
            dx = ptStd1.x - xPos;
            dy = ptStd1.y - yPos;
            if (abs(dx) + abs(dy) <= 4) {
                m_mode = MODE_PT1;
                flag = true;
            }
        }
        if (flag) {
            ::SetCursor(m_ahCursors[IDC_MOVE - 1]);
        } else {
            ::SetCursor(m_ahCursors[IDC_CROSS2 - 1]);
        }
        ::SetCapture(m_hRealClientWnd);
        m_two_button_moved = false;
    } // onLButtonDown

    // WM_MBUTTONDOWN
    void onMButtonDown(UINT fwKeys, INT xPos, INT yPos) {
        m_pt0 = {xPos, yPos};
        m_pt1 = {xPos, yPos};
        mapClientPixelsToImagePixels(m_pt0, m_ept0);
        m_two_button_moved = false;
        ::SetCursor(m_ahCursors[IDC_PAN - 1]);
    } // onMButtonDown

    // WM_RBUTTONDOWN
    void onRButtonDown(UINT fwKeys, INT xPos, INT yPos) {
        m_pt0 = {xPos, yPos};
        m_pt1 = {xPos, yPos};
        mapClientPixelsToImagePixels(m_pt0, m_ept0);
        m_two_button_moved = false;
    } // onRButtonDown

    // WM_LBUTTONUP
    void onLButtonUp(UINT fwKeys, INT xPos, INT yPos) {
        switch (m_nTaskIndex) {
        case DLGINDEX_INPUTSTD: case DLGINDEX_MEASURING:
            break;
        default:
            return;
        }
        POINT pt0;
        mapImagePixelsToClientPixels(m_ept0, pt0);
        POINT pt1 = {xPos, yPos};
        mapClientPixelsToImagePixels(pt1, m_ept1);
        INT dx = pt0.x - pt1.x;
        INT dy = pt0.y - pt1.y;
        bool dragged = (abs(dx) + abs(dy) > 2);
        if (m_mode == MODE_SEGMENT) {
            if (dragged) {
                m_bHasSegment = true;
                m_eptSegment0 = m_ept0;
                m_eptSegment1 = m_ept1;
            }
        } else if (m_mode == MODE_PT0) {
            if (dragged) {
                m_bHasSegment = true;
                m_eptSegment0 = m_ept1;
            }
        } else if (m_mode == MODE_PT1) {
            if (dragged) {
                m_bHasSegment = true;
                m_eptSegment1 = m_ept1;
            }
        } else {
            ;
        }
        if (m_bHasSegment) {
            if (m_nTaskIndex == DLGINDEX_INPUTSTD) {
                m_eptStd0 = m_eptSegment0;
                m_eptStd1 = m_eptSegment1;
                if ((m_eptStd0.x != m_eptStd1.x) ||
                    (m_eptStd0.y != m_eptStd1.y))
                {
                    m_bHasStd = true;
                    ::EnableWindow(::GetDlgItem(
                        m_hTaskDialogs[DLGINDEX_INPUTSTD], psh2), TRUE);
                } else {
                    m_bHasStd = false;
                    ::EnableWindow(::GetDlgItem(
                        m_hTaskDialogs[DLGINDEX_INPUTSTD], psh2), FALSE);
                }
            } else if (m_nTaskIndex == DLGINDEX_MEASURING) {
                switch (m_measure_type) {
                case MEASURE_LENGTH:
                    updateLength();
                    break;
                case MEASURE_INCLINATION:
                    updateInclination();
                    break;
                case MEASURE_ANGLE:
                    updateAngle();
                    break;
                default:
                    break;
                }
            } else {
                ;
            }
        }
        m_mode = MODE_NOTHING;
        ::ReleaseCapture();
        updateClientImage();
    } // onLButtonUp

    // WM_MBUTTONUP
    void onMButtonUp(UINT fwKeys, INT xPos, INT yPos) {
        ::SetCursor(::LoadCursor(m_hInst, IDC_ARROW));
    } // onMButtonUp

    void showRClickMenu() {
        HMENU hMenu = ::LoadMenu(m_hInst, MAKEINTRESOURCE(2));
        HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
        POINT pt;
        ::GetCursorPos(&pt);
        ::SetForegroundWindow(m_hWnd);
        ::TrackPopupMenuEx(hSubMenu, TPM_LEFTBUTTON,
            pt.x, pt.y, m_hWnd, NULL);
        ::DestroyMenu(hMenu);
        ::PostMessageW(m_hWnd, WM_NULL, 0, 0);
    } // showRClickMenu

    // WM_RBUTTONUP
    void onRButtonUp(UINT fwKeys, INT xPos, INT yPos) {
        m_pt1 = {xPos, yPos};
        if (!m_two_button_moved) {
            INT dx = m_pt1.x - m_pt0.x;
            INT dy = m_pt1.y - m_pt0.y;
            if (abs(dx) + abs(dy) <= 2) {
                showRClickMenu();
            }
        }
    } // onRButton

    void updateLength(bool flag = false) {
        DOUBLE dx, dy;
        if (flag) {
            dx = m_ept1.x - m_ept0.x;
            dy = m_ept1.y - m_ept0.y;
        } else {
            dx = m_eptSegment1.x - m_eptSegment0.x;
            dy = m_eptSegment1.y - m_eptSegment0.y;
        }
        DOUBLE distance = sqrt(dx * dx + dy * dy);
        DOUBLE length =
            distance * m_eStdLengthInTheUnits / m_eStdImagePixels;
        TCHAR szBuf[128];
        #ifdef UNICODE
            swprintf(szBuf, 128, L"%g%ls", length, m_szStdUnit);
        #else
            sprintf_s(szBuf, "%g%s", length, m_szStdUnit);
        #endif
        ::SetDlgItemText(m_hTaskDialogs[m_nTaskIndex], edt1, szBuf);
        ::SendDlgItemMessage(
            m_hTaskDialogs[m_nTaskIndex], edt1, EM_SETSEL, 0, -1);
    } // updateLength

    void updateInclination(bool flag = false) {
        DOUBLE dx, dy;
        if (flag) {
            dx = m_ept1.x - m_ept0.x;
            dy = m_ept1.y - m_ept0.y;
        } else {
            dx = m_eptSegment1.x - m_eptSegment0.x;
            dy = m_eptSegment1.y - m_eptSegment0.y;
        }
        TCHAR szBuf[64];
        if ((dx != 0) || (dy != 0)) {
            DOUBLE angle = atan2(-dy, dx);
            if (m_in_radian) {
                #ifdef UNICODE
                    swprintf(szBuf, 64, L"%g%ls", angle, loadString(14));
                #else
                    sprintf_s(szBuf, "%g%s", angle, loadString(14));
                #endif
            } else {
                angle *= 180.0 / M_PI;
                #ifdef UNICODE
                    swprintf(szBuf, 64, L"%g%ls", angle, loadString(13));
                #else
                    sprintf_s(szBuf, "%g%s", angle, loadString(13));
                #endif
            }
        } else {
            szBuf[0] = 0;
        }
        ::SetDlgItemText(m_hTaskDialogs[m_nTaskIndex], edt1, szBuf);
        ::SendDlgItemMessage(m_hTaskDialogs[m_nTaskIndex], edt1, EM_SETSEL, 0, -1);
    } // updateInclination

    void updateAngle(bool flag = false) {
        DOUBLE dx0, dy0, dx1, dy1;
        dx0 = m_eptStd1.x - m_eptStd0.x;
        dy0 = m_eptStd1.y - m_eptStd0.y;
        if (flag) {
            dx1 = m_ept1.x - m_ept0.x;
            dy1 = m_ept1.y - m_ept0.y;
        } else {
            dx1 = m_eptSegment1.x - m_eptSegment0.x;
            dy1 = m_eptSegment1.y - m_eptSegment0.y;
        }
        TCHAR szBuf[64];
        if (((dx0 != 0) || (dy0 != 0)) && ((dx1 != 0) || (dy1 != 0))) {
            DOUBLE angle0 = atan2(-dy0, dx0);
            DOUBLE angle1 = atan2(-dy1, dx1);
            DOUBLE delta = angle1 - angle0;
            if (delta >= M_PI) {
                delta -= 2 * M_PI;
            }
            if (delta <= -M_PI) {
                delta += 2 * M_PI;
            }
            if (m_in_radian) {
                #ifdef UNICODE
                    swprintf(szBuf, 64, L"%g%ls", delta, loadString(14));
                #else
                    sprintf_s(szBuf, "%g%s", delta, loadString(14));
                #endif
            } else {
                delta *= 180.0 / M_PI;
                #ifdef UNICODE
                    swprintf(szBuf, 64, L"%g%ls", delta, loadString(13));
                #else
                    sprintf_s(szBuf, "%g%s", delta, loadString(13));
                #endif
            }
        } else {
            szBuf[0] = 0;
        }
        ::SetDlgItemText(m_hTaskDialogs[m_nTaskIndex], edt1, szBuf);
        ::SendDlgItemMessage(m_hTaskDialogs[m_nTaskIndex], edt1, EM_SETSEL, 0, -1);
    } // updateAngle

    // WM_MOUSEMOVE
    void onMouseMove(UINT fwKeys, INT xPos, INT yPos) {
        SCROLLINFO si;
        SIZE siz;

        if ((fwKeys & MK_LBUTTON) && (fwKeys & MK_RBUTTON)) {
            m_two_button_moved = true;
        }

        if ((fwKeys & MK_MBUTTON) || (
                (fwKeys & MK_LBUTTON) && (fwKeys & MK_RBUTTON)
            )
        )
        {
            // wheel dragging
            ::SetCursor(m_ahCursors[IDC_PAN - 1]);

            siz.cx = m_pt0.x - xPos;
            siz.cy = m_pt0.y - yPos;

            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
            ::GetScrollInfo(m_hRealClientWnd, SB_HORZ, &si);
            si.nPos += siz.cx;
            if (si.nPos > si.nMax) {
                si.nPos = si.nMax;
            }
            if (si.nPos < si.nMin) {
                si.nPos = si.nMin;
            }
            ::SetScrollPos(m_hRealClientWnd, SB_HORZ, si.nPos, TRUE);

            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
            ::GetScrollInfo(m_hRealClientWnd, SB_VERT, &si);
            si.nPos += siz.cy;
            if (si.nPos > si.nMax) {
                si.nPos = si.nMax;
            }
            if (si.nPos < si.nMin) {
                si.nPos = si.nMin;
            }
            ::SetScrollPos(m_hRealClientWnd, SB_VERT, si.nPos, TRUE);

            updateClientImage();
            m_pt0 = {xPos, yPos};
        } else {
            bool flag = false;
            switch (m_nTaskIndex) {
            case DLGINDEX_INPUTSTD: case DLGINDEX_MEASURING:
                flag = true;
                break;
            default:
                break;
            }

            if (flag) {
                flag = false;
                if (m_bHasSegment) {
                    INT dx, dy;
                    POINT ptStd0, ptStd1;
                    mapImagePixelsToClientPixels(m_eptSegment0, ptStd0);
                    mapImagePixelsToClientPixels(m_eptSegment1, ptStd1);
                    dx = ptStd0.x - xPos;
                    dy = ptStd0.y - yPos;
                    if (abs(dx) + abs(dy) <= 4) {
                        flag = true;
                    }
                    dx = ptStd1.x - xPos;
                    dy = ptStd1.y - yPos;
                    if (abs(dx) + abs(dy) <= 4) {
                        flag = true;
                    }
                }

                if (flag) {
                    ::SetCursor(m_ahCursors[IDC_MOVE - 1]);
                } else {
                    ::SetCursor(m_ahCursors[IDC_CROSS2 - 1]);
                }

                m_pt1 = {xPos, yPos};
                mapClientPixelsToImagePixels(m_pt1, m_ept1);
                if (m_mode == MODE_PT0) {
                    m_eptSegment0 = m_ept1;
                }
                if (m_mode == MODE_PT1) {
                    m_eptSegment1 = m_ept1;
                }

                RECT rcClient = getRealClientRect();
                switch (m_mode) {
                case MODE_SEGMENT: case MODE_PT0: case MODE_PT1:
                    if (::GetCapture() == m_hRealClientWnd) {
                        if (m_pt1.x < rcClient.left) {
                            onHScroll(SB_LINELEFT, 0);
                        }
                        if (m_pt1.x > rcClient.right) {
                            onHScroll(SB_LINERIGHT, 0);
                        }
                        if (m_pt1.y < rcClient.top) {
                            onVScroll(SB_LINEUP, 0);
                        }
                        if (m_pt1.y > rcClient.bottom) {
                            onVScroll(SB_LINEDOWN, 0);
                        }
                    }
                    updateClientImage();
                    break;
                default:
                    break;
                }

                if (m_nTaskIndex == DLGINDEX_MEASURING) {
                    switch (m_measure_type) {
                    case MEASURE_LENGTH:
                        switch (m_mode) {
                        case MODE_SEGMENT:
                            updateLength(true);
                            break;
                        case MODE_PT0:
                        case MODE_PT1:
                        case MODE_NOTHING:
                            if (m_bHasSegment) {
                                updateLength();
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case MEASURE_INCLINATION:
                        switch (m_mode) {
                        case MODE_SEGMENT:
                            updateInclination(true);
                            break;
                        case MODE_PT0:
                        case MODE_PT1:
                        case MODE_NOTHING:
                            if (m_bHasSegment) {
                                updateInclination();
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case MEASURE_ANGLE:
                        switch (m_mode) {
                        case MODE_SEGMENT:
                            updateAngle(true);
                            break;
                        case MODE_PT0:
                        case MODE_PT1:
                        case MODE_NOTHING:
                            if (m_bHasSegment) {
                                updateAngle();
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        // update status info
        updateStatusBar();
    } // onMouseMove

    INT m_nWheelScrollLines;
    INT m_nWheelScrollChars;

    void getWheelLines() {
        HKEY hKey;
        TCHAR sz[32];
        DWORD cbSize;
        INT nWheelScrollLines = 3, nWheelScrollChars = 3;
        if (ERROR_SUCCESS == ::RegOpenKeyEx(
            HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0,
            KEY_READ, &hKey)
        )
        {
            cbSize = sizeof(sz);
            if (ERROR_SUCCESS == ::RegQueryValueEx(hKey, 
                TEXT("WheelScrollLines"), NULL,
                NULL, reinterpret_cast<LPBYTE>(sz), &cbSize)
            )
            {
                #ifdef UNICODE
                    nWheelScrollLines = _wtoi(sz);
                #else
                    nWheelScrollLines = atoi(sz);
                #endif
            }
            cbSize = sizeof(sz);
            if (ERROR_SUCCESS == ::RegQueryValueEx(hKey, 
                TEXT("WheelScrollChars"), NULL,
                NULL, reinterpret_cast<LPBYTE>(sz), &cbSize)
            )
            {
                #ifdef UNICODE
                    nWheelScrollChars = _wtoi(sz);
                #else
                    nWheelScrollChars = atoi(sz);
                #endif
            }
            ::RegCloseKey(hKey);
        }
        if (nWheelScrollLines == -1) {
            ;
        } else {
            if (nWheelScrollLines < 1) {
                nWheelScrollLines = 1;
            }
            if (nWheelScrollLines > 30) {
                nWheelScrollLines = 30;
            }
        }
        m_nWheelScrollLines = nWheelScrollLines;
        if (nWheelScrollChars == -1) {
            ;
        } else {
            if (nWheelScrollChars < 1) {
                nWheelScrollChars = 1;
            }
            if (nWheelScrollChars > 30) {
                nWheelScrollChars = 30;
            }
        }
        m_nWheelScrollChars = nWheelScrollChars;
    }

    void zoomIn() {
        DOUBLE e = m_eZoomPercent;
        if (e < 50.0) {
            e += 2.5;
        } else if (e < 100.0) {
            e += 5.0;
        } else if (e < 1000.0) {
            e += 25.0;
        } else {
            e += 250.0;
        }
        setZoomRate(e);
        m_fit_mode = FIT_NONE;
    }

    void zoomOut() {
        DOUBLE e = m_eZoomPercent;
        if (e < 50.0) {
            e -= 2.5;
        } else if (e < 100.0) {
            e -= 5.0;
        } else if (e < 1000.0) {
            e -= 25.0;
        } else {
            e -= 250.0;
        }
        setZoomRate(e);
        m_fit_mode = FIT_NONE;
    }

    // WM_MOUSEWHEEL
    void onMouseWheel(HWND hwnd, INT x, INT y, INT zDelta, UINT fwKeys) {
        if (hwnd == m_hWnd) {
            onMouseWheel(m_hRealClientWnd, x, y, zDelta, fwKeys);
            return;
        }
        if (::GetAsyncKeyState(VK_CONTROL) < 0) {
            if (zDelta < 0) {
                zoomOut();
            } else if (zDelta > 0) {
                zoomIn();
            }
        } else {
            static bool gotWheelLines = false;
            if (!gotWheelLines) {
                getWheelLines();
            }
            if (::GetAsyncKeyState(VK_SHIFT) < 0) {
                ::SetCursor(m_ahCursors[IDC_LEFTRIGHT - 1]);
                if (m_nWheelScrollChars == -1) {
                    if (zDelta < 0)
                        ::SendMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_PAGELEFT, 0), 0);
                    else if (zDelta > 0)
                        ::SendMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_PAGERIGHT, 0), 0);
                } else {
                    for (INT i = 0; i < m_nWheelScrollChars; ++i) {
                        if (zDelta < 0)
                            ::SendMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), 0);
                        else if (zDelta > 0)
                            ::SendMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), 0);
                    }
                }
            } else {
                ::SetCursor(m_ahCursors[IDC_UPDOWN - 1]);
                if (m_nWheelScrollLines == -1) {
                    if (zDelta < 0)
                        ::SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
                    else if (zDelta > 0)
                        ::SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
                } else {
                    for (INT i = 0; i < m_nWheelScrollLines; ++i) {
                        if (zDelta < 0)
                            ::SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
                        else if (zDelta > 0)
                            ::SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
                    }
                }
            }
        }
    } // onMouseWheel

    void onHScroll(UINT nCode, INT nPos) {
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
        ::GetScrollInfo(m_hRealClientWnd, SB_HORZ, &si);

        switch (nCode) {
        case SB_LEFT:
            si.nPos = si.nMin;
            break;
        case SB_RIGHT:
            si.nPos = si.nMax;
            break;
        case SB_LINELEFT:
            si.nPos -= 10;
            break;
        case SB_LINERIGHT:
            si.nPos += 10;
            break;
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            si.nPos = nPos;
            break;
        default:
            break;
        }
        if (si.nPos > si.nMax) {
            si.nPos = si.nMax;
        }
        if (si.nPos < si.nMin) {
            si.nPos = si.nMin;
        }
        ::SetScrollInfo(m_hRealClientWnd, SB_HORZ, &si, TRUE);
        updateClientImage();
    } // onHScroll

    void onVScroll(UINT nCode, INT nPos) {
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
        ::GetScrollInfo(m_hRealClientWnd, SB_VERT, &si);

        switch (nCode) {
        case SB_TOP:
            si.nPos = si.nMin;
            break;
        case SB_BOTTOM:
            si.nPos = si.nMax - si.nPage;
            break;
        case SB_LINEUP:
            si.nPos -= 10;
            break;
        case SB_LINEDOWN:
            si.nPos += 10;
            break;
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            si.nPos = nPos;
            break;
        default:
            break;
        }
        if (si.nPos > si.nMax) {
            si.nPos = si.nMax;
        }
        if (si.nPos < si.nMin) {
            si.nPos = si.nMin;
        }
        ::SetScrollInfo(m_hRealClientWnd, SB_VERT, &si, TRUE);
        updateClientImage();
    } // onVScroll

    // WM_GETMINMAXINFO
    void onGetMinMaxInfo(LPMINMAXINFO pmmi) {
        if (m_hWnd == NULL) {
            return;
        }

        RECT rcStatus;
        ::SendMessage(m_hStatusBar, WM_SIZE, 0, 0);
        ::GetWindowRect(m_hStatusBar, &rcStatus);
        SIZE sizStatus = {
            rcStatus.right - rcStatus.left,
            rcStatus.bottom - rcStatus.top
        };

        RECT rc = {0, 0, m_sizTaskDlg.cx, m_sizTaskDlg.cy};

        DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
        DWORD dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        ::AdjustWindowRectEx(&rc, dwStyle, TRUE, dwExStyle);

        INT cx = rc.right - rc.left;
        INT cy = rc.bottom - rc.top;
        pmmi->ptMinTrackSize.x = cx * 2;
        pmmi->ptMinTrackSize.y = cy + sizStatus.cy;
    } // onGetMinMaxInfo

    // WM_SIZE
    void onSize(UINT type, INT width, INT height) {
        // NOTE: m_hWnd can be NULL at here. Needs check.
        if (m_hWnd == NULL) {
            return;
        }

        int cxTask = m_sizTaskDlg.cx;

        RECT rc;
        ::GetClientRect(m_hWnd, &rc);

        RECT rcStatus;
        ::SendMessage(m_hStatusBar, WM_SIZE, 0, 0);
        ::GetWindowRect(m_hStatusBar, &rcStatus);
        SIZE sizStatus = {
            rcStatus.right - rcStatus.left,
            rcStatus.bottom - rcStatus.top
        };

        HDWP hDWP = ::BeginDeferWindowPos(2);
        ::DeferWindowPos(hDWP, m_hTaskDialogs[m_nTaskIndex], NULL,
            rc.left,
            rc.top,
            cxTask,
            rc.bottom - rc.top - sizStatus.cy,
            SWP_NOACTIVATE | SWP_NOZORDER
        );
        ::DeferWindowPos(hDWP, m_hRealClientWnd, NULL,
            rc.left + cxTask,
            rc.top,
            (rc.right - rc.left) - cxTask,
            rc.bottom - rc.top - sizStatus.cy,
            SWP_NOACTIVATE | SWP_NOZORDER
        );
        ::EndDeferWindowPos(hDWP);

        updateScrollInfo();
        switch (m_fit_mode) {
        case FIT_NONE:
            updateClientImage();
            break;
        case FIT_WHOLE:
            fitWhile();
            break;
        case FIT_HORIZONTAL:
            fitHorizontal();
            break;
        case FIT_VERTICAL:
            fitVertical();
            break;
        default:
            assert(0);
            break;
        }
    } // onSize

    // IDM_EXIT
    void onExit() {
        ::DestroyWindow(m_hWnd);
    } // onExit

    // IDM_ABOUT
    void onAbout();

    std::vector<tstring>   m_unit_names;

    void initTaskDialog3(HWND hWnd) {
        if (m_unit_names.empty()) {
            COMBOBOXEXITEM item;
            LPTSTR psz = makeFilter(loadString(7));
            LPTSTR pch = psz;
            ::SendDlgItemMessage(hWnd, cmb1, CB_RESETCONTENT, 0, 0);
            while (*pch != 0) {
                LPTSTR pchUnit = pch;
                item.mask = CBEIF_TEXT;
                item.iItem = -1;
                item.pszText = pchUnit;
                item.cchTextMax = -1;
                ::SendDlgItemMessage(hWnd, cmb1, CBEM_INSERTITEM, 0,
                                     reinterpret_cast<LPARAM>(&item));
                m_unit_names.emplace_back(pchUnit);
                pch += ::lstrlen(pch) + 1;
            }
            ::SendDlgItemMessage(hWnd, cmb1, CB_SETCURSEL, 0, 0);
        }

        TCHAR szBuf[64];
        if (m_eStdLengthInTheUnits > 0.0) {
            #ifdef UNICODE
                swprintf(szBuf, 64, L"%g", m_eStdLengthInTheUnits);
            #else
                sprintf_s(szBuf, "%g", m_eStdLengthInTheUnits);
            #endif
            ::SetDlgItemText(hWnd, edt1, szBuf);
            ::SetDlgItemText(hWnd, cmb1, m_szStdUnit);
        } else {
            szBuf[0] = 0;
            ::SetDlgItemText(hWnd, edt1, szBuf);
            ::SetDlgItemText(hWnd, cmb1, TEXT("mm"));
        }
    } // initTaskDialog3

    void onChx1Check(bool checked) {
        m_bGotStarted = checked;
        saveSettings();
    }

    bool loadSettings(void) {
        bool ret = false;
        HKEY hSoftware, hCompany, hAppKey, hRecentKey;

        m_recent_files.clear();

        if (ERROR_SUCCESS == ::RegOpenKeyEx(
            HKEY_CURRENT_USER, s_szSoftware, 0, KEY_READ, &hSoftware))
        {
            if (ERROR_SUCCESS == ::RegOpenKeyEx(
                hSoftware, s_szCompanyName, 0, KEY_READ, &hCompany))
            {
                if (ERROR_SUCCESS == ::RegOpenKeyEx(
                    hCompany, s_szClassName, 0, KEY_READ, &hAppKey))
                {
                    DWORD dwValue, cbSize;

                    if (ERROR_SUCCESS == ::RegOpenKeyEx(
                        hAppKey, s_szRecentFileList, 0, KEY_READ, &hRecentKey))
                    {
                        TCHAR szValueName[32], szFilePath[MAX_PATH];
                        for (DWORD i = 1; i < MAX_RECENT; ++i) {
                            ::wsprintf(szValueName, s_szFileFormat, UINT(i));
                            cbSize = sizeof(szFilePath);
                            szFilePath[0] = 0;
                            LONG n = ::RegQueryValueEx(
                                hRecentKey, szValueName, NULL, NULL,
                                reinterpret_cast<LPBYTE>(&szFilePath), &cbSize
                            );
                            if ((n == ERROR_SUCCESS) && (szFilePath[0] != 0)) {
                                m_recent_files.emplace_back(szFilePath);
                            } else {
                                break;
                            }
                        }
                        ::RegCloseKey(hRecentKey);
                    }

                    cbSize = sizeof(DWORD);
                    dwValue = 0;
                    ::RegQueryValueEx(
                        hAppKey, s_szGotStarted, NULL, NULL,
                        reinterpret_cast<LPBYTE>(&dwValue), &cbSize
                    );
                    m_bGotStarted = !!dwValue;

                    cbSize = sizeof(DWORD);
                    dwValue = CW_USEDEFAULT;
                    ::RegQueryValueEx(
                        hAppKey, s_szWindowWidth, NULL, NULL,
                        reinterpret_cast<LPBYTE>(&dwValue), &cbSize
                    );
                    m_nWindowWidth = INT(dwValue);

                    cbSize = sizeof(DWORD);
                    dwValue = CW_USEDEFAULT;
                    ::RegQueryValueEx(
                        hAppKey, s_szWindowHeight, NULL, NULL,
                        reinterpret_cast<LPBYTE>(&dwValue), &cbSize
                    );
                    m_nWindowHeight = INT(dwValue);

                    ret = true;
                    ::RegCloseKey(hAppKey);
                }
                ::RegCloseKey(hCompany);
            }
            ::RegCloseKey(hSoftware);
        }
        return ret;
    } // loadSettings

    bool saveSettings(void) {
        bool ret = false;
        HKEY hSoftware, hCompany, hAppKey, hRecentKey;
        DWORD dwDisp;
        if (ERROR_SUCCESS == ::RegCreateKeyEx(
            HKEY_CURRENT_USER, s_szSoftware, 0, NULL, 0,
            KEY_ALL_ACCESS, NULL, &hSoftware, &dwDisp))
        {
            if (ERROR_SUCCESS == ::RegCreateKeyEx(
                hSoftware, s_szCompanyName, 0, NULL, 0,
                KEY_ALL_ACCESS, NULL, &hCompany, &dwDisp))
            {
                if (ERROR_SUCCESS == ::RegCreateKeyEx(
                    hCompany, s_szClassName, 0, NULL, 0,
                    KEY_ALL_ACCESS, NULL, &hAppKey, &dwDisp))
                {
                    if (ERROR_SUCCESS == ::RegCreateKeyEx(
                        hAppKey, s_szRecentFileList, 0, NULL, 0,
                        KEY_ALL_ACCESS, NULL, &hRecentKey, &dwDisp))
                    {
                        TCHAR szValueName[32], szFilePath[MAX_PATH];
                        for (size_t i = 0; i < m_recent_files.size(); ++i) {
                            if (i >= MAX_RECENT) {
                                break;
                            }
                            ::wsprintf(szValueName, s_szFileFormat, UINT(i + 1));
                            ::lstrcpyn(szFilePath, m_recent_files[i].c_str(), MAX_PATH);
                            ::RegSetValueEx(hRecentKey, szValueName, 0,
                                REG_SZ,
                                reinterpret_cast<BYTE *>(szFilePath),
                                (::lstrlen(szFilePath) + 1) * sizeof(TCHAR)
                            );
                        }
                        ::RegCloseKey(hRecentKey);
                    }

                    DWORD dwValue = m_bGotStarted;
                    ::RegSetValueEx(hAppKey, s_szGotStarted, 0,
                        REG_DWORD,
                        reinterpret_cast<BYTE *>(&dwValue), sizeof(DWORD)
                    );

                    WINDOWPLACEMENT wndpl;
                    wndpl.length = sizeof(wndpl);
                    ::GetWindowPlacement(m_hWnd, &wndpl);
                    RECT rc = wndpl.rcNormalPosition;

                    dwValue = rc.right - rc.left;
                    ::RegSetValueEx(hAppKey, s_szWindowWidth, 0,
                        REG_DWORD,
                        reinterpret_cast<BYTE *>(&dwValue), sizeof(DWORD)
                    );

                    dwValue = rc.bottom - rc.top;
                    ::RegSetValueEx(hAppKey, s_szWindowHeight, 0,
                        REG_DWORD,
                        reinterpret_cast<BYTE *>(&dwValue), sizeof(DWORD)
                    );

                    ret = true;
                    ::RegCloseKey(hAppKey);
                }
                ::RegCloseKey(hCompany);
            }
            ::RegCloseKey(hSoftware);
        }
        return ret;
    } // saveSettings

    bool copyText(const tstring& text) {
        bool ret = false;
        if (::OpenClipboard(m_hWnd)) {
            DWORD dwBytes = DWORD((text.size() + 1) * sizeof(TCHAR));
            HGLOBAL hGlobal =
                ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dwBytes);
            if (hGlobal != NULL) {
                LPVOID pv = ::GlobalLock(hGlobal);
                if (pv != NULL) {
                    LPTSTR psz = reinterpret_cast<LPTSTR>(pv);
                    CopyMemory(psz, text.c_str(), dwBytes);
                    ::GlobalUnlock(hGlobal);
                    ::EmptyClipboard();
                    #ifdef UNICODE
                        HANDLE hHandle = ::SetClipboardData(CF_UNICODETEXT, hGlobal);
                    #else
                        HANDLE hHandle = ::SetClipboardData(CF_TEXT, hGlobal);
                    #endif
                    if (hHandle != NULL) {
                        ret = true;
                    }
                } else {
                    ::GlobalFree(hGlobal);
                }
            }
            ::CloseClipboard();
        }
        return ret;
    } // copyText

    void doStd1(HWND hWnd) {
        TCHAR szBuf[64];

        ::GetDlgItemText(hWnd, edt1, szBuf, 64);
        tstring strNumber = szBuf;
        mstr_trim(strNumber, s_szSpaces);
        ::LCMapString(LOCALE_USER_DEFAULT,
            LCMAP_HALFWIDTH,
            strNumber.c_str(),
            -1,
            szBuf,
            64
        );

        #ifdef UNICODE
            DOUBLE f = _wtof(szBuf);
        #else
            DOUBLE f = atof(szBuf);
        #endif
        if (f > 0.0) {
            ::GetDlgItemText(hWnd, cmb1, szBuf, 64);
            tstring strUnit = szBuf;
            mstr_trim(strUnit, s_szSpaces);
            m_eStdLengthInTheUnits = f;
            ::lstrcpyn(m_szStdUnit, strUnit.c_str(), 64);

            DOUBLE dx = m_eptStd1.x - m_eptStd0.x;
            DOUBLE dy = m_eptStd1.y - m_eptStd0.y;
            DOUBLE distance = sqrt(dx * dx + dy * dy);
            m_eStdImagePixels = distance;

            setTaskIndex(DLGINDEX_MEASURING);
        }
    } // doStd1

    void doStd2(HWND hWnd) {
        TCHAR szBuf[64];
        ::GetDlgItemText(hWnd, edt1, szBuf, 64);
        tstring strNumber = szBuf;
        mstr_trim(strNumber, s_szSpaces);
        ::LCMapString(LOCALE_USER_DEFAULT,
            LCMAP_HALFWIDTH,
            strNumber.c_str(),
            -1,
            szBuf,
            64
        );
        #ifdef UNICODE
            DOUBLE f = _wtof(szBuf);
        #else
            DOUBLE f = atof(szBuf);
        #endif
        if (f > 0.0) {
            ::EnableWindow(::GetDlgItem(hWnd, psh2), TRUE);
        } else {
            ::EnableWindow(::GetDlgItem(hWnd, psh2), FALSE);
        }
    } // doStd2

    void onInitMenuPopup(HMENU hPopup, BOOL fSystemMenu) {
        ::CheckMenuItem(hPopup, IDM_FIT_WHILE, MF_UNCHECKED);
        ::CheckMenuItem(hPopup, IDM_FIT_HORIZONTAL, MF_UNCHECKED);
        ::CheckMenuItem(hPopup, IDM_FIT_VERTICAL, MF_UNCHECKED);
        if (m_is_pdf) {
            if (m_nPageIndex + 1 < m_nPageCount) {
                ::EnableMenuItem(hPopup, IDM_NEXT_PAGE, MF_ENABLED);
            } else {
                ::EnableMenuItem(hPopup, IDM_NEXT_PAGE, MF_DISABLED);
            }
            if (m_nPageIndex > 0) {
                ::EnableMenuItem(hPopup, IDM_PREV_PAGE, MF_ENABLED);
            } else {
                ::EnableMenuItem(hPopup, IDM_PREV_PAGE, MF_DISABLED);
            }
            ::EnableMenuItem(hPopup, IDM_GO_TO_PAGE, MF_ENABLED);
        } else {
            ::EnableMenuItem(hPopup, IDM_NEXT_PAGE, MF_DISABLED);
            ::EnableMenuItem(hPopup, IDM_PREV_PAGE, MF_DISABLED);
            ::EnableMenuItem(hPopup, IDM_GO_TO_PAGE, MF_DISABLED);
        }
        switch (m_fit_mode) {
        case FIT_NONE:
            break;
        case FIT_WHOLE:
            ::CheckMenuItem(hPopup, IDM_FIT_WHILE, MF_CHECKED);
            break;
        case FIT_HORIZONTAL:
            ::CheckMenuItem(hPopup, IDM_FIT_HORIZONTAL, MF_CHECKED);
            break;
        case FIT_VERTICAL:
            ::CheckMenuItem(hPopup, IDM_FIT_VERTICAL, MF_CHECKED);
            break;
        default:
            assert(0);
            break;
        }
        switch (m_seg_color) {
        case SC_AUTO:
            ::CheckMenuRadioItem(hPopup,
                IDM_SEG_COLOR_AUTO, IDM_SEG_COLOR_BLUE, IDM_SEG_COLOR_AUTO, 0);
            break;
        case SC_RED:
            ::CheckMenuRadioItem(hPopup,
                IDM_SEG_COLOR_AUTO, IDM_SEG_COLOR_BLUE, IDM_SEG_COLOR_RED, 0);
            break;
        case SC_GREEN:
            ::CheckMenuRadioItem(hPopup,
                IDM_SEG_COLOR_AUTO, IDM_SEG_COLOR_BLUE, IDM_SEG_COLOR_GREEN, 0);
            break;
        case SC_BLUE:
            ::CheckMenuRadioItem(hPopup,
                IDM_SEG_COLOR_AUTO, IDM_SEG_COLOR_BLUE, IDM_SEG_COLOR_BLUE, 0);
            break;
        }

        HMENU hMenu = ::GetMenu(m_hWnd);
        HMENU hFileMenu = ::GetSubMenu(hMenu, 0);
        HMENU hRecentMenu = ::GetSubMenu(hFileMenu, 2);
        while (::DeleteMenu(hRecentMenu, 0, MF_BYPOSITION)) {
            ;
        }
        if (m_recent_files.empty()) {
            ::AppendMenu(hRecentMenu, MF_STRING | MF_GRAYED, 0, loadString(16));
        } else {
            for (size_t i = 0; i < m_recent_files.size(); ++i) {
                if (i >= MAX_RECENT) {
                    break;
                }
                tstring str = TEXT("&") + std::to_wstring(i + 1) + TEXT("\t");
                str += m_recent_files[i];
                ::AppendMenu(hRecentMenu, MF_STRING | MF_ENABLED,
                    IDM_RECENT_0 + i, str.c_str());
            }
        }
    }

    bool loadRecent(size_t i) {
        if (m_recent_files.size() > i) {
            return loadFile(m_recent_files[i].c_str());
        }
        return false;
    }
}; // WinApp

INT_PTR CALLBACK
AboutDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        CenterDialog(hWnd);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            ::EndDialog(hWnd, IDOK);
            break;
        case IDCANCEL:
            ::EndDialog(hWnd, IDCANCEL);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // AboutDialogProc

INT_PTR CALLBACK
TaskGetStartedProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        pApp->m_hTaskDialogs[DLGINDEX_GETSTARTED] = hWnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1:
            break;
        case psh2:
            pApp->setTaskIndex(DLGINDEX_LOADIMAGE);
            break;
        case chx1:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (::IsDlgButtonChecked(hWnd, chx1) == BST_CHECKED) {
                    pApp->onChx1Check(true);
                } else {
                    pApp->onChx1Check(false);
                }
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // TaskGetStartedProc

INT_PTR CALLBACK
TaskLoadImageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        pApp->m_hTaskDialogs[DLGINDEX_LOADIMAGE] = hWnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1:
            pApp->setTaskIndex(DLGINDEX_GETSTARTED);
            break;
        case psh2:
            pApp->setTaskIndex(DLGINDEX_MEASURETYPE);
            break;
        case psh3:
            pApp->onOpen();
            break;
        case ID_ACTIVATE:
            break;
        case ID_DEACTIVATE:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // TaskLoadImageProc

INT_PTR CALLBACK
TaskMeasureTypeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        pApp->m_hTaskDialogs[DLGINDEX_MEASURETYPE] = hWnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1:
            pApp->setTaskIndex(DLGINDEX_LOADIMAGE);
            break;
        case psh2:
            pApp->m_in_radian =
                !!(IsDlgButtonChecked(hWnd, chx1) == BST_CHECKED);
            {
                BOOL r1 = (IsDlgButtonChecked(hWnd, rad1) == BST_CHECKED);
                BOOL r2 = (IsDlgButtonChecked(hWnd, rad2) == BST_CHECKED);
                BOOL r3 = (IsDlgButtonChecked(hWnd, rad3) == BST_CHECKED);
                if (r1) {
                    pApp->m_measure_type = WinApp::MEASURE_LENGTH;
                    pApp->setTaskIndex(DLGINDEX_INPUTSTD);
                } else if (r2) {
                    pApp->m_measure_type = WinApp::MEASURE_INCLINATION;
                    pApp->m_eStdLengthInTheUnits = 0.0;
                    pApp->m_szStdUnit[0] = 0;
                    pApp->setTaskIndex(DLGINDEX_MEASURING);
                } else if (r3) {
                    pApp->m_measure_type = WinApp::MEASURE_ANGLE;
                    pApp->m_eStdLengthInTheUnits = 0.0;
                    pApp->m_szStdUnit[0] = 0;
                    pApp->setTaskIndex(DLGINDEX_INPUTSTD);
                } else {
                    assert(0);
                }
            }
            break;
        case psh3:
            break;
        case ID_ACTIVATE:
            if (pApp->m_in_radian) {
                ::CheckDlgButton(hWnd, chx1, BST_CHECKED);
            } else {
                ::CheckDlgButton(hWnd, chx1, BST_UNCHECKED);
            }
            switch (pApp->m_measure_type) {
            case WinApp::MEASURE_LENGTH:
                ::CheckRadioButton(hWnd, rad1, rad3, rad1);
                break;
            case WinApp::MEASURE_INCLINATION:
                ::CheckRadioButton(hWnd, rad1, rad3, rad2);
                break;
            case WinApp::MEASURE_ANGLE:
                ::CheckRadioButton(hWnd, rad1, rad3, rad3);
                break;
            default:
                assert(0);
                break;
            }
            break;
        case ID_DEACTIVATE:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // TaskMeasureTypeProc

INT_PTR CALLBACK
TaskInputStdProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    static bool updating_checkbox = false;
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        pApp->m_hTaskDialogs[DLGINDEX_INPUTSTD] = hWnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1:
            pApp->setTaskIndex(DLGINDEX_MEASURETYPE);
            break;
        case psh2:
            if (pApp->m_measure_type == WinApp::MEASURE_ANGLE) {
                pApp->setTaskIndex(DLGINDEX_MEASURING);
            } else {
                pApp->setTaskIndex(DLGINDEX_STDLENGTH);
            }
            break;
        case psh3:
            break;
        case chx1:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (!updating_checkbox) {
                    if (::IsDlgButtonChecked(hWnd, chx1) == BST_CHECKED) {
                        pApp->m_bDoesDrawCircle = true;
                    } else {
                        pApp->m_bDoesDrawCircle = false;
                    }
                    pApp->updateClientImage();
                }
            }
            break;
        case ID_ACTIVATE:
            if (pApp->m_bHasStd) {
                pApp->m_bHasSegment = true;
                pApp->m_eptSegment0 = pApp->m_eptStd0;
                pApp->m_eptSegment1 = pApp->m_eptStd1;
                ::EnableWindow(::GetDlgItem(hWnd, psh2), TRUE);
            } else {
                ::EnableWindow(::GetDlgItem(hWnd, psh2), FALSE);
            }
            updating_checkbox = true;
            if (pApp->m_bDoesDrawCircle) {
                ::CheckDlgButton(hWnd, chx1, BST_CHECKED);
            } else {
                ::CheckDlgButton(hWnd, chx1, BST_UNCHECKED);
            }
            updating_checkbox = false;
            break;
        case ID_DEACTIVATE:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // TaskInputStdProc

INT_PTR CALLBACK
TaskStdLengthProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        pApp->m_hTaskDialogs[DLGINDEX_STDLENGTH] = hWnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1:
            pApp->setTaskIndex(DLGINDEX_INPUTSTD);
            break;
        case psh2:
            pApp->doStd1(hWnd);
            break;
        case psh3:
            break;
        case ID_ACTIVATE:
            pApp->initTaskDialog3(hWnd);
            break;
        case ID_DEACTIVATE:
            break;
        case edt1:
            if (HIWORD(wParam) == EN_CHANGE) {
                pApp->doStd2(hWnd);
            }
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // TaskStdLengthProc

INT_PTR CALLBACK
TaskMeasuringLenProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    static bool updating_checkbox = false;
    TCHAR szBuf[64];
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        pApp->m_hTaskDialogs[DLGINDEX_MEASURING] = hWnd;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1:
            switch (pApp->m_measure_type) {
            case WinApp::MEASURE_LENGTH:
                pApp->m_bHasSegment = true;
                pApp->m_eptSegment0 = pApp->m_eptStd0;
                pApp->m_eptSegment1 = pApp->m_eptStd1;
                ::SetDlgItemText(hWnd, edt1, NULL);
                pApp->setTaskIndex(DLGINDEX_STDLENGTH);
                break;
            case WinApp::MEASURE_INCLINATION:
                pApp->m_bHasSegment = false;
                ::SetDlgItemText(hWnd, edt1, NULL);
                pApp->setTaskIndex(DLGINDEX_MEASURETYPE);
                break;
            case WinApp::MEASURE_ANGLE:
                pApp->m_bHasSegment = true;
                pApp->m_eptSegment0 = pApp->m_eptStd0;
                pApp->m_eptSegment1 = pApp->m_eptStd1;
                ::SetDlgItemText(hWnd, edt1, NULL);
                pApp->setTaskIndex(DLGINDEX_INPUTSTD);
                break;
            default:
                assert(0);
                break;
            }
            break;
        case psh2:
            ::DestroyWindow(pApp->m_hWnd);
            break;
        case psh3:
            ::GetDlgItemText(hWnd, edt1, szBuf, 64);
            pApp->copyText(szBuf);
            break;
        case chx1:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (!updating_checkbox) {
                    if (::IsDlgButtonChecked(hWnd, chx1) == BST_CHECKED) {
                        pApp->m_bDoesDrawCircle = true;
                    } else {
                        pApp->m_bDoesDrawCircle = false;
                    }
                    pApp->updateClientImage();
                }
            }
            break;
        case ID_ACTIVATE:
            pApp->m_bHasSegment = false;
            ::SetDlgItemText(hWnd, edt1, NULL);
            updating_checkbox = true;
            if (pApp->m_bDoesDrawCircle) {
                ::CheckDlgButton(hWnd, chx1, BST_CHECKED);
            } else {
                ::CheckDlgButton(hWnd, chx1, BST_UNCHECKED);
            }
            updating_checkbox = false;

            switch (pApp->m_measure_type) {
            case WinApp::MEASURE_LENGTH:
                ::SetDlgItemText(hWnd, stc1, pApp->loadString(10));
                break;
            case WinApp::MEASURE_INCLINATION:
                ::SetDlgItemText(hWnd, stc1, pApp->loadString(11));
                break;
            case WinApp::MEASURE_ANGLE:
                ::SetDlgItemText(hWnd, stc1, pApp->loadString(12));
                break;
            default:
                assert(0);
                break;
            }
            break;
        case ID_DEACTIVATE:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
} // TaskMeasuringLenProc

// WM_CREATE
bool WinApp::onCreate() {
    HWND hwndRealClient = ::CreateWindowEx(
        WS_EX_ACCEPTFILES | WS_EX_CLIENTEDGE,
        s_szRealClientWndClassName,
        NULL,
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
        0, 0, 0, 0,
        m_hWnd,
        reinterpret_cast<HMENU>(IDW_REALCLIENT),
        m_hInst,
        this
    );
    if (hwndRealClient == NULL) {
        return false;
    }
    m_hStatusBar = ::CreateStatusWindow(
        WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP,
        TEXT(""),
        m_hWnd,
        IDW_STATUS
    );
    static DLGPROC fn[] = {
        TaskGetStartedProc,
        TaskLoadImageProc,
        TaskMeasureTypeProc,
        TaskInputStdProc,
        TaskStdLengthProc,
        TaskMeasuringLenProc
    };
    for (int i = 0; i < NUM_TASKS; ++i) {
        HWND hwndTaskDialog = ::CreateDialogParam(
            m_hInst,
            MAKEINTRESOURCE(100 + i),
            m_hWnd,
            fn[i],
            reinterpret_cast<LPARAM>(this)
        );
        if (hwndTaskDialog == NULL) {
            return false;
        }
        RECT rc;
        ::GetWindowRect(hwndTaskDialog, &rc);
        SIZE siz;
        siz.cx = rc.right - rc.left;
        siz.cy = rc.bottom - rc.top;
        if (m_sizTaskDlg.cx < siz.cx) {
            m_sizTaskDlg.cx = siz.cx;
        }
        if (m_sizTaskDlg.cy < siz.cy) {
            m_sizTaskDlg.cy = siz.cy;
        }
    }
    updateStatusBar();

    ::DragAcceptFiles(m_hWnd, TRUE);
    ::EnableScrollBar(m_hRealClientWnd, SB_BOTH, ESB_DISABLE_BOTH);
    ::PostMessage(m_hWnd, WM_SIZE, 0, 0);

    if (m_bGotStarted) {
        ::CheckDlgButton(
            m_hTaskDialogs[DLGINDEX_GETSTARTED], chx1, BST_CHECKED);
        setTaskIndex(DLGINDEX_LOADIMAGE);
    } else {
        ::CheckDlgButton(
            m_hTaskDialogs[DLGINDEX_GETSTARTED], chx1, BST_UNCHECKED);
        setTaskIndex(DLGINDEX_GETSTARTED);
    }
    
    return true;
} // onCreate

void WinApp::onAbout() {
    ::DialogBox(m_hInst, MAKEINTRESOURCE(1), m_hWnd,
        AboutDialogProc);
} // WinApp::onAbout

INT_PTR CALLBACK
GoPageDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    TCHAR szBuf[64];
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        ::wsprintf(szBuf, TEXT("%d"), pApp->m_nPageIndex + 1);
        ::SetDlgItemText(hWnd, edt1, szBuf);
        CenterDialog(hWnd);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            ::GetDlgItemText(hWnd, edt1, szBuf, 64);
            {
                tstring strText = szBuf;
                mstr_trim(strText, s_szSpaces);
                ::LCMapString(LOCALE_USER_DEFAULT,
                    LCMAP_HALFWIDTH,
                    strText.c_str(),
                    -1,
                    szBuf,
                    64
                );
                #ifdef UNICODE
                    INT nPage = _wtoi(szBuf);
                #else
                    INT nPage = atoi(szBuf);
                #endif
                if (nPage <= 0 || nPage > pApp->m_nPageCount) {
                    ::SetFocus(::GetDlgItem(hWnd, edt1));
                    ::SendDlgItemMessage(hWnd, edt1, EM_SETSEL, 0, -1);
                    CenterMessageBox(hWnd, pApp->loadString(6),
                        pApp->loadString2(2), MB_ICONERROR);
                } else {
                    pApp->setPageIndex(nPage - 1);
                    ::EndDialog(hWnd, IDOK);
                }
            }
            break;
        case IDCANCEL:
            ::EndDialog(hWnd, IDCANCEL);
            break;
        default:
            break;
        }
    }
    return 0;
} // GoPageDialogProc

void WinApp::onGoToPage() {
    ::DialogBoxParam(m_hInst, MAKEINTRESOURCE(3), m_hWnd,
        GoPageDialogProc,
        reinterpret_cast<LPARAM>(this));
}

INT_PTR CALLBACK
ZoomRateDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    TCHAR szBuf[64];
    DOUBLE d;
    switch (uMsg) {
    case WM_INITDIALOG:
        pApp = reinterpret_cast<WinApp *>(lParam);
        #ifdef UNICODE
            swprintf(szBuf, 64, TEXT("%.1f%%"), pApp->m_eZoomPercent);
        #else
            sprintf_s(szBuf, TEXT("%.1f%%"), pApp->m_eZoomPercent);
        #endif
        ::SetDlgItemText(hWnd, edt1, szBuf);
        CenterDialog(hWnd);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            ::GetDlgItemText(hWnd, edt1, szBuf, 64);
            {
                tstring strText = szBuf;
                mstr_trim(strText, s_szSpaces);
                ::LCMapString(LOCALE_USER_DEFAULT,
                    LCMAP_HALFWIDTH,
                    strText.c_str(),
                    -1,
                    szBuf,
                    64
                );
            }
            #ifdef UNICODE
                d = _wtof(szBuf);
            #else
                d = atof(szBuf);
            #endif
            if (d <= 0) {
                ::SetFocus(::GetDlgItem(hWnd, edt1));
                ::SendDlgItemMessage(hWnd, edt1, EM_SETSEL, 0, -1);
                CenterMessageBox(hWnd, pApp->loadString(6),
                    pApp->loadString2(2), MB_ICONERROR);
            } else {
                pApp->m_eZoomPercent = d;
                ::EndDialog(hWnd, IDOK);
            }
            break;
        case IDCANCEL:
            ::EndDialog(hWnd, IDCANCEL);
            break;
        default:
            break;
        }
    }
    return 0;
} // ZoomRateDialogProc

void WinApp::onZoomRate() {
    if (::DialogBoxParam(m_hInst, MAKEINTRESOURCE(2),
        m_hWnd, ZoomRateDialogProc, reinterpret_cast<LPARAM>(this)) == IDOK)
    {
        setZoomRate(m_eZoomPercent);
    }
}

// the main window procedure
LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    LPCREATESTRUCT pcs;

    switch (uMsg) {
    case WM_CREATE:
        pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        assert(pcs != NULL);
        pApp = reinterpret_cast<WinApp *>(pcs->lpCreateParams);
        assert(pApp != NULL);
        pApp->m_hWnd = hWnd;
        if (!pApp->onCreate()) {
            return -1;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_EXIT:
            pApp->onExit();
            break;
        case IDM_ABOUT:
            pApp->onAbout();
            break;
        case IDM_OPEN:
            pApp->onOpen();
            break;
        case IDM_FIT_WHILE:
            pApp->m_fit_mode = WinApp::FIT_WHOLE;
            pApp->fitWhile();
            break;
        case IDM_FIT_HORIZONTAL:
            pApp->m_fit_mode = WinApp::FIT_HORIZONTAL;
            pApp->fitHorizontal();
            break;
        case IDM_FIT_VERTICAL:
            pApp->m_fit_mode = WinApp::FIT_VERTICAL;
            pApp->fitVertical();
            break;
        case IDM_ZOOM_10PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(10);
            break;
        case IDM_ZOOM_25PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(25);
            break;
        case IDM_ZOOM_50PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(50);
            break;
        case IDM_ZOOM_75PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(75);
            break;
        case IDM_ZOOM_100PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(100);
            break;
        case IDM_ZOOM_125PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(125);
            break;
        case IDM_ZOOM_150PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(150);
            break;
        case IDM_ZOOM_200PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(200);
            break;
        case IDM_ZOOM_300PER:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->setZoomRate(300);
            break;
        case IDM_ZOOM_RATE:
            pApp->m_fit_mode = WinApp::FIT_NONE;
            pApp->onZoomRate();
            break;
        case IDM_NEXT_PAGE:
            pApp->onNextPage();
            break;
        case IDM_PREV_PAGE:
            pApp->onPrevPage();
            break;
        case IDM_GO_TO_PAGE:
            pApp->onGoToPage();
            break;
        case IDM_PAGEDOWN:
            ::SendMessage(pApp->m_hRealClientWnd,
                WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
            break;
        case IDM_PAGEUP:
            ::SendMessage(pApp->m_hRealClientWnd,
                WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
            break;
        case IDM_GO_TO_HOME:
            ::SendMessage(pApp->m_hRealClientWnd,
                WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), 0);
            break;
        case IDM_GO_TO_END:
            ::SendMessage(pApp->m_hRealClientWnd,
                WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
            break;
        case IDM_SEG_COLOR_AUTO:
            pApp->m_seg_color = SC_AUTO;
            pApp->updateClientImage();
            break;
        case IDM_SEG_COLOR_RED:
            pApp->m_seg_color = SC_RED;
            pApp->updateClientImage();
            break;
        case IDM_SEG_COLOR_GREEN:
            pApp->m_seg_color = SC_GREEN;
            pApp->updateClientImage();
            break;
        case IDM_SEG_COLOR_BLUE:
            pApp->m_seg_color = SC_BLUE;
            pApp->updateClientImage();
            break;
        case IDM_RECENT_0:
            pApp->loadRecent(0);
            break;
        case IDM_RECENT_1:
            pApp->loadRecent(1);
            break;
        case IDM_RECENT_2:
            pApp->loadRecent(2);
            break;
        case IDM_RECENT_3:
            pApp->loadRecent(3);
            break;
        case IDM_RECENT_4:
            pApp->loadRecent(4);
            break;
        case IDM_RECENT_5:
            pApp->loadRecent(5);
            break;
        case IDM_RECENT_6:
            pApp->loadRecent(6);
            break;
        case IDM_RECENT_7:
            pApp->loadRecent(7);
            break;
        case IDM_ZOOM_IN:
            pApp->zoomIn();
            break;
        case IDM_ZOOM_OUT:
            pApp->zoomOut();
            break;
        }
        break;
    case WM_ACTIVATE:
        if (pApp != NULL) {
            ::SetFocus(pApp->m_hTaskDialogs[pApp->m_nTaskIndex]);
        }
        break;
    case WM_SIZE:
        if (pApp != NULL) {
            pApp->onSize(UINT(wParam), LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_GETMINMAXINFO:
        if (pApp != NULL) {
            pApp->onGetMinMaxInfo(reinterpret_cast<LPMINMAXINFO>(lParam));
        }
        break;
    case WM_DROPFILES:
        if (pApp != NULL) {
            pApp->onDropFiles(reinterpret_cast<HDROP>(wParam));
        }
        break;
    case WM_DESTROY:
        if (pApp != NULL) {
            pApp->onDestroy();
        }
        break;
    case WM_MOUSEWHEEL:
        if (pApp != NULL) {
            pApp->onMouseWheel(
                hWnd,
                SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam)),
                SHORT(HIWORD(wParam)), UINT(LOWORD(wParam))
            );
        }
        break;
    case WM_INITMENUPOPUP:
        if (pApp != NULL) {
            pApp->onInitMenuPopup(
                reinterpret_cast<HMENU>(wParam),
                BOOL(HIWORD(lParam))
            );
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
} // WindowProc

LRESULT CALLBACK
RealClientWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static WinApp *pApp = NULL;
    LPCREATESTRUCT pcs;

    switch (uMsg) {
    case WM_CREATE:
        pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        assert(pcs != NULL);
        pApp = reinterpret_cast<WinApp *>(pcs->lpCreateParams);
        assert(pApp != NULL);
        pApp->m_hRealClientWnd = hWnd;
        break;

    case WM_ERASEBKGND:
        break;

    case WM_PAINT:
        pApp->onPaint();
        break;

    case WM_DROPFILES:
        pApp->onDropFiles(reinterpret_cast<HDROP>(wParam));
        break;

    case WM_HSCROLL:
        pApp->onHScroll(LOWORD(wParam), SHORT(HIWORD(wParam)));
        break;

    case WM_VSCROLL:
        pApp->onVScroll(LOWORD(wParam), SHORT(HIWORD(wParam)));
        break;

    case WM_LBUTTONDOWN:
        pApp->onLButtonDown(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_MBUTTONDOWN:
        pApp->onMButtonDown(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_RBUTTONDOWN:
        pApp->onRButtonDown(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_LBUTTONUP:
        pApp->onLButtonUp(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_MBUTTONUP:
        pApp->onMButtonUp(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_RBUTTONUP:
        pApp->onRButtonUp(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_MOUSEMOVE:
        pApp->onMouseMove(
            UINT(wParam),
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam))
        );
        break;

    case WM_MOUSEWHEEL:
        pApp->onMouseWheel(
            hWnd,
            SHORT(LOWORD(lParam)), SHORT(HIWORD(lParam)),
            SHORT(HIWORD(wParam)), UINT(LOWORD(wParam))
        );
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

bool WinApp::registerClasses() {
    WNDCLASSEX wcx;

    ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_DBLCLKS;
    wcx.lpfnWndProc = WindowProc;
    wcx.hInstance = m_hInst;
    wcx.hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
    wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
    wcx.lpszMenuName = MAKEINTRESOURCE(1);
    wcx.lpszClassName = s_szClassName;
    wcx.hIconSm = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
    if (!::RegisterClassEx(&wcx)) {
        return false;
    }
    wcx.style = CS_DBLCLKS;
    wcx.lpfnWndProc = RealClientWndProc;
    wcx.hInstance = m_hInst;
    wcx.hIcon = NULL;
    wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(NULL_BRUSH));
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = s_szRealClientWndClassName;
    wcx.hIconSm = NULL;
    if (!::RegisterClassEx(&wcx)) {
        return false;
    }
    return true;
} // WinApp::registerClasses

// Win32 App main function
INT APIENTRY WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    INT         nCmdShow)
{
#ifdef UNICODE
    int argc;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    WinApp app(hInstance, argc, wargv);
#else
    WinApp app(hInstance, __argc, __argv);
#endif

    if (!app.registerClasses()) {
        CenterMessageBox(
            NULL, TEXT("ERROR: RegisterClass failed"), NULL,
            MB_ICONERROR);
        return 1;
    }

    int ret;

    try {
        if (!app.startup(nCmdShow)) {
            ret = 2;
        } else {
            ret = app.run();
        }
    } catch (const std::bad_alloc&) {
        CenterMessageBox(
            NULL, TEXT("ERROR: Out of memory"), NULL,
            MB_ICONERROR);
        ret = -1;
    }

#ifdef _MSC_VER
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef UNICODE
    LocalFree(wargv);
#endif
    return ret;
} // WinMain
