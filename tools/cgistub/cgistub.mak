# Microsoft Developer Studio Generated NMAKE File, Based on cgistub.dsp
!IF "$(CFG)" == ""
CFG=cgistub - Win32 Release
!MESSAGE cgistub - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "cgistub - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cgistub.mak" CFG="cgistub - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cgistub - Win32 Release" (based on "Win32 (x86) Console Application")
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
INTDIR=.\..\obj\cgistub
# Begin Custom Macros
OutDir=.\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\cgistub.exe"


CLEAN :
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cgistub.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cgistub.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\cgistub.exe" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\cgistub.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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


!IF "$(CFG)" == "cgistub - Win32 Release"
SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

