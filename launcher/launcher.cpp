#include <windows.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    WCHAR szPath[MAX_PATH];

    ::GetModuleFileNameW(NULL, szPath, MAX_PATH);
    ::PathRemoveFileSpecW(szPath);
    ::PathAppendW(szPath, L"bin\\karasunpo.exe");

    ::ShellExecuteW(NULL, NULL, szPath, NULL, NULL, nCmdShow);
    return 0;
}
