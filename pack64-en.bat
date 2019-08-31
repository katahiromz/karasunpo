if not exist launcher\launcher.exe goto need_launcher_exe

set MYDIR=karasunpo-3.8en
if not exist %MYDIR% mkdir %MYDIR%

copy /Y ReadMe.txt %MYDIR%
copy /Y License.txt %MYDIR%
copy /Y launcher\launcher.exe %MYDIR%
copy /Y karasunpo.exe %MYDIR%\bin
copy /Y katahiromz_pdfium\x64\pdfium.dll %MYDIR%\bin
copy /Y ..\x64\libgcc_s_seh-1.dll %MYDIR%\bin
copy /Y ..\x64\libgif-7.dll %MYDIR%\bin
copy /Y ..\x64\libjpeg-8.dll %MYDIR%\bin
copy /Y ..\x64\liblzma-5.dll %MYDIR%\bin
copy /Y ..\x64\libpng16-16.dll %MYDIR%\bin
copy /Y ..\x64\libstdc++-6.dll %MYDIR%\bin
copy /Y ..\x64\libtiff-5.dll %MYDIR%\bin
copy /Y ..\x64\libwinpthread-1.dll %MYDIR%\bin
copy /Y ..\x64\libzstd.dll %MYDIR%\bin
copy /Y ..\x64\zlib1.dll %MYDIR%\bin

exit 0

:need_launcher_exe
echo You need launcher\launcher.exe
exit 1
