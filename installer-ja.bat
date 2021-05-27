strip karasunpo\karasunpo.exe
candle installer-ja.wxs
light -ext WixUIExtension -cultures:ja-jp installer-ja.wixobj -o karasunpo-pro-ja-1.1-setup.msi
