# Microsoft Developer Studio Generated NMAKE File, Based on mod_messages.dsp
!IF "$(CFG)" == ""
CFG=mod_messages - Win32 Release
!MESSAGE mod_messages - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_messages - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_messages.mak" CFG="mod_messages - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_messages - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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
OUTDIR=.\..\..\obj\mod_messages
INTDIR=.\..\..\obj\mod_messages

ALL : "..\..\distrib\lib\mod_messages.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_messages.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_messages.exp"
	-@erase "$(OUTDIR)\mod_messages.lib"
	-@erase "..\..\distrib\lib\mod_messages.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_messages.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\mod_messages.pdb" /machine:I386 /def:".\mod_messages.def" /out:"..\..\distrib\lib\mod_messages.dll" /implib:"$(OUTDIR)\mod_messages.lib" 
DEF_FILE= \
	".\mod_messages.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_messages.obj"

"..\..\distrib\lib\mod_messages.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "mod_messages - Win32 Release"
SOURCE=.\mod_messages.c

"$(INTDIR)\mod_messages.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 
