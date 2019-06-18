; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2A516438-9675-40B5-AE5B-D94756F3CABE}
AppName=画像カラスンポ
AppVerName=画像カラスンポ 0.7
AppPublisher=片山博文MZ
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\karasunpo64
DefaultGroupName=画像カラスンポ 64ビット
AllowNoIcons=yes
LicenseFile=..\License.txt
OutputDir=.
OutputBaseFilename=karasunpo64_ja-0.7-setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\karasunpo.exe
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\ReadMeJP.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\karasunpo.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\katahiromz_pdfium\x64\pdfium.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libgcc_s_seh-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libgif-7.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libjpeg-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\liblzma-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libpng16-16.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libstdc++-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libtiff-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libwinpthread-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\libzstd.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x64\zlib1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{app}\画像カラスンポ"; Filename: "{app}\bin\karasunpo.exe"
Name: "{group}\画像カラスンポ"; Filename: "{app}\bin\karasunpo.exe"
Name: "{group}\ReadMeJP.txt"; Filename: "{app}\ReadMeJP.txt"
Name: "{group}\License.txt"; Filename: "{app}\License.txt"
Name: "{group}\作者のホームページ"; Filename: "http://katahiromz.web.fc2.com/"
Name: "{group}\{cm:UninstallProgram,画像カラスンポ}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\画像カラスンポ"; Filename: "{app}\bin\karasunpo.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\karasunpo.exe"; Description: "{cm:LaunchProgram,画像カラスンポ}"; Flags: nowait postinstall skipifsilent
