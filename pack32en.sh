#!/bin/bash
if [ ! -e launcher/launcher.exe ]; then
	echo You need launcher/launcher.exe
	exit 1
fi

MYDIR=karasunpo32-3.8en

if [ ! -e $MYDIR ]; then
	mkdir $MYDIR
fi
if [ ! -e $MYDIR/bin ]; then
	mkdir $MYDIR/bin
fi

cp -f ReadMe.txt $MYDIR
cp -f License.txt $MYDIR
cp -f launcher/launcher.exe $MYDIR
cp -f karasunpo.exe $MYDIR/bin
cp -f katahiromz_pdfium/x86/pdfium.dll $MYDIR/bin
cp -f ../x86/libgcc_s_dw2-1.dll $MYDIR/bin
cp -f ../x86/libgif-7.dll $MYDIR/bin
cp -f ../x86/libjpeg-8.dll $MYDIR/bin
cp -f ../x86/liblzma-5.dll $MYDIR/bin
cp -f ../x86/libpng16-16.dll $MYDIR/bin
cp -f ../x86/libstdc++-6.dll $MYDIR/bin
cp -f ../x86/libtiff-5.dll $MYDIR/bin
cp -f ../x86/libwinpthread-1.dll $MYDIR/bin
cp -f ../x86/libzstd.dll $MYDIR/bin
cp -f ../x86/zlib1.dll $MYDIR/bin
