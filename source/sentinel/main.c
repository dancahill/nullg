/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

void dorequest(int sid)
{
	unsigned char file[255];

	if (RunAsCGI) {
		read_cgienv(sid);
	} else {
		if (read_header(sid)<0) {
			closeconnect(sid, 1);
			return;
		}
	}
	logaccess(2, "%s - HTTP Request: %s %s", conn[sid].dat->in_ClientIP, conn[sid].dat->in_RequestMethod, conn[sid].dat->in_RequestURI);
	snprintf(file, sizeof(file)-1, "%s%s", config.server_http_dir, conn[sid].dat->in_RequestURI);
	if (strncmp(conn[sid].dat->in_RequestURI, "/sentinel/", 10)==0) {
		if (sendfile(sid, file)==0) {
			return;
		}
	}
	snprintf(conn[sid].dat->out_ContentType, sizeof(conn[sid].dat->out_ContentType)-1, "text/html");
	if ((strcmp(conn[sid].dat->in_RequestURI, "/")==0)&&(strcmp(conn[sid].dat->in_RequestMethod, "POST")==0)) {
		if (setcookie(sid)==0) {
			logaccess(0, "%s - Login: username=%s", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username);
			printmain(sid);
		} else {
			logaccess(0, "%s - Login failed: username=%s", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username);
			printlogin(sid);
		}
		return;
	}
	if ((getcookie(sid)!=0)||(strcmp(conn[sid].dat->in_RequestURI, "/")==0)) {
		printlogin(sid);
		return;
	}
	if (strncmp(conn[sid].dat->in_RequestURI, "/logout", 7)==0) sentinellogout(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/recvfile",    9)==0) recvfile(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mainright",  10)==0) printmainright(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/maintop",     8)==0) printmaintop(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mainmain",    9)==0) printmotd(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mainbottom", 11)==0) printmainbottom(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/menu",        5)==0) printmenu(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/admin",       6)==0) adminmain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/calendar",    9)==0) calendarmain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/company",     8)==0) companymain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/contact",     8)==0) contactmain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/fileul",      7)==0) fileul(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/forum",       6)==0) forummain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/links",       6)==0) linksmain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/message",     8)==0) messagemain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/order",       6)==0) ordermain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/product",     8)==0) productmain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/profile",     8)==0) profilemain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/query",       6)==0) querymain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/task",        5)==0) taskmain(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/mail",        5)==0) webmailmain(sid);
	else if (sendfile(sid, file)==0) return;
	else if (dirlist(sid)==0) return;
	else {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		prints("<BR><CENTER>The file or function '%s' could not be found.</CENTER>\n", conn[sid].dat->in_RequestURI);
		logerror("%s - Incorrect function call '%s' by %s", conn[sid].dat->in_ClientIP, conn[sid].dat->in_RequestURI, conn[sid].dat->in_username);
	}
	return;
}

#ifdef WIN32
/****************************************************************************
 *	WinMain()
 *
 *	Purpose	: Program entry point (Win32), thread handling and window management
 *	Args	: Command line parameters (if any)
 *	Returns	: Exit status of program
 *	Notes	: None
 ***************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RunAsCGI=0;
	snprintf(program_name, sizeof(program_name)-1, "%s", GetCommandLine());
	if (getenv("REQUEST_METHOD")!=NULL) {
		cgiinit();
		dorequest(0);
		closeconnect(0, 1);
		exit(0);
	}
	init();
	if (_beginthread(WSAReaper, 0, NULL)==-1) {
		MessageBox(0, "Winsock reaper thread failed to start", APPTITLE, MB_ICONERROR);
		exit(0);
	}
	if (_beginthread(accept_loop, 0, NULL)==-1) {
		MessageBox(0, "accept() thread failed to start", APPTITLE, MB_ICONERROR);
		exit(0);
	}
	if (strstr(lpCmdLine, "noicon")!=NULL) {
		while (1) sleep(1);
	} else {
		hInst=hInstance;
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_NULLDIALOG), NULL, NullDlgProc);
	}
	return 0;
}
#else
/****************************************************************************
 *	main()
 *
 *	Purpose	: Program entry point (UNIX) and call accept loop
 *	Args	: Command line parameters (if any)
 *	Returns	: Exit status of program
 *	Notes	: None
 ***************************************************************************/
int main(int argc, char *argv[])
{
	RunAsCGI=0;
	snprintf(program_name, sizeof(program_name)-1, "%s", argv[0]);
	if (getenv("REQUEST_METHOD")!=NULL) {
		cgiinit();
		dorequest(0);
		closeconnect(0, 1);
		exit(0);
	} else {
		init();
		accept_loop(NULL);
	}
	return 0;
}
#endif
