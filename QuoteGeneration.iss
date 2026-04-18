; ─────────────────────────────────────────────────────────────────────────────
; QuoteGeneration.iss — Inno Setup Installer Script
; PB Software Solutions
; ─────────────────────────────────────────────────────────────────────────────

[Setup]
AppName=Quote Generation
AppVersion=1.0.0
AppPublisher=PB Software Solutions
AppPublisherURL=https://www.pbsoftwaresolutions.com
AppSupportURL=https://www.pbsoftwaresolutions.com
AppUpdatesURL=https://www.pbsoftwaresolutions.com
DefaultDirName={autopf}\PBSoftwareSolutions\QuoteGeneration
DefaultGroupName=PB Software Solutions\Quote Generation
AllowNoIcons=yes
OutputDir=D:\Qt_Projects\QuoteGeneration\installer
OutputBaseFilename=QuoteGeneration_Setup_v1.0.0
SetupIconFile=D:\Qt_Projects\QuoteGeneration\resources\logos\DefaultLogo.png
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable
Source: "D:\Qt_Projects\QuoteGeneration\deploy\QuoteGeneration.exe"; DestDir: "{app}"; Flags: ignoreversion

; Qt core DLLs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6Core.dll";         DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6Gui.dll";          DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6Widgets.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6Sql.dll";          DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6Network.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6PrintSupport.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\Qt6Svg.dll";          DestDir: "{app}"; Flags: ignoreversion

; MinGW runtime DLLs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\libgcc_s_seh-1.dll";  DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\libstdc++-6.dll";     DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion

; DirectX and OpenGL
Source: "D:\Qt_Projects\QuoteGeneration\deploy\D3Dcompiler_47.dll";  DestDir: "{app}"; Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\opengl32sw.dll";      DestDir: "{app}"; Flags: ignoreversion

; Qt plugin folders
Source: "D:\Qt_Projects\QuoteGeneration\deploy\platforms\*";         DestDir: "{app}\platforms";         Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\sqldrivers\*";        DestDir: "{app}\sqldrivers";        Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\imageformats\*";      DestDir: "{app}\imageformats";      Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\iconengines\*";       DestDir: "{app}\iconengines";       Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\styles\*";            DestDir: "{app}\styles";            Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\tls\*";               DestDir: "{app}\tls";               Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\translations\*";      DestDir: "{app}\translations";      Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\generic\*";           DestDir: "{app}\generic";           Flags: ignoreversion recursesubdirs
Source: "D:\Qt_Projects\QuoteGeneration\deploy\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs

; Application resources
Source: "D:\Qt_Projects\QuoteGeneration\deploy\resources\logos\DefaultLogo.png";     DestDir: "{app}\resources\logos";   Flags: ignoreversion
Source: "D:\Qt_Projects\QuoteGeneration\deploy\resources\buttons\bluebutton.png";    DestDir: "{app}\resources\buttons"; Flags: ignoreversion

; PDF output folder — empty folder created for saving quotes
Source: "D:\Qt_Projects\QuoteGeneration\deploy\QGenPdfFolder\*"; DestDir: "{app}\QGenPdfFolder"; Flags: ignoreversion recursesubdirs createallsubdirs

[Dirs]
; Ensure PDF output folder exists even if empty
Name: "{app}\QGenPdfFolder"

[Icons]
Name: "{group}\Quote Generation";    Filename: "{app}\QuoteGeneration.exe"
Name: "{group}\Uninstall";           Filename: "{uninstallexe}"
Name: "{autodesktop}\Quote Generation"; Filename: "{app}\QuoteGeneration.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\QuoteGeneration.exe"; Description: "{cm:LaunchProgram,Quote Generation}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove the database and INI file on uninstall.
Type: files; Name: "{app}\QuoteGeneration.sqlite"
Type: files; Name: "{app}\QuoteGeneration.ini"
Type: filesandordirs; Name: "{app}\QGenPdfFolder"