# Microsoft Developer Studio Generated NMAKE File, Based on mod_cgi.dsp
!IF "$(CFG)" == ""
CFG=mod_cgi - Win32 Release
!MESSAGE mod_cgi - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_cgi - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_cgi.mak" CFG="mod_cgi - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_cgi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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
OUTDIR=.\..\..\obj\mod_cgi
INTDIR=.\..\..\obj\mod_cgi

ALL : "..\..\distrib\lib\mod_cgi.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_cgi.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_cgi.exp"
	-@erase "$(OUTDIR)\mod_cgi.lib"
	-@erase "..\..\distrib\lib\mod_cgi.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_cgi.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\mod_cgi.pdb" /machine:I386 /def:".\mod_cgi.def" /out:"..\..\distrib\lib\mod_cgi.dll" /implib:"$(OUTDIR)\mod_cgi.lib" 
DEF_FILE= \
	".\mod_cgi.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_cgi.obj"

"..\..\distrib\lib\mod_cgi.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../include" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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


!IF "$(CFG)" == "mod_cgi - Win32 Release"
SOURCE=.\mod_cgi.c

"$(INTDIR)\mod_cgi.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

