/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define _MT 1
#pragma comment(lib, "libcmt.lib")
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <shellapi.h>
#include <signal.h>
#include <windowsx.h>
#include <io.h>
#include <direct.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#define closesocket close
#endif

#ifdef HAVE_SSL
#include "common/ssl.h"
//#include <openssl/crypto.h>
//#include <openssl/x509.h>
//#include <openssl/pem.h>
//#include <openssl/ssl.h>
//#include <openssl/err.h>
#endif

#ifdef WIN32
#ifndef BCC
	#define S_IFMT		_S_IFMT   
	#define S_IFDIR		_S_IFDIR  
	#define S_IFCHR		_S_IFCHR  
	#define S_IFIFO		_S_IFIFO  
	#define S_IFREG		_S_IFREG  
	#define S_IREAD		_S_IREAD  
	#define S_IWRITE	_S_IWRITE 
	#define S_IEXEC		_S_IEXEC 
	#define S_IFBLK		0000000
	#define S_ISDIR(x)	(x & _S_IFDIR)
#endif
#define S_IFLNK		0000000
#define S_IFSOCK	0000000
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001
#define S_ISLNK(x)	0
#define ATTRIBUTES	(_A_RDONLY|_A_HIDDEN|_A_SYSTEM|_A_SUBDIR)
#define	MAXNAMLEN	255
#endif

#ifdef WIN32
struct direct {
	ino_t d_ino;
	int d_reclen;
	int d_namlen;
	char d_name[MAXNAMLEN+1];
};
struct _dircontents {
	char *_d_entry;
	struct _dircontents *_d_next;
};
typedef struct _dirdesc {
	int dd_id;
	long dd_loc;
	struct _dircontents *dd_contents;
	struct _dircontents *dd_cp;
} DIR;
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
/* pthread_ typedefs */
typedef HANDLE pthread_t;
typedef struct thread_attr {
	DWORD dwStackSize;
	DWORD dwCreatingFlag;
	int priority;
} pthread_attr_t;
typedef struct {
	int dummy;
} pthread_condattr_t;
typedef unsigned int uint;
typedef struct {
	uint waiting;
	HANDLE semaphore;
} pthread_cond_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif

#ifdef WIN32
int	winsystem(const char *format, ...);
void	TrayIcon(int newstatus);
void	TrayBlink();
BOOL	CALLBACK NullDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
unsigned sleep(unsigned seconds);
DIR	*opendir(const char *);
void	closedir(DIR *);
#define	rewinddir(dirp)	seekdir(dirp, 0L)
void	seekdir(DIR *, long);
long	telldir(DIR *);
struct	direct *readdir(DIR *);
int	gettimeofday(struct timeval *tv, struct timezone *tz); 

#define dlopen(x, y) LoadLibrary(x)
#define dlsym(x, y) GetProcAddress(x, y)
#define dlclose(x) FreeLibrary(x)
/* registry and service control functions */
int get_reg_entries(void);
/* The following is to emulate the posix thread interface */
#define pthread_mutex_init(A,B)  InitializeCriticalSection(A)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
#define pthread_mutex_unlock(A)  LeaveCriticalSection(A)
#define pthread_mutex_destroy(A) DeleteCriticalSection(A)
#define pthread_handler_decl(A,B) unsigned __cdecl A(void *B)
#define pthread_self() GetCurrentThreadId()
typedef unsigned (__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
int pthread_kill(pthread_t handle, int sig);
void pthread_exit(unsigned A);
#endif

#ifdef WIN32
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

typedef struct {
	pthread_t handle;
#ifdef WIN32
	unsigned int id;
#else
	pthread_t id;
#endif
	short int state;
#ifdef HAVE_SSL
	SSL_CTX *ctx;
	SSL *ssl;
#endif
	short int socket;
	struct sockaddr_in ClientAddr;
	time_t ctime; // Creation time
	time_t atime; // Last Access time
	// TCP INPUT BUFFER
	short int recvbufsize;
	short int recvbufoffset;
	char      recvbuf[2048];
} CONN;
