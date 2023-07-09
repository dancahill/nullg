/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
*/

//https://msdn.microsoft.com/en-us/library/ms235281.aspx

#include "nsp/nsp.h"
#include "nsp/nsplib.h"
#include <windows.h>
#include <vcclr.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>

#using <System.dll>

#define COMMABUFSIZE 32

using namespace System;
using namespace System::Runtime::InteropServices;

public delegate void WriteBufferDelegate(String ^outbuf);
public delegate void SuspendScriptDelegate();

static NSP_FUNCTION(nsp_edit_break);
static NSP_FUNCTION(nsp_edit_flush);

public ref class NSPObject {
public:
	obj_t *obj;
	unsigned short type;
	String^ name;
	String^ value;

	NSPObject() {
		type = 0;
		name = gcnew String("");
		value = gcnew String("");
	}

	NSPObject(obj_t *o) {
		obj = o;
		if (obj && obj->val) {
			type = obj->val->type;
			name = gcnew String(obj->name);
			if (obj->val->type == NT_NUMBER) {
				char numbuf[128];
				n_ntoa(NULL, numbuf, obj->val->d.num, -10, 6);
				value = gcnew String(numbuf);
			} else {
				value = gcnew String(nsp_tostr(NULL, obj));
			}
		} else {
			NSPObject();
		}
	}

	~NSPObject() {
	}
protected:
	!NSPObject() {
	}
public:
	NSPObject^ GetFirst() {
		if (obj->val->type == NT_TABLE) {
			return gcnew NSPObject(obj->val->d.table.f);
		} else {
			return gcnew NSPObject();
		}
	}

	NSPObject^ GetNext() {
		return gcnew NSPObject(obj->next);
	}

	bool IsValid() {
		return (obj);
	}
};

