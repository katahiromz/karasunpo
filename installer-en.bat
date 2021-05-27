strip karasunpo\karasunpo.exe
candle installer-en.wxs
light -ext WixUIExtension -cultures:en-us installer-en.wixobj -o karasunpo-en-1.1-setup.msi
