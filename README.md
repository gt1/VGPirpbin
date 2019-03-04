# VGPirpbin
Text to binary and binary to text converter for the IRP file format

## usage

Convert IRP text to binary

```
VGPirpbin a2b reads.bsq reads.irp
```

This conversion performs multiple scans of the file reads.irp to compute
statistics for redundancy coding, so reads.irp needs to be a regular file.
The maximum number of lines scanned for obtaining statistics can be set
using the maxstatlines swich (i.e. set e.g. --maxstatlines50000).

Convert binary IRP to text

```
VGPirpbin b2a reads.bsq > reads.irp
```

The binary to text conversion supports ranges via the --from and --to
switches, e.g.:

```
VGPirpbin b2a --from50 --to100 reads.bsq > reads.irp
```

Note that not converting the whole binary file to text form will produce a
file without a header.

## building

The VGPirpbin comes with a Makefile which will build the VGPirpbin and an
accompanying library for accessing binary IRP files from C programs. For
building those just run make. The C compiler used can be set via the CC
variable, e.g.

```
CC=gcc make
```

## installation

The package can be installed via

```
make install
```

The default installation path is /usr/local. This can be changed using the
PREFIX variable when calling make install:

```
PREFIX=${HOME}/vgp make install
```

## using the library in C code

Here is a short example for accessing binary IRP files from C code. The code
reads a binary IRP file and outputs the records in FastA format.

```
#include "VGPirpbin_decodeBinaryFile.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
	int returncode = EXIT_SUCCESS;
        IRPBINDecoder * I = NULL;
        IRPBinDecoderContext * context = NULL;
        char const * fn = NULL;
        int done = 0;
        unsigned long id = 0;

	if ( argc < 2 )
	{
		fprintf(stderr,"usage: %s <in>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	fn = argv[1];

	/* open file */
	if ( ! (I = IRPBINDecoder_allocateFromFile(fn)) )
	{
		fprintf(stderr,"Unable to open file\n");
		returncode = EXIT_FAILURE;
                goto cleanup;
        }

	/* get decoder context */
        if (!(context = IRPBINDecoder_getContext(I)))
        {
                returncode = -1;
                goto cleanup;
        }
        
        /* print number of pairs in file */
        fprintf(stderr,"found %lu pairs in file\n", I->nr);
        
        while ( ! done )
        {
		int const r0 = IRPBINDecoder_decodePair(I,context);
        
		/* decode error */
		if ( r0 < 0 )
		{
			fprintf(stderr,"Decoding failed\n");
			returncode = EXIT_FAILURE;
			goto cleanup;
		}
		/* end of file */
		else if ( r0 == 0 )
		{
			done = 1;
		}
		/* pair */
		else if ( r0 == 1 )
		{
			/* first mate (0 terminated) */
			char const * seq_a = context->DF->S;
			/* length of forward sequence */
			/* uint64_t seq_a_len = context->DF->S_l; */
			fprintf(stdout,">%lu/1\n",id);
			fprintf(stdout,"%s\n",seq_a);

			/* second mate (0 terminated) */
			char const * seq_b = context->DR->S;
			/* length of forward sequence */
			/* uint64_t seq_b_len = context->DR->S_l; */
			fprintf(stdout,">%lu/2\n",id);
			fprintf(stdout,"%s\n",seq_b);
			
			id += 1;
		}
        }

	cleanup:
	IRPBinDecoderContext_deallocate(context);
	IRPBINDecoder_deallocate(I);

	return returncode;
}
```

Assuming the code is saved as IRPtoFastA.c, the program can be compiled
using

```
cc -I${HOME}/vgp/include IRPtoFastA.c -o IRPtoFastA -L${HOME}/vgp/lib -lVGPirpbin
```

if VGPirpbin has previously been installed at ${HOME}/vgp (using
`PREFIX=$HOME/vgp make install`).
