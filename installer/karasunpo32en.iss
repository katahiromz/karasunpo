; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2A516438-9675-40B5-AE5B-D94757F3CABE}
AppName=karasunpo
AppVerName=karasunpo 0.8
AppPublisher=Katayama Hirofumi MZ
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\karasunpo
DefaultGroupName=karasunpo
AllowNoIcons=yes
LicenseFile=..\License.txt
OutputDir=.
OutputBaseFilename=karasunpo32en-0.8-setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\bin\karasunpo.exe

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\ReadMe.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\launcher\launcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\karasunpo.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\katahiromz_pdfium\x86\pdfium.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libgcc_s_dw2-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libgif-7.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libjpeg-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\liblzma-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libpng16-16.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libstdc++-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libtiff-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libwinpthread-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\libzstd.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\x86\zlib1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{app}\Karasunpo"; Filename: "{app}\bin\karasunpo.exe"
Name: "{group}\karasunpo"; Filename: "{app}\bin\karasunpo.exe"
Name: "{group}\ReadMe.txt"; Filename: "{app}\ReadMe.txt"
Name: "{group}\License.txt"; Filename: "{app}\License.txt"
Name: "{group}\{cm:ProgramOnTheWeb,karasunpo}"; Filename: "http://katahiromz.web.fc2.com/"
Name: "{group}\{cm:UninstallProgram,karasunpo}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\karasunpo"; Filename: "{app}\bin\karasunpo.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\karasunpo.exe"; Description: "{cm:LaunchProgram,karasunpo}"; Flags: nowait postinstall skipifsilent
