# Microsoft Developer Studio Generated NMAKE File, Based on mod_mail.dsp
!IF "$(CFG)" == ""
CFG=mod_mail - Win32 Release
!MESSAGE mod_mail - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_mail - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_mail.mak" CFG="mod_mail - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_mail - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\obj\mod_mail
INTDIR=.\..\..\obj\mod_mail

ALL : "..\..\distrib\lib\mod_mail.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_mail_addr.obj"
	-@erase "$(INTDIR)\mod_mail_codec.obj"
	-@erase "$(INTDIR)\mod_mail_folders.obj"
	-@erase "$(INTDIR)\mod_mail_html.obj"
	-@erase "$(INTDIR)\mod_mail_main.obj"
	-@erase "$(INTDIR)\mod_mail_mime.obj"
	-@erase "$(INTDIR)\mod_mail_server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_mail.exp"
	-@erase "$(OUTDIR)\mod_mail.lib"
	-@erase "..\..\distrib\lib\mod_mail.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_mail.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\mod_mail.pdb" /machine:I386 /def:".\mod_mail.def" /out:"..\..\distrib\lib\mod_mail.dll" /implib:"$(OUTDIR)\mod_mail.lib" 
DEF_FILE= \
	".\mod_mail.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_mail_addr.obj" \
	"$(INTDIR)\mod_mail_codec.obj" \
	"$(INTDIR)\mod_mail_folders.obj" \
	"$(INTDIR)\mod_mail_html.obj" \
	"$(INTDIR)\mod_mail_main.obj" \
	"$(INTDIR)\mod_mail_mime.obj" \
	"$(INTDIR)\mod_mail_server.obj"

"..\..\distrib\lib\mod_mail.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "mod_mail - Win32 Release"
SOURCE=.\mod_mail_addr.c

"$(INTDIR)\mod_mail_addr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_mail_codec.c

"$(INTDIR)\mod_mail_codec.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_mail_folders.c

"$(INTDIR)\mod_mail_folders.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_mail_html.c

"$(INTDIR)\mod_mail_html.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_mail_main.c

"$(INTDIR)\mod_mail_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_mail_mime.c

"$(INTDIR)\mod_mail_mime.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_mail_server.c

"$(INTDIR)\mod_mail_server.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

