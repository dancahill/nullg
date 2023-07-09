/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "libnullg.h"
#ifdef WIN32
#else
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif

#ifndef RTLD_DEEPBIND
#define RTLD_DEEPBIND 0
#endif

void *lib_open(const char *file)
{
#ifdef WIN32
	return LoadLibrary(file);
#else
#ifdef HAVE_DLFCN_H
	return dlopen(file, RTLD_NOW|RTLD_DEEPBIND);
#else
	return NULL;
#endif
#endif
}

void *lib_sym(void *handle, const char *name)
{
#ifdef WIN32
	return GetProcAddress(handle, name);
#else
#ifdef HAVE_DLFCN_H
	return dlsym(handle, name);
#else
	return NULL;
#endif
#endif
}

char *lib_error(void)
{
#ifdef WIN32
	return "";
#else
#ifdef HAVE_DLFCN_H
	return dlerror();
#else
	return NULL;
#endif
#endif
}

int lib_close(void *handle)
{
#ifdef WIN32
	return FreeLibrary(handle);
#else
#ifdef HAVE_DLFCN_H
	return dlclose(handle);
#else
	return 0;
#endif
#endif
}
