# Microsoft Developer Studio Generated NMAKE File, Based on dbutil.dsp
!IF "$(CFG)" == ""
CFG=dbutil - Win32 Release
!MESSAGE dbutil - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "dbutil - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbutil.mak" CFG="dbutil - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbutil - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\distrib\bin
INTDIR=.\..\obj\dbutil
# Begin Custom Macros
OutDir=.\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\dbutil.exe"


CLEAN :
	-@erase "$(INTDIR)\dbio.obj"
	-@erase "$(INTDIR)\dbutil.res"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\sql.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dbutil.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
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

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dbutil.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbutil.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\dbutil.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dbio.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\sql.obj" \
	"$(INTDIR)\dbutil.res"

"$(OUTDIR)\dbutil.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "dbutil - Win32 Release"
SOURCE=.\dbio.c

"$(INTDIR)\dbio.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dbutil.rc

"$(INTDIR)\dbutil.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sql.c

"$(INTDIR)\sql.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

