/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
	pthread_attr_t thr_attr;
	pthread_t rc;

	setvbuf(stdout, NULL, _IONBF, 0);
	proc.RunAsCGI=0;
	conn=NULL;
	memset((char *)&proc, 0, sizeof(proc));
	proc.stats.starttime=time(NULL);
	snprintf(proc.program_name, sizeof(proc.program_name)-1, "%s", GetCommandLine());
	if (getenv("REQUEST_METHOD")!=NULL) {
		cgiinit();
		return 0;
	} else {
		init();
		if (pthread_attr_init(&thr_attr)) exit(1);
		if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
		if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
			logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
			exit(0);
		}
		if (strstr(lpCmdLine, "noicon")!=NULL) {
			proc.DaemonThread=(pthread_t)pthread_self();
			conn_reaper(NULL);
			logerror(NULL, __FILE__, __LINE__, "conn_reaper() loop failed to start.");
			exit(0);
		} else {
			if (pthread_attr_init(&thr_attr)) exit(1);
			if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
			if (pthread_create(&rc, &thr_attr, conn_reaper, NULL)==-1) {
				logerror(NULL, __FILE__, __LINE__, "conn_reaper() thread failed to start.");
				exit(0);
			}
			proc.DaemonThread=rc;
			proc.hInst=hInstance;
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_NULLDIALOG), NULL, NullDlgProc);
		}
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
	pthread_attr_t thr_attr;

	setvbuf(stdout, NULL, _IONBF, 0);
//	umask(077);
	conn=NULL;
	memset((char *)&proc, 0, sizeof(proc));
	proc.stats.starttime=time(NULL);
	snprintf(proc.program_name, sizeof(proc.program_name)-1, "%s", argv[0]);
	if (getenv("REQUEST_METHOD")!=NULL) {
		cgiinit();
		return 0;
	} else {
		init();
		daemon(0, 0);
		if (pthread_attr_init(&thr_attr)) exit(1);
		if (pthread_attr_setstacksize(&thr_attr, 65536L)) exit(1);
		if (pthread_create(&proc.ListenThread, &thr_attr, accept_loop, NULL)==-1) {
			logerror(NULL, __FILE__, __LINE__, "accept() loop failed to start.");
			exit(0);
		}
		proc.DaemonThread=pthread_self();
		conn_reaper(NULL);
	}
	return 0;
}
#endif
