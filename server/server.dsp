# Microsoft Developer Studio Project File - Name="server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

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
!MESSAGE "server - Win32 Release" (based on "Win32 (x86) Application")
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
# PROP BASE Intermediate_Dir "obj"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\distrib\bin"
# PROP Intermediate_Dir "..\obj\server"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc" /out:"..\distrib\bin\groupware.exe"
# SUBTRACT LINK32 /map
# Begin Target

# Name "server - Win32 Release"
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\include\defines.h
# End Source File
# Begin Source File

SOURCE=..\include\functions.h
# End Source File
# Begin Source File

SOURCE="..\include\language-en.h"
# End Source File
# Begin Source File

SOURCE=..\include\main.h
# End Source File
# Begin Source File

SOURCE=..\include\mysql.h
# End Source File
# Begin Source File

SOURCE=..\include\pgsql.h
# End Source File
# Begin Source File

SOURCE=..\include\sqlite.h
# End Source File
# Begin Source File

SOURCE=..\include\typedefs.h
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\auth.c
# End Source File
# Begin Source File

SOURCE=.\config.c
# End Source File
# Begin Source File

SOURCE=.\format.c
# End Source File
# Begin Source File

SOURCE=.\html.c
# End Source File
# Begin Source File

SOURCE=.\http.c
# End Source File
# Begin Source File

SOURCE=.\io.c
# End Source File
# Begin Source File

SOURCE=.\log.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\md5.c
# End Source File
# Begin Source File

SOURCE=.\modctl.c
# End Source File
# Begin Source File

SOURCE=.\sanity.c
# End Source File
# Begin Source File

SOURCE=.\server.c
# End Source File
# Begin Source File

SOURCE=.\sql.c
# End Source File
# Begin Source File

SOURCE=.\win32.c
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*.ico, *.rc"
# Begin Source File

SOURCE=.\icons\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icons\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\server.rc
# End Source File
# End Group
# End Target
# End Project
