# Microsoft Developer Studio Generated NMAKE File, Based on gsmon.dsp
!IF "$(CFG)" == ""
CFG=gsmon - Win32 Release
!MESSAGE gsmon - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "gsmon - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gsmon.mak" CFG="gsmon - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gsmon - Win32 Release" (based on "Win32 (x86) Application")
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
OUTDIR=.\..\..\..\bin
INTDIR=.\..\..\..\obj\gsmon
# Begin Custom Macros
OutDir=.\..\..\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\gsmon.exe"


CLEAN :
	-@erase "$(INTDIR)\gsmon.obj"
	-@erase "$(INTDIR)\gsmon.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gsmon.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gsmon.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\gsmon.exe" 
LINK32_OBJS= \
	"$(INTDIR)\gsmon.obj" \
	"$(INTDIR)\gsmon.res" \
	"..\..\..\lib\libnsp.lib"

"$(OUTDIR)\gsmon.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\gsmon.res" /d "NDEBUG" 


!IF "$(CFG)" == "gsmon - Win32 Release"
SOURCE=.\gsmon.c

"$(INTDIR)\gsmon.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gsmon.rc

"$(INTDIR)\gsmon.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

