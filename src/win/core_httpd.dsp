# Microsoft Developer Studio Project File - Name="core_httpd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=core_httpd - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "core_httpd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core_httpd.mak" CFG="core_httpd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core_httpd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\obj\modules\core\httpd"
# PROP Intermediate_Dir "..\..\..\..\obj\modules\core\httpd"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /machine:I386 /out:"..\..\..\..\lib\core\httpd.dll"
# Begin Target

# Name "core_httpd - Win32 Release"
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_auth.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_conf.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_format.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_html.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_http.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_io.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_lang.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_log.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_main.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_modctl.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_nil.c
# End Source File
# Begin Source File

SOURCE=..\modules\core\httpd\httpd_sql.c
# End Source File
# End Target
# End Project