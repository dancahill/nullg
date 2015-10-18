/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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
#include "nsp/nsplib.h"
#include "wingdi.h"
#ifdef WIN32
//#include <mmsystem.h>
//#include <shellapi.h>
#include <wingdi.h>
#include <winuser.h>
#include <stdio.h>

//#define snprintf _snprintf
//#define vsnprintf _vsnprintf
//#define strcasecmp stricmp
//#define strncasecmp strnicmp

typedef struct WINDOW {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's socket-specific */
	WNDCLASS wndClass;
	HWND hwnd;
	HFONT hFont;
	HPEN hPen;
	HBRUSH hBrush;
	short MouseX;
	short MouseY;
} WINDOW;

void window_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":window_murder()"
	WINDOW *win;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	DestroyWindow(win->hwnd);
	n_free(N, (void *)&cobj->val->d.str, sizeof(WINDOW) + 1);
	return;
#undef __FN__
}

static void winerror(int line)
{
	LPVOID lpMsgBuf;
	DWORD rc;

	rc = GetLastError();
	if (rc == 87) {
		printf("**** you, microsoft\r\n");
		return;
	}
	if (!rc) return;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		rc,
		0,
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
		);

	//	printf("%d rc=0x%x\r\n", line, rc);
	//	if (lpMsgBuf) {
	printf("rc %d, %s\r\n", rc, lpMsgBuf);
	LocalFree(lpMsgBuf);
	//	}
	SetLastError(0);
	winerror(__LINE__);
	ExitProcess(0);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//	winerror(__LINE__);
	//	if (uMsg>=WM_CREATE && uMsg<=WM_SHOWWINDOW)
	//		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	//	if (uMsg>=WM_CONTEXTMENU && uMsg<=WM_NCPAINT)
	//		return DefWindowProc(hwnd, uMsg, wParam, lParam);


	/*	winerror(__LINE__);
		printf("0x%x\r\n", uMsg);
		switch(uMsg) {
		case WM_MOUSEMOVE:
			MouseX=GET_X_LPARAM(lParam);
			MouseY=GET_Y_LPARAM(lParam);
			break;
		}
	*/
	return DefWindowProc(hwnd, uMsg, wParam, lParam);


	//	WM_SETCURSOR 0x20
	//	WM_NCHITTEST 0x84
	//	WM_MOUSEMOVE 0x200

	/*

		switch(uMsg) {
		case WM_KEYUP:
			printf("WM_KEYUP\r\n");
			return FALSE;
		case WM_MOUSEFIRST:
			printf("WM_MOUSEFIRST\r\n");
			return FALSE;
	//	case WM_SETREDRAW:
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
		case WM_GETMINMAXINFO:
		case WM_NCCREATE:
		case WM_NCACTIVATE:
		case WM_NCDESTROY:
		case WM_ACTIVATE:
		case WM_GETICON:

		case WM_ACTIVATEAPP:
		case WM_IME_SETCONTEXT:
		case WM_IME_NOTIFY:
		case WM_PAINT:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			printf("0x%x\r\n", uMsg);winerror(__LINE__);
		}
		return FALSE;
	*/
}

