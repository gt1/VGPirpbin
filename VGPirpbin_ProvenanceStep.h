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
#if ! defined(VGPIRPBIN_PROVENANCESTEP_H)
#define VGPIRPBIN_PROVENANCESTEP_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_BitLevelEncoder.h"
#include "VGPirpbin_BitLevelDecoder.h"
#include "VGPirpbin_CString.h"

typedef struct _ProvenanceStep
{
	char * program;
	char * version;
	char * commandline;
	char * date;
	struct _ProvenanceStep * next;
} ProvenanceStep;

ProvenanceStep * ProvenanceStep_create(int argc, char * argv[], char const * version);
int ProvenanceStep_print(FILE * out, ProvenanceStep * P);
void ProvenanceStep_deallocate(ProvenanceStep * P);
ProvenanceStep * ProvenanceStep_allocate(
	CString const * a,
	CString const * b,
	CString const * c,
	CString const * d
);
void ProvenanceStep_add(ProvenanceStep ** Q, ProvenanceStep ** L, ProvenanceStep * P);
int ProvenanceStep_encode(BitLevelEncoder * BLE, ProvenanceStep * P);
ProvenanceStep * ProvenanceStep_decode(BitLevelDecoder * BLD);
#endif
