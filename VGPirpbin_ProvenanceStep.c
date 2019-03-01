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
#include "VGPirpbin_ProvenanceStep.h"
#include "VGPirpbin_BitLevelDecoder.h"
#include "VGPirpbin_BitLevelEncoder.h"
#include "VGPirpbin_CString.h"
#include "VGPirpbin_mstrdup.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

ProvenanceStep * ProvenanceStep_create(int argc, char * argv[], char const * version)
{
	ProvenanceStep * P = NULL;
	time_t t;
	char * p;
	char * clp;
	size_t l;
	size_t ll = 0;
	int i;

	for ( i = 0; i < argc; ++i )
		ll += strlen(argv[i]);
	ll += argc;


	P = (ProvenanceStep *)malloc(sizeof(ProvenanceStep));

	if ( ! P )
		goto cleanup;

	memset(P,0,sizeof(ProvenanceStep));

	P->commandline = (char *)malloc(ll*sizeof(char));

	if ( ! P->commandline )
		goto cleanup;


	clp = P->commandline;
	for ( i = 0; i < argc; ++i )
	{
		size_t const l = strlen(argv[i]);
		memcpy(clp,argv[i],l);
		clp += l;
		if ( i + 1 == argc )
			*(clp++) = 0;
		else
			*(clp++) = ' ';
	}

	P->program = mstrdup(argv[0]);

	if ( ! P->program )
		goto cleanup;


	P->version = mstrdup(version);

	if ( ! P->version )
		goto cleanup;

	t = time(NULL);
	p = ctime(&t);

	P->date = mstrdup(p);

	l = strlen(P->date);

	while ( l && isspace(P->date[l-1]) )
		--l;

	P->date[l] = 0;

	return P;

	cleanup:
	if ( P )
	{
		free(P->date);
		free(P->commandline);
		free(P->version);
		free(P->program);
		free(P);
	}
	return NULL;
}

int ProvenanceStep_print(FILE * out, ProvenanceStep * P)
{
	while ( P )
	{
		if ( fprintf(out,"!") < 0 )
			return -1;

		if ( fprintf(out," %lu %s",(unsigned long)strlen(P->program),P->program) < 0 )
			return -1;
		if ( fprintf(out," %lu %s",(unsigned long)strlen(P->version),P->version) < 0 )
			return -1;
		if ( fprintf(out," %lu %s",(unsigned long)strlen(P->commandline),P->commandline) < 0 )
			return -1;
		if ( fprintf(out," %lu %s",(unsigned long)strlen(P->date),P->date) < 0 )
			return -1;
		if ( fprintf(out,"\n") < 0 )
			return -1;

		P = P->next;
	}

	return 0;
}

void ProvenanceStep_deallocate(ProvenanceStep * P)
{
	while ( P )
	{
		ProvenanceStep * next = P->next;

		free(P->program);
		free(P->version);
		free(P->commandline);
		free(P->date);
		free(P);

		P = next;
	}
}

ProvenanceStep * ProvenanceStep_allocate(
	CString const * a,
	CString const * b,
	CString const * c,
	CString const * d
)
{
	ProvenanceStep * P = NULL;

	if ( !(P = (ProvenanceStep *)malloc(sizeof(ProvenanceStep))) )
	{
		return NULL;
	}

	memset(P,0,sizeof(*P));

	P->program = CString_tostring(a);
	P->version = CString_tostring(b);
	P->commandline = CString_tostring(c);
	P->date = CString_tostring(d);
	P->next = NULL;

	if (
		!(P->program)
		||
		!(P->version)
		||
		!(P->commandline)
		||
		!(P->date)
	)
	{
		ProvenanceStep_deallocate(P);
		return NULL;
	}

	return P;
}

void ProvenanceStep_add(ProvenanceStep ** Q, ProvenanceStep ** L, ProvenanceStep * P)
{
	if ( ! *Q )
	{
		*Q = P;
		*L = P;
	}
	else
	{
		assert ( *L );
		assert ( ! ((*L)->next) );

		(*L)->next = P;
		*L = P;
	}
}

int ProvenanceStep_encode(BitLevelEncoder * BLE, ProvenanceStep * P)
{
	ProvenanceStep * PP = P;
	uint64_t n = 0;

	while ( PP )
	{
		n += 1;
		PP = PP->next;
	}

	if ( BitLevelEncoder_encodeGamma(BLE,n) < 0 )
		return -1;

	while ( P )
	{
		if ( BitLevelEncoder_encodeString(BLE,P->program) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeString(BLE,P->version) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeString(BLE,P->commandline) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeString(BLE,P->date) < 0 )
			return -1;

		P = P->next;
	}

	return 0;
}

ProvenanceStep * ProvenanceStep_decode(BitLevelDecoder * BLD)
{
	uint64_t v;
	uint64_t n;
	uint64_t i;
	ProvenanceStep * PA = NULL;
	ProvenanceStep * PE = NULL;

	if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		return NULL;

	n = v;

	for ( i = 0; i < n; ++i )
	{
		char * program = NULL;
		char * version = NULL;
		char * commandline = NULL;
		char * date = NULL;
		ProvenanceStep * P = NULL;

		program = BitLevelDecoder_decodeString(BLD);
		version = BitLevelDecoder_decodeString(BLD);
		commandline = BitLevelDecoder_decodeString(BLD);
		date = BitLevelDecoder_decodeString(BLD);

		if ( ! program || !version || ! commandline || !date )
		{
			free(program);
			free(version);
			free(commandline);
			free(date);
		}

		P = (ProvenanceStep *)malloc(sizeof(ProvenanceStep));

		if ( ! P )
		{
			free(program);
			free(version);
			free(commandline);
			free(date);
			ProvenanceStep_deallocate(PA);
			return NULL;
		}

		P->program = program;
		P->version = version;
		P->commandline = commandline;
		P->date = date;
		P->next = NULL;

		if ( ! PA )
		{
			PA = PE = P;
		}
		else
		{
			PE->next = P;
			PE = P;
		}
	}

	return PA;
}
