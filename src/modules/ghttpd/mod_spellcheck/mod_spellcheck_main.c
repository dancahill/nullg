/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#define SRVMOD_MAIN 1
#include "nullgw/ghttpd/mod.h"
#ifndef WIN32
#include <sys/wait.h>
#endif
/* Much of this module was based on (and partly inspired by) code found at
 * http://spellerpages.sourceforge.net/
 */
#define BUFF_SIZE 8192

static int pipe_read(int pipe, char *buffer, int max)
{
	int bytesin;

#ifdef WIN32
	ReadFile((HANDLE)pipe, buffer, max, &bytesin, NULL);
#else
	bytesin=read(pipe, buffer, max);
#endif
	return bytesin;
}

static int pipe_write(int pipe, char *buffer, int max)
{
	int bytesout;

#ifdef WIN32
	WriteFile((HANDLE)pipe, buffer, max, &bytesout, NULL);
#else
	bytesout=write(pipe, buffer, max);
#endif
	return bytesout;
}

int spellcheck_shellexec(CONN *sid)
{
#ifdef WIN32
	char *aspell_loc="c:/cygwin/bin/aspell.exe -a";
	DWORD exitcode=0;
	HANDLE hMyProcess=GetCurrentProcess();
	SECURITY_ATTRIBUTES saAttr;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char Command[512];
	char Path[255];
#else
	char *aspell_loc="/usr/bin/aspell";
	int status;
	int pset1[2];
	int pset2[2];
#endif
	char *args[5];
	char *ptemp;
	char szBuffer[BUFF_SIZE];
	pipe_fd local;
	pipe_fd remote;
	int nOutRead;
	int pid;
	unsigned int i;
	unsigned int bytesleft;
	char searchstring[8192];

	int rxoffset;
	int rxsize;

	int x;
	char *obuffer;

	unsigned int numsubs;
	//int numx;
	int numwords;

	DEBUG_IN(sid, "spellcheck_main()");
	fixslashes(aspell_loc);
	if ((ptemp=getpostenv(sid, "TEXTINPUTS"))==NULL) return -1;
	memset(searchstring, 0, sizeof(searchstring));
	snprintf(searchstring, sizeof(searchstring)-1, "%s", ptemp);
	if (strlen(searchstring)<1) return 0;
	memset(args, 0, sizeof(args));
	args[0]=aspell_loc;
	args[1]="-a";
#ifdef WIN32
	memset(Command, 0, sizeof(Command));
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	snprintf(Path, sizeof(Path)-1, "%s", aspell_loc);
	if ((ptemp=strrchr(Path, '\\'))!=NULL) *ptemp='\0';
	snprintf(Command, sizeof(Command)-1, "%s", aspell_loc);
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe((HANDLE)&remote.in, (HANDLE)&local.out, &saAttr, BUFF_SIZE)) {
		send_error(sid, 500, "Internal Server Error", "Unable to create pipe.");
		return 0;
	}
	if (!CreatePipe((HANDLE)&local.in, (HANDLE)&remote.out, &saAttr, BUFF_SIZE)) {
		CloseHandle((HANDLE)remote.in);
		CloseHandle((HANDLE)local.out);
		send_error(sid, 500, "Internal Server Error", "Unable to create pipe.");
		return 0;
	}
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdInput=(HANDLE)remote.in;
	si.hStdOutput=(HANDLE)remote.out;
	si.hStdError=(HANDLE)remote.out;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, CREATE_NEW_CONSOLE|CREATE_NO_WINDOW, NULL, Path, &si, &pi)) {
		CloseHandle((HANDLE)local.in);
		CloseHandle((HANDLE)local.out);
		CloseHandle((HANDLE)remote.in);
		CloseHandle((HANDLE)remote.out);
		log_error("mod_spellcheck", __FILE__, __LINE__, 1, "program failed. [%s]", Command);
		send_error(sid, 500, "Internal Server Error", "There was a problem running the requested program.");
		return 0;
	}
	pid=pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
