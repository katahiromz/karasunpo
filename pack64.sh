#!/bin/bash
if [ ! -e launcher/launcher.exe ]; then
	echo You need launcher/launcher.exe
	exit 1
fi

for i in en ja; do
	MYDIR=karasunpo64$i-0.8

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
	cp -f katahiromz_pdfium/x64/pdfium.dll $MYDIR/bin
	cp -f ../x64/libgcc_s_seh-1.dll $MYDIR/bin
	cp -f ../x64/libgif-7.dll $MYDIR/bin
	cp -f ../x64/libjpeg-8.dll $MYDIR/bin
	cp -f ../x64/liblzma-5.dll $MYDIR/bin
	cp -f ../x64/libpng16-16.dll $MYDIR/bin
	cp -f ../x64/libstdc++-6.dll $MYDIR/bin
	cp -f ../x64/libtiff-5.dll $MYDIR/bin
	cp -f ../x64/libwinpthread-1.dll $MYDIR/bin
	cp -f ../x64/libzstd.dll $MYDIR/bin
	cp -f ../x64/zlib1.dll $MYDIR/bin
done
