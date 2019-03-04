# VGPirpbin
Text to binary and binary to text converter for the IRP file format

# usage:

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
