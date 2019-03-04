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
#include "VGPirpbin_getBinaryFileType.h"
#include "VGPirpbin_Arguments.h"
#include "VGPirpbin_produceBinary.h"
#include "VGPirpbin_decodeBinaryFile.h"

#include <stdlib.h>
#include <string.h>

/* program version */
static char const * version = "0.0";

int main(int argc, char * argv[])
{
	char const * infn;
	char const * outfn;
	char const * command;
	int64_t maxstatlines = INT64_MAX;
	ProvenanceStep * PS = NULL;
	Arguments * A = NULL;
	int returncode = EXIT_SUCCESS;

	if ( !(PS = ProvenanceStep_create(argc,argv,version)) )
	{
		fprintf(stderr,"[E] unable to create ProvenanceStep object\n");
		returncode = EXIT_FAILURE;
		goto cleanup;
	}

	if ( !(A = Arguments_parse(argc,argv)) )
	{
		fprintf(stderr,"[E] unable to parse arguments\n");
		returncode = EXIT_FAILURE;
		goto cleanup;
	}

	if ( A->numpos < 1 )
	{
		fprintf(stderr,"usage: %s <command>\n", argv[0]);
		fprintf(stderr,"\n");
		fprintf(stderr," commands:\n");
		fprintf(stderr,"\n");
		fprintf(stderr," * a2b: convert irp file to binary\n");
		fprintf(stderr," * b2a: convert binary file to irp\n");
		fprintf(stderr,"\n");
		returncode = EXIT_FAILURE;
		goto cleanup;
	}

	command = A->posArgs[0];

	if ( strcmp(command,"a2b") == 0 )
	{
		if ( A->numpos < 3 )
		{
			fprintf(stderr,"usage: %s a2b <out> <in>\n", argv[0]);
			returncode = EXIT_FAILURE;
			goto cleanup;
		}

		outfn = A->posArgs[1];
		infn  = A->posArgs[2];
		maxstatlines = Arguments_haveNonPosInteger(A,"--maxstatlines") ? Arguments_getNonPosInteger(A,"--maxstatlines",-1) : INT64_MAX;

		fprintf(stderr,"[V] output file name %s\n", outfn);
		fprintf(stderr,"[V] input file name %s\n", infn);
		fprintf(stderr,"[V] maxstatlines %ld\n", (long)maxstatlines);

		if ( VGP_IRPBIN_produceBinaryFile(outfn,infn,maxstatlines,&PS,getBinaryFileType()) < 0 )
		{
			fprintf(stderr,"[E] failed to VGP_IRPBIN_produce binary file\n");
			returncode = EXIT_FAILURE;
			goto cleanup;
		}
	}
	else if ( strcmp(command,"b2a") == 0 )
	{
		if ( A->numpos < 2 )
		{
			fprintf(stderr,"usage: %s b2a <in>\n", argv[0]);
			returncode = EXIT_FAILURE;
			goto cleanup;
		}

		infn = A->posArgs[1];

		if ( VGP_IRPBIN_decodeBinaryFile(infn,&PS) < 0 )
		{
			fprintf(stderr,"[E] failed to check binary file\n");
			returncode = EXIT_FAILURE;
			goto cleanup;
		}
	}
	else
	{
		fprintf(stderr,"[E] unknown command %s\n",command);
		returncode = EXIT_FAILURE;
		goto cleanup;
	}

	cleanup:
	ProvenanceStep_deallocate(PS);
	Arguments_deallocate(A);

	return returncode;
}
