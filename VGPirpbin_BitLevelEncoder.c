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
#include "VGPirpbin_BitLevelEncoder.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

uint64_t BitLevelEncoder_getOffset(BitLevelEncoder const * A)
{
	return CHAR_BIT * A->w + A->f;
}

BitLevelEncoder * BitLevelEncoder_allocate(FILE * out)
{
	BitLevelEncoder * enc = (BitLevelEncoder *)malloc(sizeof(BitLevelEncoder));
	if ( ! enc )
		return NULL;

	enc->out = out;
	enc->c = 0;
	enc->f = 0;
	enc->w = 0;

	return enc;
}

int BitLevelEncoder_write(BitLevelEncoder * enc)
{
	if (
		fwrite(
			&(enc->c),
			sizeof(enc->c),
			1,
			enc->out
		)
		!=
		1
	)
	{
		return -1;
	}

	enc->f = 0;
	enc->c = 0;
	enc->w += 1;

	return 0;
}

int BitLevelEncoder_flush(BitLevelEncoder * enc)
{
	if ( enc->f )
	{
		unsigned int const cspace = sizeof(enc->c) * CHAR_BIT;
		unsigned int space = cspace - enc->f;

		enc->c <<= space;
		enc->f += space;

		if ( BitLevelEncoder_write(enc) != 0 )
			return -1;
	}

	return 0;
}

int BitLevelEncoder_encode(BitLevelEncoder * enc, uint64_t codeword, unsigned int codelength)
{
	while ( codelength )
	{
		/* space in output word */
		unsigned int const cspace = sizeof(enc->c) * CHAR_BIT;
		/* rest of space in current word */
		unsigned int space = cspace - enc->f;
		/* number of bits to put */
		unsigned int const toput = codelength <= space ? codelength : space;
		/* number of unencoded bits in codeword */
		unsigned int const unencoded = codelength - toput;

		assert ( space );

		enc->c <<= toput;
		enc->c |= (codeword >> unencoded);
		enc->f += toput;

		codelength -= toput;

		if ( enc->f == cspace )
		{
			if ( BitLevelEncoder_write(enc) != 0 )
				return -1;
		}
	}

	return 0;
}

int BitLevelEncoder_encodeGamma(BitLevelEncoder * enc, uint64_t v)
{
	unsigned int l = 0;
	uint64_t vv = v;

	while ( vv )
	{
		vv >>= 1;
		l += 1;
	}

	/* write length of number */
	if ( BitLevelEncoder_encode(enc,1 /* code */,l+1 /* length */) != 0 )
		return -1;

	/* write number */
	if ( BitLevelEncoder_encode(enc,v /* code */,l /* length */) != 0 )
		return -1;

	return 0;
}

int BitLevelEncoder_deallocate(BitLevelEncoder * enc)
{
	int r = 0;
	if ( enc )
	{
		r = BitLevelEncoder_flush(enc);
		free(enc);
	}

	return r;
}

int BitLevelEncoder_encodeString(BitLevelEncoder * BLE, char const * c)
{
	size_t const l = strlen(c);
	size_t i;

	if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
		return -1;

	for ( i = 0; i < l; ++i )
		if ( BitLevelEncoder_encode(BLE,c[i],8) < 0 )
			return -1;

	return 0;
}

int BitLevelEncoder_encodeStringP(BitLevelEncoder * BLE, char const * c, char const * e)
{
	size_t const l = e-c;
	size_t i;

	if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
		return -1;

	for ( i = 0; i < l; ++i )
		if ( BitLevelEncoder_encode(BLE,c[i],8) < 0 )
			return -1;

	return 0;
}
