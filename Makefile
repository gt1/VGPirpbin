CC ?= cc
CFLAGS ?= -O3 -g -ansi -pedantic
CPPFLAGS ?= -W -Wall

BINARIES=VGPirpbin
LIBRARIES=libVGPirpbin.a

all: ${BINARIES}

LIBHEADERS = \
	VGPirpbin_Arguments.h \
	VGPirpbin_BitLevelDecoder.h \
	VGPirpbin_BitLevelEncoder.h \
	VGPirpbin_CodeTableEntry.h \
	VGPirpbin_CodeTable.h \
	VGPirpbin_CString.h \
	VGPirpbin_decodeBinaryFile.h \
	VGPirpbin_DecodeResult.h \
	VGPirpbin_expect.h \
	VGPirpbin_getBinaryFileType.h \
	VGPirpbin_getNumber.h \
	VGPirpbin_getQualityCode.h \
	VGPirpbin_getQualityTable.h \
	VGPirpbin_HeaderStatsLine.h \
	VGPirpbin_HuffmanCode.h \
	VGPirpbin_HuffmanDecodeQueueEntry.h \
	VGPirpbin_HuffmanInnerNode.h \
	VGPirpbin_HuffmanInnerTable.h \
	VGPirpbin_IRPBinDecoderContext.h \
	VGPirpbin_IRPBINDecoder.h \
	VGPirpbin_IRPBINDecoderInput.h \
	VGPirpbin_LineBuffer.h \
	VGPirpbin_mconcat.h \
	VGPirpbin_mstrdup.h \
	VGPirpbin_Pair.h \
	VGPirpbin_PairTable.h \
	VGPirpbin_pre.h \
	VGPirpbin_produceBinary.h \
	VGPirpbin_ProvenanceStep.h \
	VGPirpbin_QualityHuffman.h \
	VGPirpbin_Table.h \

VGPirpbin_Arguments.o: VGPirpbin_Arguments.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_Arguments.c -o VGPirpbin_Arguments.o
VGPirpbin_BitLevelDecoder.o: VGPirpbin_BitLevelDecoder.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_BitLevelDecoder.c -o VGPirpbin_BitLevelDecoder.o
VGPirpbin_BitLevelEncoder.o: VGPirpbin_BitLevelEncoder.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_BitLevelEncoder.c -o VGPirpbin_BitLevelEncoder.o
VGPirpbin_CodeTable.o: VGPirpbin_CodeTable.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_CodeTable.c -o VGPirpbin_CodeTable.o
VGPirpbin_CodeTableEntry.o: VGPirpbin_CodeTableEntry.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_CodeTableEntry.c -o VGPirpbin_CodeTableEntry.o
VGPirpbin_CString.o: VGPirpbin_CString.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_CString.c -o VGPirpbin_CString.o
VGPirpbin_decodeBinaryFile.o: VGPirpbin_decodeBinaryFile.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_decodeBinaryFile.c -o VGPirpbin_decodeBinaryFile.o
VGPirpbin_DecodeResult.o: VGPirpbin_DecodeResult.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_DecodeResult.c -o VGPirpbin_DecodeResult.o
VGPirpbin_expect.o: VGPirpbin_expect.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_expect.c -o VGPirpbin_expect.o
VGPirpbin_getBinaryFileType.o: VGPirpbin_getBinaryFileType.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getBinaryFileType.c -o VGPirpbin_getBinaryFileType.o
VGPirpbin_getNumber.o: VGPirpbin_getNumber.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getNumber.c -o VGPirpbin_getNumber.o
VGPirpbin_getQualityCode.o: VGPirpbin_getQualityCode.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getQualityCode.c -o VGPirpbin_getQualityCode.o
VGPirpbin_getQualityTable.o: VGPirpbin_getQualityTable.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getQualityTable.c -o VGPirpbin_getQualityTable.o
VGPirpbin_HeaderStatsLine.o: VGPirpbin_HeaderStatsLine.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HeaderStatsLine.c -o VGPirpbin_HeaderStatsLine.o
VGPirpbin_HuffmanCode.o: VGPirpbin_HuffmanCode.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HuffmanCode.c -o VGPirpbin_HuffmanCode.o
VGPirpbin_HuffmanDecodeQueueEntry.o: VGPirpbin_HuffmanDecodeQueueEntry.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HuffmanDecodeQueueEntry.c -o VGPirpbin_HuffmanDecodeQueueEntry.o
VGPirpbin_HuffmanInnerTable.o: VGPirpbin_HuffmanInnerTable.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HuffmanInnerTable.c -o VGPirpbin_HuffmanInnerTable.o
VGPirpbin_IRPBINDecoder.o: VGPirpbin_IRPBINDecoder.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_IRPBINDecoder.c -o VGPirpbin_IRPBINDecoder.o
VGPirpbin_IRPBinDecoderContext.o: VGPirpbin_IRPBinDecoderContext.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_IRPBinDecoderContext.c -o VGPirpbin_IRPBinDecoderContext.o
VGPirpbin_IRPBINDecoderInput.o: VGPirpbin_IRPBINDecoderInput.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_IRPBINDecoderInput.c -o VGPirpbin_IRPBINDecoderInput.o
VGPirpbin_LineBuffer.o: VGPirpbin_LineBuffer.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_LineBuffer.c -o VGPirpbin_LineBuffer.o
VGPirpbin_main.o: VGPirpbin_main.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_main.c -o VGPirpbin_main.o
VGPirpbin_mconcat.o: VGPirpbin_mconcat.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_mconcat.c -o VGPirpbin_mconcat.o
VGPirpbin_mstrdup.o: VGPirpbin_mstrdup.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_mstrdup.c -o VGPirpbin_mstrdup.o
VGPirpbin_Pair.o: VGPirpbin_Pair.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_Pair.c -o VGPirpbin_Pair.o
VGPirpbin_PairTable.o: VGPirpbin_PairTable.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_PairTable.c -o VGPirpbin_PairTable.o
VGPirpbin_produceBinary.o: VGPirpbin_produceBinary.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_produceBinary.c -o VGPirpbin_produceBinary.o
VGPirpbin_ProvenanceStep.o: VGPirpbin_ProvenanceStep.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_ProvenanceStep.c -o VGPirpbin_ProvenanceStep.o
VGPirpbin_QualityHuffman.o: VGPirpbin_QualityHuffman.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_QualityHuffman.c -o VGPirpbin_QualityHuffman.o
VGPirpbin_Table.o: VGPirpbin_Table.c ${LIBHEADERS}
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_Table.c -o VGPirpbin_Table.o