#else
	if ((pipe(pset1)==-1) || (pipe(pset2)==-1)) {
		close(pset1[0]);
		close(pset1[1]);
		log_error("mod_spellcheck", __FILE__, __LINE__, 1, "pipe() error");
		send_error(sid, 500, "Internal Server Error", "Unable to create pipe.");
		return 0;
	}
	local.in=pset1[0]; remote.out=pset1[1];
	remote.in=pset2[0]; local.out=pset2[1];
	pid=fork();
	if (pid<0) {
		log_error("mod_spellcheck", __FILE__, __LINE__, 1, "fork() error");
		return 0;
	} else if (pid==0) {
		close(local.in);
		close(local.out);
		dup2(remote.in, fileno(stdin));
		dup2(remote.out, fileno(stdout));
		execve(args[0], &args[0], NULL);
		log_error("mod_spellcheck", __FILE__, __LINE__, 1, "execve() error [%s][%s]", args[0], args[1]);
		exit(0);
	} else {
		close(remote.in);
		close(remote.out);
	}
#endif
//sleep(5);
// prints(sid, "/* postdata=[%s] */\r\n", sid->PostData);
	// flush the input buffer (read the app hello)
	do {
		memset(szBuffer, 0, sizeof(szBuffer));
		nOutRead=pipe_read(local.in, szBuffer, sizeof(szBuffer)-1);
		if (nOutRead<1) break;
	} while (strchr(szBuffer, '\n')==NULL);
// prints(sid, "/* reply=[%s] */\r\n", szBuffer);
	flushbuffer(sid);

// prints(sid, "/* ss1=[%s][%d] */\r\n", ptemp, strlen(searchstring));
// flushbuffer(sid);
	// enter terse mode
	nOutRead=pipe_write(local.out, "!\r\n", 3);

	decodeurl(searchstring);
	ptemp=searchstring;
	bytesleft=strlen(searchstring);
	for (i=0;i<bytesleft;i++) {
		if (strchr("\r\n<>", searchstring[i])) searchstring[i]=' ';
	}
