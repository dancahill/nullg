/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#include "libnsd.h"

char *fixslashes(char *string)
{
	char *p=string;

 	while (*p) {
 		if (*p=='\\') *p='/';
		p++;
	}
	return string;
}

char *striprn(char *string)
{
	int i=strlen(string)-1;

	while (1) {
		if (i<0) break;
		if (string[i]=='\r') { string[i]='\0'; i--; continue; }
		if (string[i]=='\n') { string[i]='\0'; i--; continue; }
		break;
	}
	return string;
}

void swapchar(char *string, char oldchar, char newchar)
{
	while (*string) {
		if (*string==oldchar) *string=newchar;
		string++;
	}
}
