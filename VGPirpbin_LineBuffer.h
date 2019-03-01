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
#if ! defined(VGPIRPBIN_LINEBUFFER_H)
#define VGPIRPBIN_LINEBUFFER_H

#include "VGPirpbin_pre.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct _LineBuffer
{
	/* input stream */
	FILE * file;

	/* buffer */
	char * buffer;
	size_t buffer_n;

	/* size of buffer */
	size_t bufsize;
	/* set to true when end of file has been observed */
	int eof;

	/* start of buffer pointer */
	char * bufferptra;
	/* end of input data */
	char * bufferptrin;
	/* current read pointer */
	char * bufferptrout;
	/* end of buffer pointer */
	char * bufferptre;

	size_t prevlinelen;
	size_t spos;
} LineBuffer;

int64_t LineBuffer_read(LineBuffer * LB, char * p, size_t n);
LineBuffer * LineBuffer_allocatew(FILE * f, size_t initsize);
size_t LineBuffer_getPos(LineBuffer * LB);
int LineBuffer_getline(LineBuffer * LB, char const ** a, char const ** e);
void LineBuffer_putback(LineBuffer * LB, char const * a);
void LineBuffer_deallocate(LineBuffer * LB);
LineBuffer * LineBuffer_allocate(FILE * f, size_t initsize);
#endif