// prints(sid, "/* ss2=[%s][%d] */\r\n", ptemp, strlen(searchstring));
// flushbuffer(sid);
	if (bytesleft>0) {
		nOutRead=pipe_write(local.out, " ", 1);
		while (bytesleft>0) {
			i=(bytesleft<4096)?bytesleft:4096;
// prints(sid, "/* wstring=[%s][%d][%d]", ptemp, i, nOutRead);
			nOutRead=pipe_write(local.out, ptemp, i);
// prints(sid, "*/\r\n");
			if (nOutRead>0) {
				ptemp+=nOutRead;
				bytesleft-=nOutRead;
			} else if (nOutRead<1) {
				break;
			}
		}
		nOutRead=pipe_write(local.out, "\r\n", 2);
		nOutRead=pipe_write(local.out, "\r\n", 2);
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	rxoffset=0;
	rxsize=0;
	numwords=0;
	do {
		if ((szBuffer[0]=='\r')||(szBuffer[0]=='\n')) break;
		if (strchr(szBuffer, '\n')==NULL) {
			x=sizeof(szBuffer)-rxoffset-rxsize-2;
			obuffer=szBuffer+rxoffset+rxsize;
			nOutRead=pipe_read(local.in, obuffer, x);
// prints(sid, "/* rstring=[%s] */\r\n", obuffer);
// flushbuffer(sid);
			if (nOutRead<1) break;
			rxsize+=nOutRead;
			continue;
		};
		ptemp=szBuffer;
/*
		// start line parsing code here
		prints(sid, "b2<BR>", searchstring); flushbuffer(sid); sleep(1);
		prints(sid, "[");
		while (*ptemp!='\n') { prints(sid, "%c", *ptemp); ptemp++; rxoffset++; rxsize--; }
		if (*ptemp=='\n') { prints(sid, "%c", *ptemp); ptemp++; rxoffset++; rxsize--; }
		prints(sid, "]<BR>\r\n");
		flushbuffer(sid);
		// end line parsing code here
*/
		// start line parsing code here
//		prints(sid, "[");
		while (rxsize>0) {
//prints(sid, "b2 line=[%s]<BR>", ptemp); flushbuffer(sid); sleep(1);


			if (strchr(ptemp, '\n')==NULL) {
//				prints(sid, "[no \\n]"); flushbuffer(sid); sleep(1);
				break;
			}

//			@(#) International Ispell Version 3.1.20 (but really Aspell 0.60.3-20050121)
//			this is a tset
//			*
//			*
//			*
//			& tset 5 10: test, stet, Set, Tet, set

			if (*ptemp=='*') {
				do {
					ptemp++;
					rxoffset++;
					rxsize--;
				} while (*ptemp!='\n');
			} else if (*ptemp=='&') {
				ptemp++; rxoffset++; rxsize--;
				while (*ptemp==' ') { ptemp++; rxoffset++; rxsize--; }
				prints(sid, "words[0][%d] = \"", numwords);
				while (*ptemp!=' ') { prints(sid, "%c", *ptemp); ptemp++; rxoffset++; rxsize--; }
				prints(sid, "\";\r\n");
				while (*ptemp==' ') { ptemp++; rxoffset++; rxsize--; }
				numsubs=atoi(ptemp);
				while (*ptemp!=' ') { ptemp++; rxoffset++; rxsize--; }
				while (*ptemp==' ') { ptemp++; rxoffset++; rxsize--; }
				//numx=atoi(ptemp);
				while (*ptemp!=' ') { ptemp++; rxoffset++; rxsize--; }
				while (*ptemp==' ') { ptemp++; rxoffset++; rxsize--; }
				prints(sid, "suggs[0][%d] = [", numwords);
				for (i=0;i<numsubs;i++) {
					prints(sid, "\"");
					while ((*ptemp!=',')&&(*ptemp!='\n')) {
						if (*ptemp=='\'') prints(sid, "\\");
						prints(sid, "%c", *ptemp); ptemp++; rxoffset++; rxsize--;
					}
					prints(sid, "\"");
					while (*ptemp==' ') { ptemp++; rxoffset++; rxsize--; }
					if (i<numsubs-1) {
						while (*ptemp==',') { ptemp++; rxoffset++; rxsize--; }
						while (*ptemp==' ') { ptemp++; rxoffset++; rxsize--; }
						prints(sid, ", ");
					}
				}
				while (*ptemp!='\n') { ptemp++; rxoffset++; rxsize--; }
				prints(sid, "];\r\n");
				numwords++;
			} else {
//				prints(sid, "%c", *ptemp);
				rxoffset++;
				rxsize--;
				if (*ptemp=='\n') { ptemp++; break; }
				ptemp++;
				if (*ptemp=='\0') break;
			}
		}
		flushbuffer(sid);
		// end line parsing code here
		if (rxsize>0) {
			memmove(szBuffer, szBuffer+rxoffset, rxsize);
			memset(szBuffer+rxsize, 0, sizeof(szBuffer)-rxsize);
			rxoffset=0;
		} else {
			memset(szBuffer, 0, sizeof(szBuffer));
			rxoffset=0;
			rxsize=0;
		}
	} while (nOutRead>0);
	/* cleanup */
#ifdef WIN32
	GetExitCodeProcess(pi.hProcess, &exitcode);
	if (exitcode==STILL_ACTIVE) TerminateProcess(pi.hProcess, 1);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle((HANDLE)local.in);
	CloseHandle((HANDLE)local.out);
#else
	close(local.in);
	close(local.out);
	wait(&status);
#endif
	flushbuffer(sid);
	return 0;
}

