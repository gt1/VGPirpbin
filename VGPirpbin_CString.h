/**
    Copyright (C) 2009-2019 German Tischler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#if ! defined(VGPIRPBIN_CSTRING_H)
#define VGPIRPBIN_CSTRING_H

#include "VGPirpbin_pre.h"

typedef struct _CString
{
	char const * a;
	char const * e;
} CString;

CString CString_getString(char const ** a, char const ** e);
char * CString_tostring(CString const * a);
#endif
