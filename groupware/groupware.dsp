# Microsoft Developer Studio Project File - Name="Groupware" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Groupware - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "groupware.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "groupware.mak" CFG="Groupware - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Groupware - Win32 Release" (based on "Win32 (x86) Application")
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
# PROP Intermediate_Dir "obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "./include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc" /libpath:"../win/lib"
# SUBTRACT LINK32 /map
# Begin Custom Build - Incrementing Build Number
IntDir=.\obj
InputPath=\null\nullgroupware\gw-2002-10-18\source\distrib\bin\groupware.exe
SOURCE="$(InputPath)"

"$(IntDir)\incver.txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	incver.exe

# End Custom Build
# Begin Target

# Name "Groupware - Win32 Release"
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=".\include\language-en.h"
# End Source File
# Begin Source File

SOURCE=.\include\main.h
# End Source File
# Begin Source File

SOURCE=.\include\resource.h
# End Source File
# Begin Source File

SOURCE=.\include\version.h
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter "*.c"
# Begin Group "Base"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\base\auth.c
# End Source File
# Begin Source File

SOURCE=.\base\cgi.c
# End Source File
# Begin Source File

SOURCE=.\base\config.c
# End Source File
# Begin Source File

SOURCE=.\base\connio.c
# End Source File
# Begin Source File

SOURCE=.\base\dbio.c
# End Source File
# Begin Source File

SOURCE=.\base\fileio.c
# End Source File
# Begin Source File

SOURCE=.\base\format.c
# End Source File
# Begin Source File

SOURCE=.\base\html.c
# End Source File
# Begin Source File

SOURCE=.\base\http.c
# End Source File
# Begin Source File

SOURCE=.\base\logging.c
# End Source File
# Begin Source File

SOURCE=.\base\main.c
# End Source File
# Begin Source File

SOURCE=.\base\md5.c
# End Source File
# Begin Source File

SOURCE=.\base\sanity.c
# End Source File
# Begin Source File

SOURCE=.\base\server.c
# End Source File
# Begin Source File

SOURCE=.\base\sql.c
# End Source File
# Begin Source File

SOURCE=.\base\time.c
# End Source File
# Begin Source File

SOURCE=.\base\win32.c
# End Source File
# End Group
# Begin Group "Modules"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\mods\admin.c
# End Source File
# Begin Source File

SOURCE=.\mods\bookmarks.c
# End Source File
# Begin Source File

SOURCE=.\mods\calendar.c
# End Source File
# Begin Source File

SOURCE=.\mods\calls.c
# End Source File
# Begin Source File

SOURCE=.\mods\contacts.c
# End Source File
# Begin Source File

SOURCE=.\mods\files.c
# End Source File
# Begin Source File

SOURCE=.\mods\forums.c
# End Source File
# Begin Source File

SOURCE=.\mods\messages.c
# End Source File
# Begin Source File

SOURCE=.\mods\notes.c
# End Source File
# Begin Source File

SOURCE=.\mods\orders.c
# End Source File
# Begin Source File

SOURCE=.\mods\products.c
# End Source File
# Begin Source File

SOURCE=.\mods\profile.c
# End Source File
# Begin Source File

SOURCE=.\mods\searches.c
# End Source File
# Begin Source File

SOURCE=.\mods\tasks.c
# End Source File
# Begin Source File

SOURCE=.\mods\wmcodec.c
# End Source File
# Begin Source File

SOURCE=.\mods\wmmain.c
# End Source File
# Begin Source File

SOURCE=.\mods\wmserver.c
# End Source File
# Begin Source File

SOURCE=".\mods\xml-rpc.c"
# End Source File
# End Group
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*.ico, *.rc"
# Begin Source File

SOURCE=.\winres\groupware.rc
# End Source File
# Begin Source File

SOURCE=.\winres\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\winres\icon2.ico
# End Source File
# End Group
# End Target
# End Project