LIBOBJECTS=VGPirpbin_Arguments.o \
	VGPirpbin_BitLevelDecoder.o \
	VGPirpbin_BitLevelEncoder.o \
	VGPirpbin_CodeTable.o \
	VGPirpbin_CodeTableEntry.o \
	VGPirpbin_CString.o \
	VGPirpbin_decodeBinaryFile.o \
	VGPirpbin_DecodeResult.o \
	VGPirpbin_expect.o \
	VGPirpbin_getBinaryFileType.o \
	VGPirpbin_getNumber.o \
	VGPirpbin_getQualityCode.o \
	VGPirpbin_getQualityTable.o \
	VGPirpbin_HeaderStatsLine.o \
	VGPirpbin_HuffmanCode.o \
	VGPirpbin_HuffmanDecodeQueueEntry.o \
	VGPirpbin_HuffmanInnerTable.o \
	VGPirpbin_IRPBINDecoder.o \
	VGPirpbin_IRPBinDecoderContext.o \
	VGPirpbin_IRPBINDecoderInput.o \
	VGPirpbin_LineBuffer.o \
	VGPirpbin_mconcat.o \
	VGPirpbin_mstrdup.o \
	VGPirpbin_Pair.o \
	VGPirpbin_PairTable.o \
	VGPirpbin_produceBinary.o \
	VGPirpbin_ProvenanceStep.o \
	VGPirpbin_QualityHuffman.o \
	VGPirpbin_Table.o

libVGPirpbin.a: ${LIBOBJECTS}
	ar rc libVGPirpbin.a ${LIBOBJECTS}

VGPirpbin: VGPirpbin_main.o libVGPirpbin.a 
	${CC} -ansi -pedantic -W -Wall -O3 -g VGPirpbin_main.o -o VGPirpbin libVGPirpbin.a

clean:
	rm -f ${LIBOBJECTS} VGPirpbin_main.o
	rm -f libVGPirpbin.a

distclean: clean
	rm -f VGPirpbin

DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/lib

install: ${LIBRARIES} VGPirpbin
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(LIBDIR)
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp ${LIBRARIES} $(DESTDIR)$(LIBDIR)/
	cp ${LIBHEADERS} $(DESTDIR)$(INCLUDEDIR)/
	cp ${BINARIES} $(DESTDIR)$(BINDIR)

#	cc -W -Wall -O3 -ansi -pedantic VGPirpbin.c -o VGPirpbin
