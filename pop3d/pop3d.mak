# Microsoft Developer Studio Generated NMAKE File, Based on pop3d.dsp
!IF "$(CFG)" == ""
CFG=pop3d - Win32 Release
!MESSAGE pop3d - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "pop3d - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pop3d.mak" CFG="pop3d - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pop3d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe
OUTDIR=.\..\distrib\bin
INTDIR=.\..\obj\pop3d
# Begin Custom Macros
OutDir=.\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\nullgw-pop3d.exe"


CLEAN :
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\format.obj"
	-@erase "$(INTDIR)\io.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\sanity.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\sql.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(OUTDIR)\nullgw-pop3d.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pop3d.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\nullgw-pop3d.exe" 
LINK32_OBJS= \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\format.obj" \
	"$(INTDIR)\io.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\sanity.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\sql.obj" \
	"$(INTDIR)\win32.obj"

"$(OUTDIR)\nullgw-pop3d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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


!IF "$(CFG)" == "pop3d - Win32 Release"
SOURCE=.\auth.c

"$(INTDIR)\auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\config.c

"$(INTDIR)\config.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\format.c

"$(INTDIR)\format.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\io.c

"$(INTDIR)\io.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\log.c

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pop3.c

"$(INTDIR)\pop3.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sanity.c

"$(INTDIR)\sanity.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\server.c

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sql.c

"$(INTDIR)\sql.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\win32.c

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