int spellcheck_check(CONN *sid)
{
	char *ptemp;

	prints(sid, "<html>\r\n");
	prints(sid, "<head>\r\n");
	prints(sid, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n");
	prints(sid, "<script language=\"javascript\" src=\"/groupware/js/spellcheck/wordWindow.js\"></script>\r\n");
	prints(sid, "<script language=\"javascript\">\r\n");
	prints(sid, "var suggs = new Array();\r\n");
	prints(sid, "var words = new Array();\r\n");
	prints(sid, "var textinputs = new Array();\r\n");
	prints(sid, "var error;\r\n");
//	prints(sid, "textinputs[0] = decodeURIComponent(\"\");\r\n");
//	prints(sid, "textinputs[1] = decodeURIComponent(\"this%%20is%%20a%%20tset\");\r\n");

//	prints(sid, "// [%s]\r\n", sid->PostData);
	if ((ptemp=getpostenv(sid, "TEXTINPUTS"))!=NULL) {
//		prints(sid, "// [%s]\r\n", ptemp);
		prints(sid, "textinputs[0] = decodeURIComponent(\"", ptemp);
		prints(sid, "%s", ptemp);
		prints(sid, "\");\r\n", ptemp);
	}

	prints(sid, "words[0] = [];\r\n");
	prints(sid, "suggs[0] = [];\r\n");
//	prints(sid, "// words[0][0] = 'tset';\r\n");
//	prints(sid, "// suggs[0][0] = ['test', 'stet', 'Set', 'Tet', 'set'];\r\n");
	spellcheck_shellexec(sid);
	prints(sid, "\r\n");
	prints(sid, "var wordWindowObj = new wordWindow();\r\n");
	prints(sid, "wordWindowObj.originalSpellings = words;\r\n");
	prints(sid, "wordWindowObj.suggestions = suggs;\r\n");
	prints(sid, "wordWindowObj.textInputs = textinputs;\r\n");
	prints(sid, "\r\n");
	prints(sid, "function init_spell() {\r\n");
	prints(sid, "	// check if any error occured during server-side processing\r\n");
	prints(sid, "	if( error ) {\r\n");
	prints(sid, "		alert( error );\r\n");
	prints(sid, "	} else {\r\n");
	prints(sid, "		// call the init_spell() function in the parent frameset\r\n");
	prints(sid, "		if (parent.frames.length) {\r\n");
	prints(sid, "			parent.init_spell( wordWindowObj );\r\n");
	prints(sid, "		} else {\r\n");
	prints(sid, "			alert('This page was loaded outside of a frameset. It might not display properly');\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "</script>\r\n");
	prints(sid, "</head>\r\n");
	prints(sid, "<body onLoad=\"init_spell();\">\r\n");
	prints(sid, "<script type=\"text/javascript\">\r\n");
	prints(sid, "wordWindowObj.writeBody();\r\n");
	prints(sid, "</script>\r\n");
	prints(sid, "</body>\r\n");
	prints(sid, "</html>\r\n");
	return 0;
}

int spellcheck_popup(CONN *sid)
{
	prints(sid, "<script>\r\n");
	prints(sid, "var wordWindow = null;\r\n");
	prints(sid, "var controlWindow = null;\r\n");
	prints(sid, "function init_spell( spellerWindow ) {\r\n");
	prints(sid, "	if( spellerWindow ) {\r\n");
	prints(sid, "		if( spellerWindow.windowType == \"wordWindow\" ) {\r\n");
	prints(sid, "			wordWindow = spellerWindow;\r\n");
	prints(sid, "		} else if ( spellerWindow.windowType == \"controlWindow\" ) {\r\n");
	prints(sid, "			controlWindow = spellerWindow;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	if( controlWindow && wordWindow ) {\r\n");
	prints(sid, "		// populate the speller object and start it off!\r\n");
	prints(sid, "		var speller = opener.speller;\r\n");
	prints(sid, "		wordWindow.speller = speller;\r\n");
	prints(sid, "		speller.startCheck( wordWindow, controlWindow );\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// encodeForPost\r\n");
	prints(sid, "function encodeForPost( str ) {\r\n");
	prints(sid, "	var s = new String( str );\r\n");
	prints(sid, "	s = encodeURIComponent( s );\r\n");
	prints(sid, "	// additionally encode single quotes to evade any PHP \r\n");
	prints(sid, "	// magic_quotes_gpc setting (it inserts escape characters and \r\n");
	prints(sid, "	// therefore skews the btye positions of misspelled words)\r\n");
	prints(sid, "	return s.replace( /\'/g, '%27' );\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// post the text area data to the script that populates the speller\r\n");
	prints(sid, "function postWords() {\r\n");
	prints(sid, "	var bodyDoc = window.frames[0].document;\r\n");
	prints(sid, "	bodyDoc.open();\r\n");
	prints(sid, "	bodyDoc.write('<html>');\r\n");
	prints(sid, "	bodyDoc.write('<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">');\r\n");
	prints(sid, "	if (opener) {\r\n");
	prints(sid, "		var speller = opener.speller;\r\n");
	prints(sid, "		bodyDoc.write('<body class=\"normalText\" onLoad=\"document.forms[0].submit();\">');\r\n");
	prints(sid, "		bodyDoc.write('<p>Spell check in progress...</p>');\r\n");
	prints(sid, "		bodyDoc.write('<form action=\"'+speller.spellCheckScript+'\" method=\"post\">');\r\n");
//	prints(sid, "		for( var i = 0; i < speller.textInputs.length; i++ ) {\r\n");
//	prints(sid, "			bodyDoc.write('<input type=hidden name=\"textinputs[]\" value=\"'+encodeForPost(speller.textInputs[i].value)+'\">');\r\n");
	prints(sid, "		bodyDoc.write('<input type=hidden name=\"textinputs\" value=\"'+encodeForPost(speller.textInputs[0].value)+'\">');\r\n");
//	prints(sid, "		}\r\n");
	prints(sid, "		bodyDoc.write('</form>');\r\n");
	prints(sid, "		bodyDoc.write('</body>');\r\n");
	prints(sid, "	} else {\r\n");
	prints(sid, "		bodyDoc.write('<body class=\"normalText\">');\r\n");
	prints(sid, "		bodyDoc.write('<p><b>This page cannot be displayed</b></p><p>The window was not opened from another window.</p>');\r\n");
	prints(sid, "		bodyDoc.write('</body>');\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	bodyDoc.write('</html>');\r\n");
	prints(sid, "	bodyDoc.close();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "</script>\r\n");
	prints(sid, "<html>\r\n");
	prints(sid, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n");
	prints(sid, "<head>\r\n");
	prints(sid, "<title>Speller Pages</title>\r\n");
	prints(sid, "</head>\r\n");
	prints(sid, "<frameset rows=\"*,201\" onLoad=\"postWords();\">\r\n");
	prints(sid, "<frame src=/spellcheck/blank>\r\n");
	prints(sid, "<frame src=/spellcheck/controls>\r\n");
	prints(sid, "</frameset>\r\n");
	prints(sid, "</html>\r\n");

	return 0;
}

int spellcheck_controls(CONN *sid)
{
	prints(sid, "<html>\r\n");
	prints(sid, "<head>\r\n");
	prints(sid, "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"/groupware/themes/%s/style.css\">\r\n", sid->dat->user_theme);
	prints(sid, "<script src=\"/groupware/js/spellcheck/controlWindow.js\"></script>\r\n");
	prints(sid, "<script>\r\n");
	prints(sid, "var spellerObject;\r\n");
	prints(sid, "var controlWindowObj;\r\n");
	prints(sid, "\r\n");
	prints(sid, "if (parent.opener) {\r\n");
	prints(sid, "	spellerObject=parent.opener.speller;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ignore_word() {\r\n");
	prints(sid, "	if (spellerObject) spellerObject.ignoreWord();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function ignore_all() {\r\n");
	prints(sid, "	if (spellerObject) spellerObject.ignoreAll();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function replace_word() {\r\n");
	prints(sid, "	if (spellerObject) spellerObject.replaceWord();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function replace_all() {\r\n");
	prints(sid, "	if (spellerObject) spellerObject.replaceAll();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function end_spell() {\r\n");
	prints(sid, "	if (spellerObject) spellerObject.terminateSpell();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function undo() {\r\n");
	prints(sid, "	if (spellerObject) spellerObject.undo();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function suggText() {\r\n");
	prints(sid, "	if (controlWindowObj) controlWindowObj.setSuggestedText();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function init_spell() {\r\n");
	prints(sid, "	var controlForm=document.spellcheck;\r\n");
	prints(sid, "\r\n");
	prints(sid, "	// create a new controlWindow object\r\n");
	prints(sid, "	controlWindowObj=new controlWindow(controlForm);\r\n");
	prints(sid, "\r\n");
	prints(sid, "	// call the init_spell() function in the parent frameset\r\n");
	prints(sid, "	if (parent.frames.length) {\r\n");
	prints(sid, "		parent.init_spell(controlWindowObj);\r\n");
	prints(sid, "	} else {\r\n");
	prints(sid, "		alert( 'This page was loaded outside of a frameset. It might not display properly' );\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "}\r\n");
	prints(sid, "</script>\r\n");
	prints(sid, "</head>\r\n");
	prints(sid, "<body MARGINHEIGHT=0 MARGINWIDTH=0 TOPMARGIN=0 LEFTMARGIN=0 onLoad=\"init_spell();\">\r\n");
	prints(sid, "<table border=0 cellpadding=0 cellspacing=0 border=0>\r\n");
	prints(sid, "<form name=\"spellcheck\">\r\n");
	prints(sid, "<tr><td colspan=3>Not in dictionary:</td></tr>\r\n");
	prints(sid, "<tr><td colspan=3><input type=text name=misword disabled style='width:200px'></td></tr>\r\n");
	prints(sid, "<tr><td colspan=3 height=5></td></tr>\r\n");
	prints(sid, "<tr><td >Change to:</td></tr>\r\n");
	prints(sid, "<tr valign=top>\r\n");
	prints(sid, "	<td>\r\n");
	prints(sid, "		<table border=0 cellpadding=0 cellspacing=0 border=0>\r\n");
	prints(sid, "		<tr><td><input type=text name=txtsugg style='width:200px'></td></tr>\r\n");
	prints(sid, "		<tr><td><select name=sugg size=6 onChange=suggText(); onDblClick=replace_word(); style='width:200px'><option></option></select></td></tr>\r\n");
	prints(sid, "		</table>\r\n");
	prints(sid, "	</td>\r\n");
	prints(sid, "	<td>&nbsp;&nbsp;</td>\r\n");
	prints(sid, "	<td>\r\n");
	prints(sid, "		<table border=0 cellpadding=0 cellspacing=0 border=0>\r\n");
	prints(sid, "		<tr><td><input type=button CLASS=frmButton style='width:100px' value=Ignore onClick=ignore_word();></td><td>&nbsp;&nbsp;</td><td><input type=button CLASS=frmButton style='width:100px' value='Ignore All' onClick=ignore_all();></td></tr>\r\n");
	prints(sid, "		<tr><td colspan=3 height=5></td></tr>\r\n");
	prints(sid, "		<tr><td><input type=button CLASS=frmButton style='width:100px' value=Replace onClick=replace_word();></td><td>&nbsp;&nbsp;</td><td><input type=button CLASS=frmButton style='width:100px' value='Replace All' onClick=replace_all();></td></tr>\r\n");
	prints(sid, "		<tr><td colspan=3 height=5></td></tr>\r\n");
	prints(sid, "		<tr><td><input type=button CLASS=frmButton style='width:100px' name=btnUndo value=Undo onClick=undo(); disabled></td><td>&nbsp;&nbsp;</td><td><input type=button CLASS=frmButton style='width:100px' value='Close' onClick=end_spell();></td></tr>\r\n");
	prints(sid, "		</table>\r\n");
	prints(sid, "	</td>\r\n");
	prints(sid, "</tr>\r\n");
	prints(sid, "</form>\r\n");
	prints(sid, "</table>\r\n");
	prints(sid, "</body>\r\n");
	prints(sid, "</html>\r\n");
	return 0;
}

int spellcheck_test(CONN *sid)
{
	prints(sid, "<html>\r\n");
	prints(sid, "<head>\r\n");
	prints(sid, "<title>Spell Check Tester</title>\r\n");
	prints(sid, "<script src=\"/groupware/js/spellcheck/spellChecker.js\"></script>\r\n");
	prints(sid, "<script>\r\n");
	prints(sid, "function openSpellChecker() {\r\n");
	// example 1.
	// Pass in the text inputs or textarea inputs that you
	// want to spell-check to the object's constructor,
	// then call the openChecker() method.
//	prints(sid, "	var text1 = document.form1.text1;\r\n");
	prints(sid, "	var textarea1 = document.form1.textarea1;\r\n");
//	prints(sid, "	var speller = new spellChecker( text1, textarea1 );\r\n");
	prints(sid, "	var speller = new spellChecker( textarea1 );\r\n");
	prints(sid, "	speller.openChecker();\r\n");
	// example 2.
	// Rather than passing in the form elements to the object's
	// constructor, populate the object's textInputs property,
	// then call the openChecker() method.
/*
	prints(sid, "	var speller = new spellChecker();\r\n");
	prints(sid, "	var spellerInputs = new Array();\r\n");
	prints(sid, "	for( var i = 0 ; i < document.form1.elements.length; i++ ) {\r\n");
	prints(sid, "		if( document.form1.elements[i].type.match( /^text/ )) {\r\n");
	prints(sid, "			spellerInputs[spellerInputs.length] = document.form1.elements[i];\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	speller.textInputs = spellerInputs;\r\n");
	prints(sid, "	speller.openChecker();\r\n");
*/
	// example 3.
	// use the spellCheckAll() method to check every text input
	// and textarea input in every form in the HTML document.
	// You can also use the checkTextBoxes() method or checkTextAreas()
	// method instead of spellCheckAll() to check only text inputs
	// or textarea inputs, respectively
/*
	prints(sid, "	var speller = new spellChecker();\r\n");
	prints(sid, "	speller.spellCheckAll();\r\n");
*/
	prints(sid, "}\r\n");
	prints(sid, "</script>\r\n");
	prints(sid, "</head>\r\n");
	prints(sid, "<body>\r\n");
	prints(sid, "<h1>Speller Pages</h1>\r\n");
	prints(sid, "<form name=form1>\r\n");
	prints(sid, "<input type=text name=text1 size=30>\r\n");
	prints(sid, "<br />\r\n");
	prints(sid, "<textarea name=textarea1 rows=10 cols=60></textarea>\r\n");
	prints(sid, "<br />\r\n");
	prints(sid, "<input type=button value='Check Spelling' onClick=openSpellChecker();>\r\n");
	prints(sid, "</form>\r\n");
	prints(sid, "</body>\r\n");
	prints(sid, "</html>\r\n");
	return 0;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if (strncmp(sid->dat->in_RequestURI, "/spellcheck/blank", 17)==0) {
		return 0;
	} else if (strncmp(sid->dat->in_RequestURI, "/spellcheck/test", 16)==0) {
		spellcheck_test(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/spellcheck/popup", 17)==0) {
		spellcheck_popup(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/spellcheck/controls", 20)==0) {
		spellcheck_controls(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/spellcheck/check", 17)==0) {
		spellcheck_check(sid);
		return 0;
	} else {
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_spellcheck",	// mod_name
		0,			// mod_submenu
		"",			// mod_menuname
		"",			// mod_menupic
		"",			// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/spellcheck/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
//	if (mod_export_function("mod_contacts", "mod_contacts_read", dbread_contact)!=0) return -1;
	return 0;
}
