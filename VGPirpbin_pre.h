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
#if ! defined(VGPIRPBIN_PRE_H)
#define VGPIRPBIN_PRE_H

#if defined(__linux__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__APPLE__)
#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200112L
#define FTELL ftello
#define FSEEK fseeko
#else
#define FTELL ftell
#define FSEEK fseek
#endif

#include <stdio.h>

#define VGPIRPBIN_PHREDSHIFT 33
#define VGPIRPBIN_HUFFMAN_ESCAPE_CODE 256

#endif
