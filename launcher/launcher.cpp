#include <windows.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    INT argc;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);


    WCHAR szPath[MAX_PATH];
    ::GetModuleFileNameW(NULL, szPath, MAX_PATH);
    ::PathRemoveFileSpecW(szPath);
    ::PathAppendW(szPath, L"bin\\karasunpo.exe");

    if (argc >= 2)
        ::ShellExecuteW(NULL, NULL, szPath, wargv[1], NULL, nCmdShow);
    else
        ::ShellExecuteW(NULL, NULL, szPath, NULL, NULL, nCmdShow);
    return 0;
}
