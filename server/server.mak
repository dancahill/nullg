# Microsoft Developer Studio Generated NMAKE File, Based on server.dsp
!IF "$(CFG)" == ""
CFG=server - Win32 Release
!MESSAGE server - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "server - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "server.mak" CFG="server - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "server - Win32 Release" (based on "Win32 (x86) Application")
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
INTDIR=.\..\obj\server
# Begin Custom Macros
OutDir=.\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\groupware.exe"


CLEAN :
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\format.obj"
	-@erase "$(INTDIR)\html.obj"
	-@erase "$(INTDIR)\http.obj"
	-@erase "$(INTDIR)\io.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\modctl.obj"
	-@erase "$(INTDIR)\sanity.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.res"
	-@erase "$(INTDIR)\sql.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(OUTDIR)\groupware.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\server.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\groupware.exe" 
LINK32_OBJS= \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\format.obj" \
	"$(INTDIR)\html.obj" \
	"$(INTDIR)\http.obj" \
	"$(INTDIR)\io.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\modctl.obj" \
	"$(INTDIR)\sanity.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\sql.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\server.res"

"$(OUTDIR)\groupware.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\server.res" /d "NDEBUG" 


!IF "$(CFG)" == "server - Win32 Release"
SOURCE=.\auth.c

"$(INTDIR)\auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\config.c

"$(INTDIR)\config.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\format.c

"$(INTDIR)\format.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\html.c

"$(INTDIR)\html.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\http.c

"$(INTDIR)\http.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\io.c

"$(INTDIR)\io.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\log.c

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\modctl.c

"$(INTDIR)\modctl.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sanity.c

"$(INTDIR)\sanity.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\server.c

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sql.c

"$(INTDIR)\sql.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\win32.c

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\server.rc

"$(INTDIR)\server.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

