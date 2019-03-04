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
