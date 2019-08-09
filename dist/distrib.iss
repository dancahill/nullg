[Setup]
OutputBaseFileName=nullg-current
VersionInfoVersion=2.0.0
AppVerName=NullLogic GroupServer 2.0.0-PreAlpha
AppName=NullLogic GroupServer
AppPublisher=NullLogic
AppPublisherURL=http://nullgroupware.sourceforge.net/
AppSupportURL=http://nullgroupware.sourceforge.net/
AppUpdatesURL=http://nullgroupware.sourceforge.net/
;DefaultDirName={pf}\NullLogic GroupServer
DefaultDirName={sd}\nullg
DefaultGroupName=NullLogic GroupServer
LicenseFile=..\doc\COPYRIGHT
InfoAfterFile=..\doc\README
OutputDir=.\

[Files]
Source: "..\bin\*.exe";        Flags: ignoreversion recursesubdirs; DestDir: "{app}\bin"
Source: "..\etc\*";            Flags: ignoreversion recursesubdirs; DestDir: "{app}\etc"
Source: "..\lib\*.lib";        Flags: ignoreversion;                DestDir: "{app}\lib"
Source: "lib\*.dll";           Flags: ignoreversion;                DestDir: "{app}\lib"
Source: "..\lib\core\*.dll";   Flags: ignoreversion;                DestDir: "{app}\lib\core"
Source: "..\lib\htdocs\*";     Flags: ignoreversion recursesubdirs; DestDir: "{app}\lib\htdocs"
Source: "..\lib\locale\*";     Flags: ignoreversion recursesubdirs; DestDir: "{app}\lib\locale"
Source: "..\lib\scripts\*";    Flags: ignoreversion recursesubdirs; DestDir: "{app}\lib\scripts"
Source: "..\lib\shared\*.dll"; Flags: ignoreversion;                DestDir: "{app}\lib\nsp"
Source: "..\var\*";            Flags: ignoreversion recursesubdirs createallsubdirs; DestDir: "{app}\var"
[Icons]
Name: "{group}\GroupServer Configuration"; Workingdir: "{app}\bin"; Filename: "{app}\bin\nullg-config.exe"
Name: "{group}\GroupServer Monitor"; Workingdir: "{app}\bin"; Filename: "{app}\bin\gsmon.exe"
Name: "{group}\GroupServer Help"; Filename: "{app}\var\share\htdocs\nullg\help\en\index.html"
Name: "{group}\GroupServer Online"; Filename: "http://nullgroupware.sourceforge.net/"
Name: "{commonstartup}\GroupServer Monitor"; Workingdir: "{app}\bin"; Filename: "{app}\bin\gsmon.exe"
[Run]
Filename: "{app}\bin\nullg-config.exe"; Workingdir: "{app}\bin"; Description: "Edit GroupServer Configuration"; Flags: postinstall skipifsilent unchecked
Filename: "{app}\bin\gsmon.exe"; Workingdir: "{app}\bin"; Description: "Launch GroupServer Monitor"; Flags: nowait postinstall skipifsilent unchecked
[UninstallRun]
Filename: "net.exe"; Parameters: "stop nullg"; Flags: runminimized
Filename: "{app}\bin\nullg.exe"; Parameters: "remove"; Workingdir: "{app}\bin"; Flags: runminimized
