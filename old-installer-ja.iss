; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2A516438-9675-40B5-AE5B-D94756F3CABD}
AppName=画像カラスンポ
AppVerName=画像カラスンポ 1.1
AppPublisher=片山博文MZ
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\karasunpo
DefaultGroupName=画像カラスンポ
AllowNoIcons=yes
LicenseFile=LICENSE.txt
OutputDir=.
OutputBaseFilename=karasunpo-ja-1.1-setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\karasunpo.exe

[Languages]
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "READMEJP.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "karasunpo/karasunpo.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "katahiromz_pdfium\x86\pdfium.dll"; DestDir: "{app}\x86"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{app}\画像カラスンポ"; Filename: "{app}\karasunpo.exe"
Name: "{group}\画像カラスンポ"; Filename: "{app}\karasunpo.exe"
Name: "{group}\READMEJP.txt"; Filename: "{app}\READMEJP.txt"
Name: "{group}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{group}\作者のホームページ"; Filename: "http://katahiromz.web.fc2.com/"
Name: "{group}\{cm:UninstallProgram,画像カラスンポ}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\画像カラスンポ"; Filename: "{app}\karasunpo.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\karasunpo.exe"; Description: "{cm:LaunchProgram,画像カラスンポ}"; Flags: nowait postinstall skipifsilent
