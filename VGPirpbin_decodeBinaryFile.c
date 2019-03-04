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
#include "VGPirpbin_decodeBinaryFile.h"
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int VGP_IRPBIN_decodeBinaryFile(char const * fn, ProvenanceStep ** insPS, char const * binfiletype)
{
	int returncode = 0;
	uint64_t iii = 0;
	uint64_t s = 0;
	time_t t;
	time_t t0;
	IRPBinDecoderContext * context = NULL;

	IRPBINDecoder * I = NULL;

	if ( ! (I = IRPBINDecoder_allocateFromFile(fn,binfiletype)) )
	{
		returncode = -1;
		goto cleanup;
	}

	IRPBINDecoder_addStep(I,insPS);

	if (!(context = IRPBinDecoderContext_allocate()))
	{
		returncode = -1;
		goto cleanup;
	}

	if ( IRPBINDecoder_printHeader(I,stdout) < 0 )
	{
		returncode = -1;
		goto cleanup;
	}

	/* IRPBINDecoder_seek(I,0); */

	t = time(NULL);
	t0 = t;
	while ( iii < I->nr )
	{
		int r0;
		r0 = IRPBINDecoder_decodePair(I,context);

		if ( r0 < 0 )
		{
			fprintf(stderr,"[E] failed to decode pair\n");
			goto cleanup;
		}
		else if ( r0 > 0 )
		{
			/* fprintf(stderr,"[V] found group\n"); */
			fprintf(stdout,"g %lu %lu %s\n", context->groupsize, strlen(context->groupname), context->groupname);

			free(context->groupname);
			context->groupname = NULL;

			continue;
		}
		else
		{
			assert ( r0 == 0 );
		}

		if ( IRPBinDecoderContext_printPair(context, stdout) < 0 )
		{
			fprintf(stderr,"[E] failed to print pair\n");
			goto cleanup;
		}

		s += context->DF->S_o;
		s += context->DR->S_o;
		s += context->DF->Q_o;
		s += context->DR->Q_o;

		iii += 1;
		if ( iii % (1024*1024)  == 0 )
		{
			time_t const tn = time(NULL);

			fprintf(stderr,"[V] decoded %lu time %lu acc time %lu syms %lu syms/sec %f\n",(unsigned long)iii,(unsigned long)(tn-t),(unsigned long)(tn-t0),(unsigned long)s, s/(double)(tn-t0));

			t = tn;
		}
	}

	cleanup:
	IRPBINDecoder_deallocate(I);
	IRPBinDecoderContext_deallocate(context);

	return returncode;
}
