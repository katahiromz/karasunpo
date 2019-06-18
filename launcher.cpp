//////////////////////////////////////////////////////////////////////////////
// launcher.cpp --- karasunpo launcher
// Copyright (C) 2019 Katayama Hirofumi MZ. All Rights Reserved.
// See License.txt about licensing.
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <shlwapi.h>
#include <cstring>

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    WCHAR szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
    LPWSTR pch = wcsrchr(szPath, L'\\');
    if (!pch)
        pch = wcsrchr(szPath, L'/');
    if (pch)
    {
        *pch = 0;
        PathAppendW(szPath, L"bin\\karasunpo.exe");
        ShellExecuteW(NULL, NULL, szPath, NULL, NULL, nCmdShow);
        return 0;
    }
    return 1;
}
