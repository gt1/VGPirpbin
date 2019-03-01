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
#include "VGPirpbin_Arguments.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void Arguments_deallocate(Arguments * A)
{
	if ( A )
	{
		free(A->posArgs);
		free(A->nonPosArgs);
		free(A);
	}
}

int Arguments_argStringCompare(void const * A, void const * B)
{
	char const * CA = *((char const **)A);
	char const * CB = *((char const **)B);

	return strcmp(CA,CB);
}

Arguments * Arguments_parse(int const argc, char * argv[])
{
	Arguments * A = NULL;
	int i = 0;
	int parse;
	int numpos;
	int numnonpos;

	A = (Arguments *)malloc(sizeof(Arguments));

	if ( ! A )
		return NULL;

	memset(A,0,sizeof(Arguments));

	if ( argc )
		A->progname = argv[0];

	parse = 1;
	numpos = 0;
	numnonpos = 0;
	for ( i = 1; i < argc; ++i )
	{
		if ( (! parse) || argv[i][0] != '-' )
		{
			numpos += 1;
		}
		else if ( argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 )
		{
			parse = 0;
		}
		else
		{
			numnonpos += 1;
		}
	}

	A->numpos = numpos;
	A->numnonpos = numnonpos;

	A->posArgs = (char **)malloc(numpos * sizeof(char *));
	A->nonPosArgs = (char **)malloc(numnonpos * sizeof(char *));

	parse = 1;
	numpos = 0;
	numnonpos = 0;

	for ( i = 1; i < argc; ++i )
	{
		if ( (! parse) || argv[i][0] != '-' )
		{
			A->posArgs[numpos++] = argv[i];
		}
		else if ( argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 )
		{
			parse = 0;
		}
		else
		{
			A->nonPosArgs[numnonpos++] = argv[i];
		}
	}

	/* sort non positional arguments */
	qsort(A->nonPosArgs,A->numnonpos,sizeof(char const *),Arguments_argStringCompare);

	for ( i = 0; i < A->numnonpos; ++i )
	{
		fprintf(stderr,"nonpos[%d]=%s\n",i,A->nonPosArgs[i]);
	}

	return A;
}

int64_t Arguments_getNonPosInteger(Arguments const * A, char const * prefix, int64_t const def)
{
	int i;
	size_t const lprefix = strlen(prefix);

	for ( i = 0; i < A->numnonpos; ++i )
		if ( strncmp(prefix,A->nonPosArgs[i],lprefix) == 0 )
		{
			char const * c = A->nonPosArgs[i] + lprefix;
			int64_t n = 0;
			unsigned int ni = 0;

			while ( isdigit(*c) )
			{
				n *= 10;
				n += *c - '0';
				++c;
				++ni;
			}

			if ( *c == 0 && ni != 0 )
				return n;

			fprintf(stderr,"[E] ignoring unparsable argument %s\n", A->nonPosArgs[i]);
		}

	return def;
}

int Arguments_haveNonPosInteger(Arguments const * A, char const * prefix)
{
	return Arguments_getNonPosInteger(A,prefix,-1) != -1;
}
