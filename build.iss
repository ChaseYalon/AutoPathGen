[Setup]
AppName=Auto Path Generator
AppVersion=1.0.0
DefaultDirName={pf}\AutoPathGen
DefaultGroupName=Auto Path Generator
OutputBaseFilename=AutoPathGenInstaller
Compression=lzma
SolidCompression=yes
SetupIconFile=assets\icon.ico
UninstallDisplayIcon={app}\auto_path_gen.exe
AppPublisher=Chase Yalon
AppPublisherURL=chaseyalon.com

[Files]
Source: "build\auto_path_gen.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Auto Path Generator"; Filename: "{app}\auto_path_gen.exe"; IconFilename: "{app}\assets\icon.png"
Name: "{commondesktop}\Auto Path Generator"; Filename: "{app}\auto_path_gen.exe"; IconFilename: "{app}\assets\icon.png"

[Run]
Filename: "{app}\auto_path_gen.exe"; Description: "Launch Auto Path Generator"; Flags: nowait postinstall skipifsilent
