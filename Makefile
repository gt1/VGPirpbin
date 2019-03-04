SOURCE=VGPirpbin_LineBuffer.c VGPirpbin_mstrdup.c VGPirpbin_expect.c VGPirpbin_getNumber.c VGPirpbin_CString.c VGPirpbin_Pair.c \
	VGPirpbin_HuffmanInnerTable.c VGPirpbin_BitLevelEncoder.c VGPirpbin_BitLevelDecoder.c VGPirpbin_Table.c \
	VGPirpbin_PairTable.c VGPirpbin_CodeTableEntry.c VGPirpbin_CodeTable.c \
	VGPirpbin_HuffmanDecodeQueueEntry.c VGPirpbin_HuffmanCode.c VGPirpbin_QualityHuffman.c \
	VGPirpbin_mconcat.c VGPirpbin_ProvenanceStep.c VGPirpbin_HeaderStatsLine.c \
	VGPirpbin_DecodeResult.c VGPirpbin_IRPBINDecoder.c VGPirpbin_getQualityTable.c \
	VGPirpbin_getQualityCode.c VGPirpbin_produceBinary.c VGPirpbin_Arguments.c \
	VGPirpbin_decodeBinaryFile.c VGPirpbin_IRPBinDecoderContext.c VGPirpbin_IRPBINDecoderInput.c \
	VGPirpbin_getBinaryFileType.c

VGPirpbin: ${SOURCE} VGPirpbin_main.c
	${CC} -ansi -pedantic -W -Wall -O3 -g ${SOURCE} VGPirpbin_main.c -o VGPirpbin

clean:

distclean: clean
	rm -f VGPirpbin

#	cc -W -Wall -O3 -ansi -pedantic VGPirpbin.c -o VGPirpbin