public ref class NSP {
public:
	NSP() {
	}

	~NSP() {
	}
protected:
	!NSP() {
	}
public:
	static WriteBufferDelegate ^WriteBuffer;
	static SuspendScriptDelegate ^SuspendScript;
	NSPState *NState;
	char *srcfilename;

	NSPObject^ GetGlobal() {
		if (!NState) return nullptr;
		return gcnew NSPObject(NState->getG());
	}

	NSPObject^ GetThis() {
		if (!NState) return nullptr;
		return gcnew NSPObject(NState->getT());
	}

	NSPObject^ GetLocal() {
		if (!NState) return nullptr;
		return gcnew NSPObject(NState->getL());
	}

	int ExecScript(String ^script, String ^filename) {
		IntPtr ip = Marshal::StringToHGlobalAnsi(script);
		IntPtr ip2 = Marshal::StringToHGlobalAnsi(filename);
		const char *str = static_cast<const char*>(ip.ToPointer());

		srcfilename = static_cast<char*>(ip2.ToPointer());
		Beep(440, 250);
		//__try {
		NState = new NSPState();
		nsp_state *N = NState->getN();

		init_stuff(N);
		NState->exec(str);
		Marshal::FreeHGlobal(ip);
		if (N->err) {
			NSP::WriteBuffer(gcnew String("Exception: ") + gcnew String(N->errbuf));
		}
		nsp_freestate(N);
		printstate(N);
		nsp_endstate(N);
		NState = NULL;
		//}
		//__except (do_filter(GetExceptionInformation())) {
		//}
		//delete NState;
		Beep(880, 250);
		return 0;
	}
private:
	void printstate(nsp_state *N)
	{
		char buf1[COMMABUFSIZE];
		char buf2[COMMABUFSIZE];

		if (N->allocs == N->frees) return;
		NSP::WriteBuffer("\r\nINTERNAL STATE REPORT\r\n");
		if (N->allocs)   NSP::WriteBuffer(String::Format(gcnew String("\tallocs   = {0} ({1} bytes)\r\n"), printcommas(N->allocs, buf1, 1), printcommas(N->allocmem, buf2, 1)));
		if (N->frees)    NSP::WriteBuffer(String::Format(gcnew String("\tfrees    = {0} ({1} bytes)\r\n"), printcommas(N->frees, buf1, 1), printcommas(N->freemem, buf2, 1)));
		if (N->allocs)   NSP::WriteBuffer(String::Format(gcnew String("\tdiff     = {0} ({1} bytes)\r\n"), printcommas(abs(N->allocs - N->frees), buf1, 1), printcommas(abs(N->allocmem - N->freemem), buf2, 1)));
		if (N->peakmem)  NSP::WriteBuffer(String::Format(gcnew String("\tpeak     = {0} bytes\r\n"), printcommas(N->peakmem, buf1, 1)));
		if (N->counter1) NSP::WriteBuffer(String::Format(gcnew String("\tcounter1 = {0}\r\n"), printcommas(N->counter1, buf1, 1)));
		NSP::WriteBuffer(String::Format(gcnew String("\tsizeof(nsp_state)  = {0}\r\n"), (int)sizeof(nsp_state)));
		NSP::WriteBuffer(String::Format(gcnew String("\tsizeof(nsp_objrec) = {0}\r\n"), (int)sizeof(nsp_objrec)));
		NSP::WriteBuffer(String::Format(gcnew String("\tsizeof(nsp_valrec) = {0}\r\n"), (int)sizeof(nsp_valrec)));
		return;
	}

	String ^printcommas(int num, char *buf, short clear)
	{
		if (clear) buf[0] = 0;
		if (num > 999) printcommas(num / 1000, buf, 0);
		snprintf(buf + strlen(buf), COMMABUFSIZE - strlen(buf), "%0*d%s", num > 999 ? 3 : 1, num % 1000, clear ? "" : ",");
		return gcnew String(buf);
	}

	void init_stuff(nsp_state *N)
	{
		char tmpbuf[MAX_OBJNAMELEN + 1];
		obj_t *tobj;
		int i;
		char *p;

		N->debug = 0;
		/* add env */
		tobj = nsp_settable(N, &N->g, "_ENV");
		for (i = 0; environ[i] != NULL; i++) {
			strncpy(tmpbuf, environ[i], MAX_OBJNAMELEN);
			p = strchr(tmpbuf, '=');
			if (!p) continue;
			*p = '\0';

			/* env vars should ignore case, so force uniformity */
			p = tmpbuf; while (*p) *p++ = toupper(*p);

			p = strchr(environ[i], '=') + 1;
			nsp_setstr(N, tobj, tmpbuf, p, -1);
		}
		preppath(N, srcfilename);
		tobj = nsp_settable(N, nsp_settable(N, &N->g, "lib"), "debug");
		nsp_setcfunc(N, tobj, "break2", nsp_edit_break);
		tobj = nsp_settable(N, nsp_settable(N, &N->g, "lib"), "io");
		nsp_setcfunc(N, tobj, "flush", nsp_edit_flush);
		return;
	}

	void preppath(nsp_state *N, char *name)
	{
		char buf[1024];
		char *p;
		unsigned int j;

		if (strlen(name) == 0) return;
		p = name;
		if ((name[0] == '/') || (name[0] == '\\') || (name[1] == ':')) {
			/* it's an absolute path.... probably... */
			strncpy(buf, name, sizeof(buf));
		} else if (name[0] == '.') {
			/* looks relative... */
			getcwd(buf, (unsigned long)(sizeof(buf) - strlen(name) - 2));
			strcat(buf, "/");
			strcat(buf, name);
		} else {
			getcwd(buf, (unsigned long)(sizeof(buf) - strlen(name) - 2));
			strcat(buf, "/");
			strcat(buf, name);
		}
		for (j = 0; j < strlen(buf); j++) {
			if (buf[j] == '\\') buf[j] = '/';
		}
		for (j = (unsigned long)strlen(buf) - 1; j > 0; j--) {
			if (buf[j] == '/') { buf[j] = '\0'; p = buf + j + 1; break; }
		}
		nsp_setstr(N, &N->g, "_filename", p, -1);
		nsp_setstr(N, &N->g, "_filepath", buf, -1);
#if defined(_WIN32) && defined(_DEBUG)
		nsp_setbool(N, nsp_settable(N, nsp_settable(N, &N->g, "lib"), "debug"), "attached", 1);
#endif
		return;
	}

	DWORD do_filter(EXCEPTION_POINTERS *eps)
	{
		EXCEPTION_RECORD er = *eps->ExceptionRecord;
		char errbuf[255];

		memset(errbuf, 0, sizeof(errbuf));
		switch (er.ExceptionCode) {
		case 0xE06D7363: // C++ exception
			_snprintf(errbuf, sizeof(errbuf) - 1, "Unknown C++ exception thrown. 0x%08X", er.ExceptionCode);
			break;
		case EXCEPTION_ACCESS_VIOLATION:
			_snprintf(errbuf, sizeof(errbuf) - 1, "EXCEPTION_ACCESS_VIOLATION (0x%08X): ExceptionAddress=0x%p", er.ExceptionCode, er.ExceptionAddress);
			break;
		case EXCEPTION_STACK_OVERFLOW:
			_snprintf(errbuf, sizeof(errbuf) - 1, "EXCEPTION_STACK_OVERFLOW (0x%08X): ExceptionAddress=0x%p", er.ExceptionCode, er.ExceptionAddress);
			break;
		default:
			_snprintf(errbuf, sizeof(errbuf) - 1, "SEH Exception (0x%08X): ExceptionAddress=0x%p", er.ExceptionCode, er.ExceptionAddress);
			break;
		}
		/*log_error(N, "SEH Exception [%s]", errbuf);*/
		/*MessageBox(NULL, errbuf, TITLE_PREFIX "SEH Exception", MB_OK);*/
		return EXCEPTION_EXECUTE_HANDLER;
	}
};

static NSP_FUNCTION(nsp_edit_break)
{
	NSP::SuspendScript();
	return 0;
}

static NSP_FUNCTION(nsp_edit_flush)
{
	N->outbuffer[N->outbuflen] = '\0';
	NSP::WriteBuffer(gcnew String(N->outbuffer));
	N->outbuflen = 0;
	N->outbuffer[N->outbuflen] = '\0';
	return 0;
}
