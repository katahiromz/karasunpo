if not exist launcher\launcher.exe goto need_launcher_exe

set MYDIR=karasunpo-3.8ja
if not exist %MYDIR% mkdir %MYDIR%

copy /Y ReadMeJP.txt %MYDIR%
copy /Y License.txt %MYDIR%
copy /Y launcher\launcher.exe %MYDIR%
copy /Y karasunpo.exe %MYDIR%\bin
copy /Y katahiromz_pdfium\x86\pdfium.dll %MYDIR%\bin
copy /Y ..\x86\libgcc_s_dw2-1.dll %MYDIR%\bin
copy /Y ..\x86\libgif-7.dll %MYDIR%\bin
copy /Y ..\x86\libjpeg-8.dll %MYDIR%\bin
copy /Y ..\x86\liblzma-5.dll %MYDIR%\bin
copy /Y ..\x86\libpng16-16.dll %MYDIR%\bin
copy /Y ..\x86\libstdc++-6.dll %MYDIR%\bin
copy /Y ..\x86\libtiff-5.dll %MYDIR%\bin
copy /Y ..\x86\libwinpthread-1.dll %MYDIR%\bin
copy /Y ..\x86\libzstd.dll %MYDIR%\bin
copy /Y ..\x86\zlib1.dll %MYDIR%\bin

exit 0

:need_launcher_exe
echo You need launcher\launcher.exe
exit 1
