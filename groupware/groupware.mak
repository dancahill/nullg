# Microsoft Developer Studio Generated NMAKE File, Based on groupware.dsp
!IF "$(CFG)" == ""
CFG=Groupware - Win32 Release
!MESSAGE No configuration specified. Defaulting to Groupware - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "Groupware - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "groupware.mak" CFG="Groupware - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Groupware - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
OUTDIR=.\..\distrib\bin
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\groupware.exe" ".\obj\incver.txt"


CLEAN :
	-@erase "$(INTDIR)\admin.obj"
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\bookmarks.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calls.obj"
	-@erase "$(INTDIR)\cgi.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\connio.obj"
	-@erase "$(INTDIR)\contacts.obj"
	-@erase "$(INTDIR)\dbio.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\format.obj"
	-@erase "$(INTDIR)\forums.obj"
	-@erase "$(INTDIR)\groupware.res"
	-@erase "$(INTDIR)\html.obj"
	-@erase "$(INTDIR)\http.obj"
	-@erase "$(INTDIR)\logging.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\messages.obj"
	-@erase "$(INTDIR)\notes.obj"
	-@erase "$(INTDIR)\orders.obj"
	-@erase "$(INTDIR)\products.obj"
	-@erase "$(INTDIR)\profile.obj"
	-@erase "$(INTDIR)\sanity.obj"
	-@erase "$(INTDIR)\searches.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\sql.obj"
	-@erase "$(INTDIR)\tasks.obj"
	-@erase "$(INTDIR)\time.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\wmcodec.obj"
	-@erase "$(INTDIR)\wmmain.obj"
	-@erase "$(INTDIR)\wmserver.obj"
	-@erase "$(INTDIR)\xml-rpc.obj"
	-@erase "$(OUTDIR)\groupware.exe"
	-@erase ".\obj\incver.txt"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\groupware.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\groupware.exe" /libpath:"../win/lib" 
LINK32_OBJS= \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\cgi.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\connio.obj" \
	"$(INTDIR)\dbio.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\format.obj" \
	"$(INTDIR)\html.obj" \
	"$(INTDIR)\http.obj" \
	"$(INTDIR)\logging.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\sanity.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\sql.obj" \
	"$(INTDIR)\time.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\admin.obj" \
	"$(INTDIR)\bookmarks.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\calls.obj" \
	"$(INTDIR)\contacts.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\forums.obj" \
	"$(INTDIR)\messages.obj" \
	"$(INTDIR)\notes.obj" \
	"$(INTDIR)\orders.obj" \
	"$(INTDIR)\products.obj" \
	"$(INTDIR)\profile.obj" \
	"$(INTDIR)\searches.obj" \
	"$(INTDIR)\tasks.obj" \
	"$(INTDIR)\wmcodec.obj" \
	"$(INTDIR)\wmmain.obj" \
	"$(INTDIR)\wmserver.obj" \
	"$(INTDIR)\xml-rpc.obj" \
	"$(INTDIR)\groupware.res"

"$(OUTDIR)\groupware.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

IntDir=.\obj
InputPath=..\distrib\bin\groupware.exe
SOURCE="$(InputPath)"

".\obj\incver.txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	incver.exe
<< 
	
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "./include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\groupware.res" /d "NDEBUG" 

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("groupware.dep")
!INCLUDE "groupware.dep"
!ELSE 
!MESSAGE Warning: cannot find "groupware.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Groupware - Win32 Release"
SOURCE=.\base\auth.c

"$(INTDIR)\auth.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\cgi.c

"$(INTDIR)\cgi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\config.c

"$(INTDIR)\config.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\connio.c

"$(INTDIR)\connio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\dbio.c

"$(INTDIR)\dbio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\fileio.c

"$(INTDIR)\fileio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\format.c

"$(INTDIR)\format.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\html.c

"$(INTDIR)\html.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\http.c

"$(INTDIR)\http.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\logging.c

"$(INTDIR)\logging.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\sanity.c

"$(INTDIR)\sanity.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\server.c

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\sql.c

"$(INTDIR)\sql.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\time.c

"$(INTDIR)\time.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\base\win32.c

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\admin.c

"$(INTDIR)\admin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\bookmarks.c

"$(INTDIR)\bookmarks.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\calendar.c

"$(INTDIR)\calendar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\calls.c

"$(INTDIR)\calls.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\contacts.c

"$(INTDIR)\contacts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\files.c

"$(INTDIR)\files.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\forums.c

"$(INTDIR)\forums.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\messages.c

"$(INTDIR)\messages.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\notes.c

"$(INTDIR)\notes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\orders.c

"$(INTDIR)\orders.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\products.c

"$(INTDIR)\products.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\profile.c

"$(INTDIR)\profile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\searches.c

"$(INTDIR)\searches.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\tasks.c

"$(INTDIR)\tasks.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\wmcodec.c

"$(INTDIR)\wmcodec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\wmmain.c

"$(INTDIR)\wmmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mods\wmserver.c

"$(INTDIR)\wmserver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\mods\xml-rpc.c"

"$(INTDIR)\xml-rpc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\winres\groupware.rc

"$(INTDIR)\groupware.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\groupware.res" /i "winres" /d "NDEBUG" $(SOURCE)



!ENDIF 

