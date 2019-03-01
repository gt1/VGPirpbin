
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
#include "VGPirpbin_Table.h"
#include <stdlib.h>
#include <assert.h>

Table * Table_allocate(size_t size)
{
	size_t i;
	Table * T = (Table *)malloc(sizeof(Table));
	if ( ! T )
		return NULL;

	T->A = NULL;
	T->n = size;

	T->A = (uint64_t *)malloc(size * sizeof(uint64_t));
	if ( ! T->A )
	{
		free(T);
		return NULL;
	}

	for ( i = 0; i < size; ++i )
		T->A[i] = 0;

	return T;
}

int Table_increment(Table * T, size_t i)
{
	while ( i >= T->n )
	{
		size_t newsize = T->n ? 2*T->n : 1;
		size_t j;
		Table * NT = Table_allocate(newsize);
		if ( ! NT )
			return -1;

		for ( j = 0; j < T->n; ++j )
			NT->A[j] = T->A[j];

		free(T->A);
		T->A = NT->A;
		T->n = NT->n;

		free(NT);
	}

	assert ( i < T->n );

	T->A[i] ++;

	return 0;
}

void Table_deallocate(Table * T)
{
	if ( T )
	{
		free(T->A);
		free(T);
	}
}
