strip karasunpo\karasunpo.exe
candle installer-ja.wxs
light -ext WixUIExtension -cultures:ja-jp installer-ja.wixobj -o karasunpo-ja-1.0-setup.msi