NSP_CLASSMETHOD(libnsp_wingdi_update)
{
#define __FN__ __FILE__ ":libnsp_wingdi_update()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj, *tobj;
	WINDOW *win;
	MSG msg;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	if (GetAsyncKeyState(VK_ESCAPE) < 0) {
		DestroyWindow(win->hwnd);
		n_free(N, (void *)&cobj->val->d.str, sizeof(WINDOW) + 1);
		nsp_setbool(N, thisobj, "window", 0);
		n_error(N, NE_SYNTAX, __FN__, "bored of looping");
	}
	while (PeekMessage(&msg, win->hwnd, 0, 0, PM_REMOVE)) {
		winerror(__LINE__);
		switch (msg.message) {
		case WM_MOUSEMOVE:
			tobj = nsp_settable(N, thisobj, "mouse");
			nsp_setnum(N, tobj, "x", win->MouseX = LOWORD(msg.lParam));
			nsp_setnum(N, tobj, "y", win->MouseY = HIWORD(msg.lParam));
			break;
		default:
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_create)
{
#define __FN__ __FILE__ ":libnsp_wingdi_create()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;
	char *title = NULL;
	unsigned short width;
	unsigned short height;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "1")))) {
		title = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "title")))) {
		title = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for title");
	}
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "2")))) {
		width = (unsigned short)nsp_tonum(N, cobj);
	}
	else if (nsp_isnum((cobj = nsp_getobj(N, thisobj, "width")))) {
		width = (unsigned short)nsp_tonum(N, cobj);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for width");
	}
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "3")))) {
		height = (unsigned short)nsp_tonum(N, cobj);
	}
	else if (nsp_isnum((cobj = nsp_getobj(N, thisobj, "height")))) {
		height = (unsigned short)nsp_tonum(N, cobj);
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for height");
	}
	win = n_alloc(N, sizeof(WINDOW) + 1, 1);
	nc_strncpy(win->obj_type, "window", sizeof(win->obj_type) - 1);
	win->obj_term = (NSP_CFREE)window_murder;

	cobj = nsp_setcdata(N, thisobj, "window", NULL, 0);
	cobj->val->d.str = (void *)win;
	cobj->val->size = sizeof(WINDOW) + 1;

	memset(&win->wndClass, 0, sizeof(win->wndClass));
	win->wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	//	win->wndClass.hIcon = LoadIcon(hInst, "AppIcon");
	win->wndClass.lpfnWndProc = WndProc;
	win->wndClass.hInstance = NULL;
	win->wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	win->wndClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	win->wndClass.lpszClassName = "window_class";
	if (!RegisterClass(&win->wndClass)) {
		nsp_setbool(N, &N->r, "", 0);
		return FALSE;
	}
	win->hwnd = CreateWindow("window_class", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, NULL, NULL);
	ShowWindow(win->hwnd, SW_SHOW);
	nsp_setbool(N, &N->r, "", 1);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_destroy)
{
#define __FN__ __FILE__ ":libnsp_wingdi_destroy()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		goto end;
	//		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	DestroyWindow(win->hwnd);
	if (win->hBrush) {
		if (!DeleteObject(win->hBrush)) n_warn(N, __FN__, "DeleteObject failed");
		win->hBrush = NULL;
	}
	if (win->hFont) {
		if (!DeleteObject(win->hFont)) n_warn(N, __FN__, "DeleteObject failed");
		win->hFont = NULL;
	}
	if (win->hPen) {
		if (!DeleteObject(win->hPen)) n_warn(N, __FN__, "DeleteObject failed");
		win->hPen = NULL;
	}
	n_free(N, (void *)&cobj->val->d.str, sizeof(WINDOW) + 1);
	cobj->val->size = 0;
end:
	nsp_setbool(N, thisobj, "window", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_getsize)
{
#define __FN__ __FILE__ ":libnsp_wingdi_getsize()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t tobj;
	WINDOW *win;
	RECT rect;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	GetClientRect(win->hwnd, &rect);
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;
	nsp_setnum(N, &tobj, "left", rect.left);
	nsp_setnum(N, &tobj, "right", rect.right);
	nsp_setnum(N, &tobj, "top", rect.top);
	nsp_setnum(N, &tobj, "bottom", rect.bottom);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_setbrush)
{
#define __FN__ __FILE__ ":libnsp_wingdi_setbrush()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	if (win->hBrush) {
		if (!DeleteObject(win->hBrush)) n_warn(N, __FN__, "DeleteObject failed");
	}
	win->hBrush = CreateSolidBrush(RGB(0, 0, 0));
	if (!win->hBrush) n_error(N, NE_SYNTAX, __FN__, "CreateSolidBrush failed");

	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_setfont)
{
#define __FN__ __FILE__ ":libnsp_wingdi_setfont()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	if (win->hFont) {
		if (!DeleteObject(win->hFont)) n_warn(N, __FN__, "DeleteObject failed");
	}
	win->hFont = CreateFont(20, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "arial");
	if (!win->hFont) n_error(N, NE_SYNTAX, __FN__, "CreateFont failed");

	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_setpen)
{
#define __FN__ __FILE__ ":libnsp_wingdi_setpen()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;
	DWORD color;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;
	if (win->hPen) {
		if (!DeleteObject(win->hPen)) n_warn(N, __FN__, "DeleteObject failed");
	}
	if (nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) {
		color = (DWORD)cobj->val->d.num;
		color = (color >> 16 & 255) + ((color >> 8 & 255) << 8) + ((color & 255) << 16);
	}
	else {
		color = RGB(0, 255, 0);
	}
	win->hPen = CreatePen(PS_SOLID, 1, color);
	if (!win->hPen) n_error(N, NE_SYNTAX, __FN__, "CreatePen failed");

	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_fillrect)
{
#define __FN__ __FILE__ ":libnsp_wingdi_fillrect()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;
	RECT rect;
	HDC hDC;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;

	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg1");
	rect.left = (long)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "2")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	rect.right = (long)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "3")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg3");
	rect.top = (long)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "4")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg4");
	rect.bottom = (long)cobj->val->d.num;

	hDC = GetDC(win->hwnd);
	if (hDC == NULL) n_error(N, NE_SYNTAX, __FN__, "GetDC failed");
	SelectObject(hDC, win->hBrush);
	FillRect(hDC, &rect, win->hBrush);
	//	SelectObject(hDC, GetStockObject(BLACK_BRUSH));
	ReleaseDC(win->hwnd, hDC);

	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_print)
{
#define __FN__ __FILE__ ":libnsp_wingdi_print()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;
	HDC hDC;
	//	PAINTSTRUCT ps;
	unsigned short x, y;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;

	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg1");
	x = (unsigned short)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "2")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	y = (unsigned short)cobj->val->d.num;
	if (!nsp_isstr((cobj = nsp_getobj(N, &N->l, "3")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");

	//	hDC=BeginPaint(win->hwnd, &ps);
	hDC = GetDC(win->hwnd);
	if (!hDC) n_error(N, NE_SYNTAX, __FN__, "GetDC failed");
	if (win->hFont == NULL) n_error(N, NE_SYNTAX, __FN__, "win->hFont=NULL");
	SelectObject(hDC, win->hFont);
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(0, 0, 255));
	TextOut(hDC, x, y, cobj->val->d.str, cobj->val->size);
	//	SelectObject(hDC, GetStockObject(ANSI_VAR_FONT));
	//	EndPaint(win->hwnd, &ps);
	ReleaseDC(win->hwnd, hDC);

	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_wingdi_drawline)
{
#define __FN__ __FILE__ ":libnsp_wingdi_drawline()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	WINDOW *win;
	HDC hDC;
	unsigned short x1, y1, x2, y2;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "window");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "window") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a window");
	win = (WINDOW *)cobj->val->d.str;

	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "1")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg1");
	x1 = (unsigned short)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "2")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg2");
	y1 = (unsigned short)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "3")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg3");
	x2 = (unsigned short)cobj->val->d.num;
	if (!nsp_isnum((cobj = nsp_getobj(N, &N->l, "4")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for arg4");
	y2 = (unsigned short)cobj->val->d.num;

	hDC = GetDC(win->hwnd);
	if (hDC == NULL) n_error(N, NE_SYNTAX, __FN__, "GetDC failed");
	if (win->hPen == NULL) n_error(N, NE_SYNTAX, __FN__, "win->hPen=NULL");
	SelectObject(hDC, win->hPen);
	MoveToEx(hDC, x1, y1, NULL);
	LineTo(hDC, x2, y2);
	//	SelectObject(hDC, GetStockObject(BLACK_PEN));
	ReleaseDC(win->hwnd, hDC);

	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_wingdi_window)
{
#define __FN__ __FILE__ ":libnsp_wingdi_window()"
	nsp_setcfunc(N, &N->l, "create", (NSP_CFUNC)libnsp_wingdi_create);
	nsp_setcfunc(N, &N->l, "destroy", (NSP_CFUNC)libnsp_wingdi_destroy);
	nsp_setcfunc(N, &N->l, "update", (NSP_CFUNC)libnsp_wingdi_update);
	nsp_setcfunc(N, &N->l, "getsize", (NSP_CFUNC)libnsp_wingdi_getsize);
	nsp_setcfunc(N, &N->l, "setbrush", (NSP_CFUNC)libnsp_wingdi_setbrush);
	nsp_setcfunc(N, &N->l, "setfont", (NSP_CFUNC)libnsp_wingdi_setfont);
	nsp_setcfunc(N, &N->l, "setpen", (NSP_CFUNC)libnsp_wingdi_setpen);
	nsp_setcfunc(N, &N->l, "fillrect", (NSP_CFUNC)libnsp_wingdi_fillrect);
	nsp_setcfunc(N, &N->l, "print", (NSP_CFUNC)libnsp_wingdi_print);
	nsp_setcfunc(N, &N->l, "drawline", (NSP_CFUNC)libnsp_wingdi_drawline);
	nsp_setstr(N, &N->l, "title", "untitled", -1);
	nsp_setnum(N, &N->l, "width", 320);
	nsp_setnum(N, &N->l, "height", 240);
	nsp_setbool(N, &N->l, "window", 0);
	return 0;
#undef __FN__
}

#endif

int nspwingdi_register_all(nsp_state *N)
{
#ifdef WIN32
	obj_t *tobj;

	tobj = nsp_settable(N, &N->g, "win");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "gdi");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "window", (NSP_CFUNC)libnsp_wingdi_window);
#endif
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspwingdi_register_all(N);
	return 0;
}
#endif
