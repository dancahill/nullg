# Microsoft Developer Studio Project File - Name="server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=server - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "server.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "server.mak" CFG="server - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "server - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "obj"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\obj\server"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /nodefaultlib:"libc" /out:"..\..\bin\nullgs.exe"
# SUBTRACT LINK32 /map
# Begin Target

# Name "server - Win32 Release"
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE="..\..\include\nullgs\config-nt.h"
# End Source File
# Begin Source File

SOURCE=..\..\include\nullgs\defines.h
# End Source File
# Begin Source File

SOURCE=..\server\functions.h
# End Source File
# Begin Source File

SOURCE=..\server\main.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ngs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nil.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nullgs\records.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nullgs\ssl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\nullgs\typedefs.h
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\server\config.c
# End Source File
# Begin Source File

SOURCE=..\server\domains.c
# End Source File
# Begin Source File

SOURCE=..\server\format.c
# End Source File
# Begin Source File

SOURCE=..\server\language.c
# End Source File
# Begin Source File

SOURCE=..\server\log.c
# End Source File
# Begin Source File

SOURCE=..\server\main.c
# End Source File
# Begin Source File

SOURCE=..\server\modctl.c
# End Source File
# Begin Source File

SOURCE=..\server\sanity.c
# End Source File
# Begin Source File

SOURCE=..\server\server.c
# End Source File
# Begin Source File

SOURCE=..\server\sql.c
# End Source File
# Begin Source File

SOURCE=..\server\ssl.c
# End Source File
# Begin Source File

SOURCE=..\server\sys.c
# End Source File
# Begin Source File

SOURCE=..\server\tcp.c
# End Source File
# Begin Source File

SOURCE=..\server\win32.c
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*.ico, *.rc"
# Begin Source File

SOURCE=..\..\lib\libnil.lib
# End Source File
# Begin Source File

SOURCE=..\..\lib\libngs.lib
# End Source File
# End Group
# End Target
# End Project
