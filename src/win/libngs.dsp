# Microsoft Developer Studio Project File - Name="libngs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libngs - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libngs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libngs.mak" CFG="libngs - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libngs - Win32 Release" (based on "Win32 (x86) Static Library")
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
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../lib"
# PROP Intermediate_Dir "../../obj/libngs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Target

# Name "libngs - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\libngs\config.c
# End Source File
# Begin Source File

SOURCE=..\libngs\dns.c
# End Source File
# Begin Source File

SOURCE=..\libngs\format.c
# End Source File
# Begin Source File

SOURCE=..\libngs\lib.c
# End Source File
# Begin Source File

SOURCE=..\libngs\md5.c
# End Source File
# Begin Source File

SOURCE=..\libngs\sql.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\libngs\libngs.h
# End Source File
# Begin Source File

SOURCE="..\libngs\mysql-5_0.h"
# End Source File
# Begin Source File

SOURCE=..\libngs\mysql.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ngs.h
# End Source File
# Begin Source File

SOURCE="..\libngs\pgsql-8_1.h"
# End Source File
# Begin Source File

SOURCE=..\libngs\pgsql.h
# End Source File
# Begin Source File

SOURCE=..\libngs\sql.h
# End Source File
# Begin Source File

SOURCE=..\libngs\sqlite2.h
# End Source File
# Begin Source File

SOURCE=..\libngs\sqlite3.h
# End Source File
# End Group
# End Target
# End Project
