CC ?= cc
CFLAGS ?= -O3 -g
CPPFLAGS ?= -W -Wall

all: VGPirpbin

VGPirpbin_Arguments.o: VGPirpbin_Arguments.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_Arguments.c -o VGPirpbin_Arguments.o
VGPirpbin_BitLevelDecoder.o: VGPirpbin_BitLevelDecoder.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_BitLevelDecoder.c -o VGPirpbin_BitLevelDecoder.o
VGPirpbin_BitLevelEncoder.o: VGPirpbin_BitLevelEncoder.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_BitLevelEncoder.c -o VGPirpbin_BitLevelEncoder.o
VGPirpbin_CodeTable.o: VGPirpbin_CodeTable.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_CodeTable.c -o VGPirpbin_CodeTable.o
VGPirpbin_CodeTableEntry.o: VGPirpbin_CodeTableEntry.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_CodeTableEntry.c -o VGPirpbin_CodeTableEntry.o
VGPirpbin_CString.o: VGPirpbin_CString.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_CString.c -o VGPirpbin_CString.o
VGPirpbin_decodeBinaryFile.o: VGPirpbin_decodeBinaryFile.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_decodeBinaryFile.c -o VGPirpbin_decodeBinaryFile.o
VGPirpbin_DecodeResult.o: VGPirpbin_DecodeResult.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_DecodeResult.c -o VGPirpbin_DecodeResult.o
VGPirpbin_expect.o: VGPirpbin_expect.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_expect.c -o VGPirpbin_expect.o
VGPirpbin_getBinaryFileType.o: VGPirpbin_getBinaryFileType.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getBinaryFileType.c -o VGPirpbin_getBinaryFileType.o
VGPirpbin_getNumber.o: VGPirpbin_getNumber.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getNumber.c -o VGPirpbin_getNumber.o
VGPirpbin_getQualityCode.o: VGPirpbin_getQualityCode.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getQualityCode.c -o VGPirpbin_getQualityCode.o
VGPirpbin_getQualityTable.o: VGPirpbin_getQualityTable.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_getQualityTable.c -o VGPirpbin_getQualityTable.o
VGPirpbin_HeaderStatsLine.o: VGPirpbin_HeaderStatsLine.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HeaderStatsLine.c -o VGPirpbin_HeaderStatsLine.o
VGPirpbin_HuffmanCode.o: VGPirpbin_HuffmanCode.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HuffmanCode.c -o VGPirpbin_HuffmanCode.o
VGPirpbin_HuffmanDecodeQueueEntry.o: VGPirpbin_HuffmanDecodeQueueEntry.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HuffmanDecodeQueueEntry.c -o VGPirpbin_HuffmanDecodeQueueEntry.o
VGPirpbin_HuffmanInnerTable.o: VGPirpbin_HuffmanInnerTable.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_HuffmanInnerTable.c -o VGPirpbin_HuffmanInnerTable.o
VGPirpbin_IRPBINDecoder.o: VGPirpbin_IRPBINDecoder.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_IRPBINDecoder.c -o VGPirpbin_IRPBINDecoder.o
VGPirpbin_IRPBinDecoderContext.o: VGPirpbin_IRPBinDecoderContext.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_IRPBinDecoderContext.c -o VGPirpbin_IRPBinDecoderContext.o
VGPirpbin_IRPBINDecoderInput.o: VGPirpbin_IRPBINDecoderInput.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_IRPBINDecoderInput.c -o VGPirpbin_IRPBINDecoderInput.o
VGPirpbin_LineBuffer.o: VGPirpbin_LineBuffer.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_LineBuffer.c -o VGPirpbin_LineBuffer.o
VGPirpbin_main.o: VGPirpbin_main.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_main.c -o VGPirpbin_main.o
VGPirpbin_mconcat.o: VGPirpbin_mconcat.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_mconcat.c -o VGPirpbin_mconcat.o
VGPirpbin_mstrdup.o: VGPirpbin_mstrdup.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_mstrdup.c -o VGPirpbin_mstrdup.o
VGPirpbin_Pair.o: VGPirpbin_Pair.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_Pair.c -o VGPirpbin_Pair.o
VGPirpbin_PairTable.o: VGPirpbin_PairTable.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_PairTable.c -o VGPirpbin_PairTable.o
VGPirpbin_produceBinary.o: VGPirpbin_produceBinary.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_produceBinary.c -o VGPirpbin_produceBinary.o
VGPirpbin_ProvenanceStep.o: VGPirpbin_ProvenanceStep.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_ProvenanceStep.c -o VGPirpbin_ProvenanceStep.o
VGPirpbin_QualityHuffman.o: VGPirpbin_QualityHuffman.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c VGPirpbin_QualityHuffman.c -o VGPirpbin_QualityHuffman.o
VGPirpbin_Table.o: VGPirpbin_Table.c
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

#	cc -W -Wall -O3 -ansi -pedantic VGPirpbin.c -o VGPirpbin
