# Microsoft Developer Studio Generated NMAKE File, Based on mod_email.dsp
!IF "$(CFG)" == ""
CFG=mod_email - Win32 Release
!MESSAGE mod_email - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_email - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_email.mak" CFG="mod_email - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_email - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=..\..\..\..\obj\modules\ghttpd\mod_email
INTDIR=..\..\..\..\obj\modules\ghttpd\mod_email

ALL : "..\..\..\..\distrib\lib\ghttpd\mod_email.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_email_accounts.obj"
	-@erase "$(INTDIR)\mod_email_address.obj"
	-@erase "$(INTDIR)\mod_email_codec.obj"
	-@erase "$(INTDIR)\mod_email_conf.obj"
	-@erase "$(INTDIR)\mod_email_db.obj"
	-@erase "$(INTDIR)\mod_email_files.obj"
	-@erase "$(INTDIR)\mod_email_filters.obj"
	-@erase "$(INTDIR)\mod_email_folders.obj"
	-@erase "$(INTDIR)\mod_email_html.obj"
	-@erase "$(INTDIR)\mod_email_lang.obj"
	-@erase "$(INTDIR)\mod_email_main.obj"
	-@erase "$(INTDIR)\mod_email_mime.obj"
	-@erase "$(INTDIR)\mod_email_search.obj"
	-@erase "$(INTDIR)\mod_email_server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_email.exp"
	-@erase "$(OUTDIR)\mod_email.lib"
	-@erase "..\..\..\..\distrib\lib\ghttpd\mod_email.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_email.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\mod_email.pdb" /machine:I386 /def:".\mod_email.def" /out:"..\..\..\..\distrib\lib\ghttpd\mod_email.dll" /implib:"$(OUTDIR)\mod_email.lib" 
DEF_FILE= \
	".\mod_email.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_email_accounts.obj" \
	"$(INTDIR)\mod_email_address.obj" \
	"$(INTDIR)\mod_email_codec.obj" \
	"$(INTDIR)\mod_email_conf.obj" \
	"$(INTDIR)\mod_email_db.obj" \
	"$(INTDIR)\mod_email_files.obj" \
	"$(INTDIR)\mod_email_filters.obj" \
	"$(INTDIR)\mod_email_folders.obj" \
	"$(INTDIR)\mod_email_html.obj" \
	"$(INTDIR)\mod_email_lang.obj" \
	"$(INTDIR)\mod_email_main.obj" \
	"$(INTDIR)\mod_email_mime.obj" \
	"$(INTDIR)\mod_email_search.obj" \
	"$(INTDIR)\mod_email_server.obj"

"..\..\..\..\distrib\lib\ghttpd\mod_email.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "mod_email - Win32 Release"
SOURCE=.\mod_email_accounts.c

"$(INTDIR)\mod_email_accounts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_address.c

"$(INTDIR)\mod_email_address.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_codec.c

"$(INTDIR)\mod_email_codec.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_conf.c

"$(INTDIR)\mod_email_conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_db.c

"$(INTDIR)\mod_email_db.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_files.c

"$(INTDIR)\mod_email_files.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_filters.c

"$(INTDIR)\mod_email_filters.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_folders.c

"$(INTDIR)\mod_email_folders.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_html.c

"$(INTDIR)\mod_email_html.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_lang.c

"$(INTDIR)\mod_email_lang.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_main.c

"$(INTDIR)\mod_email_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_mime.c

"$(INTDIR)\mod_email_mime.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_search.c

"$(INTDIR)\mod_email_search.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_email_server.c

"$(INTDIR)\mod_email_server.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

