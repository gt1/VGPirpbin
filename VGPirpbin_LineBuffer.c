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
#include "VGPirpbin_LineBuffer.h"

void LineBuffer_deallocate(LineBuffer * LB)
{
	if ( LB )
	{
		free(LB->buffer);
		LB->buffer = NULL;
		free(LB);
	}
}

int64_t LineBuffer_read(LineBuffer * LB, char * p, size_t n)
{
	size_t const r = fread(p,1,n,LB->file);

	if ( feof(LB->file) )
		LB->eof = 1;

	if ( r > 0 )
		return r;
	else if ( LB->eof )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

LineBuffer * LineBuffer_allocate(FILE * f, size_t initsize)
{
	LineBuffer * LB = (LineBuffer *)malloc(sizeof(LineBuffer));
	if ( ! LB )
		return NULL;

	memset(LB,0,sizeof(LineBuffer));

	LB->file = f;

	LB->buffer_n = initsize;
	LB->buffer = (char *)malloc(LB->buffer_n);
	if ( ! LB->buffer )
	{
		free(LB);
		return NULL;
	}

	LB->bufsize = initsize;
	LB->eof = 0;
	LB->bufferptra = LB->buffer;
	LB->bufferptrin = NULL;
	LB->bufferptrout = LB->buffer;
	LB->bufferptre = LB->buffer + LB->buffer_n;
	LB->prevlinelen = 0;
	LB->spos = 0;

	{
		int64_t const r = LineBuffer_read(LB,LB->bufferptra,LB->bufsize);

		if ( r < 0 )
		{
			free(LB->buffer);
			free(LB);
			return NULL;
		}

		LB->bufferptrin = LB->bufferptra + r;
	}

	return LB;
}

size_t LineBuffer_getPos(LineBuffer * LB)
{
	return LB->spos;
}


/**
 * return the next line. If successful the method returns true and the start and end pointers
 * of the line are stored through the pointer a and e respectively.
 *
 * @param a pointer to line start pointer
 * @param e pointer to line end pointer
 * @return true if a line could be extracted, false otherwise
 **/
int LineBuffer_getline(
	LineBuffer * LB,
	char const ** a,
	char const ** e
)
{
	while ( 1 )
	{
		/* end of line pointer */
		char * lineend = LB->bufferptrout;

		/* search for end of buffer or line end */
		while ( lineend != LB->bufferptrin && *(lineend) != '\n' )
			++lineend;

		/* we reached the end of the data currently in memory */
		if ( lineend == LB->bufferptrin )
		{
			/* reached end of file, return what we have */
			if ( LB->eof )
			{
				/* this is the last line we will return */
				if ( LB->bufferptrout != LB->bufferptrin )
				{
					/* if file ends with a newline */
					if ( LB->bufferptrin[-1] == '\n' )
					{
						size_t linelen;
						*a = LB->bufferptrout;
						*e = LB->bufferptrin-1;
						LB->bufferptrout = LB->bufferptrin;

						linelen = (*e-*a)+1;
						LB->prevlinelen = linelen;
						LB->spos += linelen;

						return 1;
					}
					/* otherwise we append an artifical newline */
					else
					{
						size_t const numbytes = lineend - LB->bufferptrout;
						size_t linelen;

						size_t const tmpbuf_n = numbytes+1;
						char * tmpbuf = (char *)malloc(tmpbuf_n);
						if ( ! tmpbuf )
							return -1;

						memcpy(tmpbuf,LB->bufferptrout,numbytes);
						tmpbuf[numbytes] = '\n';

						free(LB->buffer);

						LB->buffer = tmpbuf;
						LB->buffer_n = tmpbuf_n;

						LB->bufsize = numbytes+1;
						LB->bufferptra = LB->buffer;
						LB->bufferptre = LB->buffer + LB->bufsize;
						LB->bufferptrin = LB->bufferptre;
						LB->bufferptrout = LB->bufferptre;

						*a = LB->bufferptra;
						*e = LB->bufferptre - 1;

						linelen = *e-*a;
						LB->prevlinelen = linelen;
						LB->spos += linelen;

						return 1;
					}
				}
				else
				{
					return 0;
				}
			}
			/* we need to read more data */
			else
			{
				/* do we need to extend the buffer? */
				if (
					(LB->bufferptrout == LB->bufferptra)
					&&
					(LB->bufferptrin == LB->bufferptre)
				)
				{
					size_t const newbufsize = LB->bufsize ? (2*LB->bufsize) : 1;

					char * newbuf = (char *)malloc(newbufsize);
					if ( ! newbuf )
						return -1;
					memcpy(newbuf,LB->buffer,LB->buffer_n);
					free(LB->buffer);
					LB->buffer = newbuf;
					LB->buffer_n = newbufsize;

					LB->bufferptre   = LB->buffer + LB->buffer_n;
					LB->bufferptrout = LB->buffer + (LB->bufferptrout - LB->bufferptra);
					LB->bufferptrin  = LB->buffer + (LB->bufferptrin - LB->bufferptra);
					LB->bufferptra   = LB->buffer;
					LB->bufsize      = LB->buffer_n;
				}
				else
				{
					/* move data to front and fill rest of buffer */
					size_t const used   = LB->bufferptrin  - LB->bufferptrout;
					size_t const unused = LB->bufsize - used;
					int64_t r;

					memmove(LB->bufferptra,LB->bufferptrout,used);

					LB->bufferptrout = LB->bufferptra;
					LB->bufferptrin  = LB->bufferptrout + used;

					r = LineBuffer_read(LB,LB->bufferptrin,unused);

					if ( r < 0 )
						return -1;

					LB->bufferptrin += r;
				}
			}
		}
		else
		{
			size_t linelen;
			*a = LB->bufferptrout;
			*e = lineend;
			assert ( *lineend == '\n' );
			LB->bufferptrout = lineend+1;

			linelen = (*e-*a)+1;
			LB->prevlinelen = linelen;
			LB->spos += linelen;

			return 1;
		}
	}

	return 0;
}

void LineBuffer_putback(LineBuffer * LB, char const * a)
{
	LB->spos -= LB->prevlinelen;
	LB->bufferptrout = (char *)a;
	LB->prevlinelen = 0;
}
