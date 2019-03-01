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
#include "VGPirpbin_pre.h"
#include "VGPirpbin_Pair.h"

static void swap(uint64_t * a, uint64_t * b)
{
	uint64_t t = *a;
	*a = *b;
	*b = t;
}

void Pair_swap(Pair * A, Pair * B)
{
	swap(&A->key,&B->key);
	swap(&A->value,&B->value);
}

int Pair_comp(const void * A, const void * B)
{
	Pair const * PA = (Pair const *)A;
	Pair const * PB = (Pair const *)B;

	if ( PA->value != PB->value )
	{
		if ( PA->value < PB->value )
			return -1;
		else
			return 1;
	}

	if ( PA->key != PB->key )
	{
		if ( PA->key < PB->key )
			return -1;
		else
			return 1;
	}

	return 0;

	#if 0
	if ( PA->key != PB->key )
		return PA->key < PB->key;
	else
		return PA->value < PB->value;
	#endif
}
