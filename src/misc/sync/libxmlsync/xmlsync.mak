# Microsoft Developer Studio Generated NMAKE File, Based on xmlsync.dsp
!IF "$(CFG)" == ""
CFG=xmlsync - Win32 Release
!MESSAGE No configuration specified. Defaulting to xmlsync - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "xmlsync - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xmlsync.mak" CFG="xmlsync - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xmlsync - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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
OUTDIR=.\..\bin
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\xmlsync.dll"


CLEAN :
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xmlsync.obj"
	-@erase "$(OUTDIR)\xmlsync.dll"
	-@erase "$(OUTDIR)\xmlsync.exp"
	-@erase "$(OUTDIR)\xmlsync.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xmlsync.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\xmlsync.pdb" /machine:I386 /def:".\xmlsync.def" /out:"$(OUTDIR)\xmlsync.dll" /implib:"$(OUTDIR)\xmlsync.lib" 
DEF_FILE= \
	".\xmlsync.def"
LINK32_OBJS= \
	"$(INTDIR)\xmlsync.obj"

"$(OUTDIR)\xmlsync.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../include" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("xmlsync.dep")
!INCLUDE "xmlsync.dep"
!ELSE 
!MESSAGE Warning: cannot find "xmlsync.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "xmlsync - Win32 Release"
SOURCE=.\xmlsync.c

"$(INTDIR)\xmlsync.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

