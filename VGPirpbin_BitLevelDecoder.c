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
#include "VGPirpbin_BitLevelDecoder.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

BitLevelDecoder * BitLevelDecoder_allocate(FILE * in)
{
	BitLevelDecoder * BLD = (BitLevelDecoder *)malloc(sizeof(BitLevelDecoder));
	if ( ! BLD )
		return NULL;

	BLD->in = in;
	BLD->c = 0;
	BLD->f = 0;

	return BLD;
}

int BitLevelDecoder_seek(BitLevelDecoder * BLD, uint64_t const p)
{
	uint64_t const b = p/CHAR_BIT;
	uint64_t r = p - b * CHAR_BIT;

	BLD->c = 0;
	BLD->f = 0;

	if ( FSEEK(BLD->in,b,SEEK_SET) < 0 )
		return -1;

	while ( r )
		if ( BitLevelDecoder_getBit(BLD) < 0 )
			return -1;
		else
			r -= 1;

	return 0;
}

int BitLevelDecoder_load(BitLevelDecoder * BLV)
{
	assert ( BLV->f == 0 );

	if ( fread(&BLV->c,sizeof(BLV->c),1,BLV->in) != 1 )
		return -1;

	BLV->f = sizeof(BLV->c) * CHAR_BIT;

	return 0;
}

int BitLevelDecoder_getBit(BitLevelDecoder * BLV)
{
	int b;

	if ( BLV->f == 0 )
	{
		if ( BitLevelDecoder_load(BLV) != 0 )
			return -1;
	}

	b = (BLV->c >> (--BLV->f)) & 1;

	return b;
}

int BitLevelDecoder_decodeGamma(BitLevelDecoder * BLV, uint64_t * v)
{
	unsigned int l = 0;


	while ( 1 )
	{
		int const r = BitLevelDecoder_getBit(BLV);

		if ( r < 0 )
			return -1;

		if ( r )
			break;
		else
			l += 1;
	}

	*v = 0;

	while ( l-- )
	{
		int const r = BitLevelDecoder_getBit(BLV);

		if ( r < 0 )
			return -1;

		*v <<= 1;
		*v |= r;
	}

	return 0;
}

int BitLevelDecoder_decode(BitLevelDecoder * BLV, uint64_t * v, unsigned int l)
{
	*v = 0;

	while ( l-- )
	{
		int const r = BitLevelDecoder_getBit(BLV);

		if ( r < 0 )
			return -1;

		*v <<= 1;
		*v |= r;
	}

	return 0;
}

void BitLevelDecoder_deallocate(BitLevelDecoder * BLV)
{
	if ( BLV )
		free(BLV);
}

char * BitLevelDecoder_decodeString(BitLevelDecoder * BLD)
{
	uint64_t v;
	uint64_t l;
	uint64_t i;
	char * c = NULL;

	if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		return NULL;

	l = v;

	c = (char *)malloc(sizeof(char)*(l+1));
	if ( ! c )
		return NULL;

	c[l] = 0;

	for ( i = 0; i < l; ++i )
	{
		if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		{
			free(c);
			return NULL;
		}
		else
			c[i] = v;
	}

	return c;
}
