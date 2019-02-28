#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// static int const phredshift = 33;
#define PHREDSHIFT 33
#define HUFFMAN_ESCAPE_CODE 256

static char const * binfiletype = "1 3 bsq 1 0\n";
static char const * version = "0.0";

typedef struct _LineBuffer
{
	/* input stream */
	FILE * file;

	/* buffer */
	char * buffer;
	size_t buffer_n;

	/* size of buffer */
	size_t bufsize;
	/* set to true when end of file has been observed */
	int eof;

	/* start of buffer pointer */
	char * bufferptra;
	/* end of input data */
	char * bufferptrin;
	/* current read pointer */
	char * bufferptrout;
	/* end of buffer pointer */
	char * bufferptre;

	size_t prevlinelen;
	size_t spos;
} LineBuffer;

int64_t LineBuffer_read(LineBuffer * LB, char * p, size_t n);
LineBuffer * LineBuffer_allocatew(FILE * f, size_t initsize);
size_t LineBuffer_getPos(LineBuffer * LB);
int LineBuffer_getline(LineBuffer * LB, char const ** a, char const ** e);
void LineBuffer_putback(LineBuffer * LB, char const * a);
void LineBuffer_deallocate(LineBuffer * LB);

void LineBuffer_deallocate(LineBuffer * LB)
{
	if ( LB )
	{
		free(LB->buffer);
		LB->buffer = NULL;
		free(LB);
	}
}

int64_t LineBuffer_read(LineBuffer * LB, char * p, size_t n)
{
	size_t const r = fread(p,1,n,LB->file);

	if ( feof(LB->file) )
		LB->eof = 1;

	if ( r > 0 )
		return r;
	else if ( LB->eof )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

LineBuffer * LineBuffer_allocate(FILE * f, size_t initsize)
{
	LineBuffer * LB = (LineBuffer *)malloc(sizeof(LineBuffer));
	if ( ! LB )
		return NULL;
		
	memset(LB,0,sizeof(LineBuffer));
	
	LB->file = f;
	
	LB->buffer_n = initsize;
	LB->buffer = (char *)malloc(LB->buffer_n);
	if ( ! LB->buffer )
	{
		free(LB);
		return NULL;
	}

	LB->bufsize = initsize;
	LB->eof = 0;
	LB->bufferptra = LB->buffer;
	LB->bufferptrin = NULL;
	LB->bufferptrout = LB->buffer;
	LB->bufferptre = LB->buffer + LB->buffer_n;
	LB->prevlinelen = 0;
	LB->spos = 0;
	
	{
		int64_t const r = LineBuffer_read(LB,LB->bufferptra,LB->bufsize);
		
		if ( r < 0 )
		{
			free(LB->buffer);
			free(LB);
			return NULL;
		}
		
		LB->bufferptrin = LB->bufferptra + r;
	}
		
	return LB;
}

size_t LineBuffer_getPos(LineBuffer * LB)
{
	return LB->spos;
}

void LineBuffer_putback(LineBuffer * LB, char const * a);

/**
 * return the next line. If successful the method returns true and the start and end pointers
 * of the line are stored through the pointer a and e respectively.
 *
 * @param a pointer to line start pointer
 * @param e pointer to line end pointer
 * @return true if a line could be extracted, false otherwise
 **/
int LineBuffer_getline(
	LineBuffer * LB,
	char const ** a,
	char const ** e
)
{
	while ( 1 )
	{
		/* end of line pointer */
		char * lineend = LB->bufferptrout;

		/* search for end of buffer or line end */
		while ( lineend != LB->bufferptrin && *(lineend) != '\n' )
			++lineend;

		/* we reached the end of the data currently in memory */
		if ( lineend == LB->bufferptrin )
		{
			/* reached end of file, return what we have */
			if ( LB->eof )
			{
				/* this is the last line we will return */
				if ( LB->bufferptrout != LB->bufferptrin )
				{
					/* if file ends with a newline */
					if ( LB->bufferptrin[-1] == '\n' )
					{
						size_t linelen;
						*a = LB->bufferptrout;
						*e = LB->bufferptrin-1;
						LB->bufferptrout = LB->bufferptrin;

						linelen = (*e-*a)+1;
						LB->prevlinelen = linelen;
						LB->spos += linelen;

						return 1;
					}
					/* otherwise we append an artifical newline */
					else
					{
						size_t const numbytes = lineend - LB->bufferptrout;
						size_t linelen;
						
						size_t const tmpbuf_n = numbytes+1;
						char * tmpbuf = (char *)malloc(tmpbuf_n);
						if ( ! tmpbuf )
							return -1;

						memcpy(tmpbuf,LB->bufferptrout,numbytes);
						tmpbuf[numbytes] = '\n';
						
						free(LB->buffer);

						LB->buffer = tmpbuf;
						LB->buffer_n = tmpbuf_n;
						
						LB->bufsize = numbytes+1;
						LB->bufferptra = LB->buffer;
						LB->bufferptre = LB->buffer + LB->bufsize;
						LB->bufferptrin = LB->bufferptre;
						LB->bufferptrout = LB->bufferptre;

						*a = LB->bufferptra;
						*e = LB->bufferptre - 1;

						linelen = *e-*a;
						LB->prevlinelen = linelen;
						LB->spos += linelen;

						return 1;
					}
				}
				else
				{
					return 0;
				}
			}
			/* we need to read more data */
			else
			{
				/* do we need to extend the buffer? */
				if (
					(LB->bufferptrout == LB->bufferptra)
					&&
					(LB->bufferptrin == LB->bufferptre)
				)
				{
					size_t const newbufsize = LB->bufsize ? (2*LB->bufsize) : 1;
					
					char * newbuf = (char *)malloc(newbufsize);
					if ( ! newbuf )
						return -1;
					memcpy(newbuf,LB->buffer,LB->buffer_n);
					free(LB->buffer);
					LB->buffer = newbuf;
					LB->buffer_n = newbufsize;

					LB->bufferptre   = LB->buffer + LB->buffer_n;
					LB->bufferptrout = LB->buffer + (LB->bufferptrout - LB->bufferptra);
					LB->bufferptrin  = LB->buffer + (LB->bufferptrin - LB->bufferptra);
					LB->bufferptra   = LB->buffer;
					LB->bufsize      = LB->buffer_n;
				}
				else
				{
					/* move data to front and fill rest of buffer */
					size_t const used   = LB->bufferptrin  - LB->bufferptrout;
					size_t const unused = LB->bufsize - used;
					int64_t r;

					memmove(LB->bufferptra,LB->bufferptrout,used);

					LB->bufferptrout = LB->bufferptra;
					LB->bufferptrin  = LB->bufferptrout + used;

					r = LineBuffer_read(LB,LB->bufferptrin,unused);
					
					if ( r < 0 )
						return -1;
					
					LB->bufferptrin += r;
				}
			}
		}
		else
		{
			size_t linelen;
			*a = LB->bufferptrout;
			*e = lineend;
			assert ( *lineend == '\n' );
			LB->bufferptrout = lineend+1;

			linelen = (*e-*a)+1;
			LB->prevlinelen = linelen;
			LB->spos += linelen;

			return 1;
		}
	}

	return 0;
}

void LineBuffer_putback(LineBuffer * LB, char const * a)
{
	LB->spos -= LB->prevlinelen;
	LB->bufferptrout = (char *)a;
	LB->prevlinelen = 0;
}

#include <stdint.h>
#include <ctype.h>
#include <stddef.h>
#include <limits.h>

typedef struct _CString
{
	char const * a;
	char const * e;
} CString;

typedef struct _Pair
{
	uint64_t key;
	uint64_t value;
} Pair;

typedef struct _HuffmanInnerNode
{
	int64_t child_a;
	int64_t child_b;
	uint64_t freq;
} HuffmanInnerNode;

typedef struct _HuffmanInnerTable
{
	HuffmanInnerNode * A;
	size_t n;
} HuffmanInnerTable;

typedef struct _PairTable
{
	Pair * A;
	uint64_t n;
} PairTable;

typedef struct _Table
{
	uint64_t * A;
	size_t n;
} Table;

typedef struct _CodeTableEntry
{
	uint64_t symbol;
	uint64_t codelength;
	uint64_t code;
} CodeTableEntry;

typedef struct _CodeTable
{
	CodeTableEntry * A;
	uint64_t n;
} CodeTable;


typedef struct _HuffmanDecodeQueueEntry
{
	size_t from;
	size_t to;
	int64_t left;
	int64_t right;
} HuffmanDecodeQueueEntry;

typedef struct _HuffmanCode
{
	PairTable * PT;
	CodeTable * CT;
	CodeTable * CTsorted;
	CodeTable * CTsortedSparse;
	HuffmanDecodeQueueEntry * DQ;
} HuffmanCode;

typedef struct _QualityHuffman
{
	HuffmanCode * firstHuf;
	HuffmanCode * difHuf;
} QualityHuffman;

typedef struct _BitLevelEncoder
{
	FILE * out;
	uint8_t c;
	unsigned int f;
	uint64_t w;
} BitLevelEncoder;

typedef struct _BitLevelDecoder
{
	FILE * in;
	uint8_t c;
	unsigned int f;
} BitLevelDecoder;


HuffmanDecodeQueueEntry * HuffmanDecodeQueueEntry_allocate(size_t n)
{
	HuffmanDecodeQueueEntry * H = NULL;
	
	H = (HuffmanDecodeQueueEntry *)malloc(n * sizeof(HuffmanDecodeQueueEntry));
	
	return H;
}

void HuffmanDecodeQueueEntry_deallocate(HuffmanDecodeQueueEntry * H)
{
	free(H);
}

int expect(char const **a, char const **e, char const c);
int64_t getNumber(char const ** a, char const ** e);
CString getString(char const ** a, char const ** e);
void swap(uint64_t * a, uint64_t * b);
void Pair_swap(Pair * A, Pair * B);
HuffmanInnerTable * HuffmanInnerTable_allocate(size_t n);
void HuffmanInnerTable_deallocate(HuffmanInnerTable * H);
PairTable * Table_getPairTable(Table const * T);
void PairTable_deallocate(PairTable * PT);
PairTable * PairTable_allocate(uint64_t const n);
Table * Table_allocate(size_t size);
int incrementTable(Table * T, size_t i);
void Table_deallocate(Table * T);
CodeTable * CodeTable_allocate(size_t n);
CodeTable * CodeTable_sortBySymbol(CodeTable const * C);
CodeTable * CodeTable_createSparse(CodeTable const * C);
void CodeTable_deallocate(CodeTable * T);
void printCodeTableEntry(FILE * out, CodeTableEntry const * C);
void printCodeTable(FILE * out, CodeTable * C);
CodeTable * CodeTable_Create(PairTable const * PT);
HuffmanCode * HuffmanCode_allocate(PairTable * PT, CodeTable * CT, CodeTable * CTsorted, CodeTable * CTsortedSparse);
void HuffmanCode_deallocate(HuffmanCode * HC);
QualityHuffman * QualityHuffman_allocate(HuffmanCode * firstHuf, HuffmanCode * difHuf);
void QualityHuffman_deallocate(QualityHuffman * Q);
uint64_t BitLevelEncoder_getOffset(BitLevelEncoder const * A);
BitLevelEncoder * BitLevelEncoder_allocate(FILE * out);
int BitLevelEncoder_write(BitLevelEncoder * enc);
int BitLevelEncoder_flush(BitLevelEncoder * enc);
int BitLevelEncoder_encode(BitLevelEncoder * enc, uint64_t codeword, unsigned int codelength);
int BitLevelEncoder_encodeGamma(BitLevelEncoder * enc, uint64_t v);
int BitLevelEncoder_deallocate(BitLevelEncoder * enc);
BitLevelDecoder * BitLevelDecoder_allocate(FILE * in);
int BitLevelDecoder_load(BitLevelDecoder * BLV);
int BitLevelDecoder_getBit(BitLevelDecoder * BLV);
int BitLevelDecoder_decodeGamma(BitLevelDecoder * BLV, uint64_t * v);
void BitLevelDecoder_deallocate(BitLevelDecoder * BLV);
HuffmanCode * computeHuffmanCodeFromLengths(PairTable ** firstPT);
int computeHuffmanCodeLength(PairTable * PT);
int BitLevelDecoder_decode(BitLevelDecoder * BLV, uint64_t * v, unsigned int l);
int HuffmanCode_encodeSymbol(BitLevelEncoder * BLE, HuffmanCode * HC, uint64_t const sym);

int HuffmanCode_encodeSymbol(BitLevelEncoder * BLE, HuffmanCode * HC, uint64_t const sym)
{
	CodeTable const * CT = HC->CTsortedSparse;
	CodeTableEntry const * CTE = NULL;
	
	if ( sym >= CT->n )
		return -1;
	
	CTE = &(CT->A[sym]);
	
	if ( BitLevelEncoder_encode(BLE,CTE->code,CTE->codelength) < 0 )
		return -1;
		
	return 0;
}

PairTable * PairTable_allocate(uint64_t const n)
{
	PairTable * P = NULL;
	uint64_t i;
	
	if ( !(P = (PairTable *)malloc(sizeof(PairTable))) )
		return NULL;
		
	P->A = NULL;
	P->n = n;
	
	P->A = (Pair *)malloc(n * sizeof(Pair));
	
	if ( ! P->A )
	{
		free(P);
		return NULL;
	}
	
	for ( i = 0; i < n; ++i )
	{
		P->A[i].key = P->A[i].value = 0;
	}
	
	return P;
}

int PairTable_encode(PairTable const * P, BitLevelEncoder * BLE)
{
	size_t i;
	
	if ( BitLevelEncoder_encodeGamma(BLE,P->n) < 0 )
		return -1;
		
	for ( i = 0; i < P->n; ++i )
	{
		if ( BitLevelEncoder_encodeGamma(BLE,P->A[i].key) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeGamma(BLE,P->A[i].value) < 0 )
			return -1;
	}

	return 0;
}

int PairTable_encodeHuffmanDif(PairTable const * P, BitLevelEncoder * BLE)
{
	size_t i;
	uint64_t prevkey = 0;
	uint64_t prevval = 0;
	
	if ( BitLevelEncoder_encodeGamma(BLE,P->n) < 0 )
		return -1;
		
	for ( i = 0; i < P->n; ++i )
	{
		fprintf(stderr,"PairTable[%d]=(%d,%d)\n",(int)i,(int)P->A[i].key,(int)P->A[i].value);
	}
		
	for ( i = 0; i < P->n; ++i )
	{
		uint64_t const vdif = P->A[i].value - prevval;
		uint64_t kdif;
		
		if ( vdif )
			prevkey = 0;

		assert ( P->A[i].key   >= prevkey );
		assert ( P->A[i].value >= prevval );
		
		kdif = P->A[i].key   - prevkey;
		
		if ( BitLevelEncoder_encodeGamma(BLE,vdif) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeGamma(BLE,kdif) < 0 )
			return -1;
		
		prevkey = P->A[i].key;	
		prevval = P->A[i].value;
	}

	return 0;
}

PairTable * PairTable_decode(BitLevelDecoder * BLD)
{
	size_t i;
	PairTable * P = NULL;
	uint64_t n;
	
	if ( BitLevelDecoder_decodeGamma(BLD,&n) < 0 )
		return NULL;
		
	if ( ! (P = PairTable_allocate(n)) )
		return NULL;
	
	for ( i = 0; i < P->n; ++i )
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&P->A[i].key) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}
		if ( BitLevelDecoder_decodeGamma(BLD,&P->A[i].value) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}
	}

	return P;
}

PairTable * PairTable_decodeHuffmanDif(BitLevelDecoder * BLD)
{
	size_t i;
	PairTable * P = NULL;
	uint64_t n;
	uint64_t prevval = 0;
	uint64_t prevkey = 0;
	uint64_t v;
	
	if ( BitLevelDecoder_decodeGamma(BLD,&n) < 0 )
		return NULL;
		
	if ( ! (P = PairTable_allocate(n)) )
		return NULL;
	
	for ( i = 0; i < P->n; ++i )
	{		
		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}
		
		prevval += v;
		P->A[i].value = prevval;
		
		if ( v )
			prevkey = 0;

		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}
		
		prevkey += v;
		P->A[i].key = prevkey;
	}

	return P;
}

int HuffmanCode_encode(BitLevelEncoder * BLE, HuffmanCode const * HC)
{
	int const r = PairTable_encodeHuffmanDif(HC->PT,BLE);
	
	return r;
}

HuffmanCode * HuffmanCode_decode(BitLevelDecoder * BLD)
{
	PairTable * PT = NULL;
	HuffmanCode * HC = NULL;
	
	if ( ! (PT = PairTable_decodeHuffmanDif(BLD)) )
		return NULL;

	if ( ! (HC = computeHuffmanCodeFromLengths(&PT)) )
		return NULL;

	return HC;
}

int QualityHuffman_encode(BitLevelEncoder * BLE, QualityHuffman const * QH)
{
	if ( HuffmanCode_encode(BLE,QH->firstHuf) < 0 )
		return -1;
	if ( HuffmanCode_encode(BLE,QH->difHuf) < 0 )
		return -1;
	return 0;
}

QualityHuffman * QualityHuffman_decode(BitLevelDecoder * BLD)
{
	QualityHuffman * QH = NULL;
	
	QH = (QualityHuffman *)malloc(sizeof(QualityHuffman));
	
	if ( ! QH )
		return NULL;
	
	QH->firstHuf = NULL;
	QH->difHuf = NULL;
	
	if ( ! (QH->firstHuf = HuffmanCode_decode(BLD)) )
	{
		QualityHuffman_deallocate(QH);
		return NULL;
	}
	if ( ! (QH->difHuf = HuffmanCode_decode(BLD)) )
	{
		QualityHuffman_deallocate(QH);
		return NULL;
	}

	return QH;
}

int expect(char const **a, char const **e, char const c)
{
	if ( *a == *e || (*a)[0] != c )
		return 0;
	else
	{
		(*a) += 1;
		return 1;
	}
}

int64_t getNumber(char const ** a, char const ** e)
{
	int64_t n = 0;
	unsigned int ni = 0;
	
	while ( *a != *e && isdigit(**a) )
	{
		int64_t const dig = *a[0] - '0';
		(*a) += 1;
		
		n *= 10;
		n += dig;
		ni += 1;
	}
	
	if ( ! ni )
		return -1;
	else
		return n;
}

CString getString(char const ** a, char const ** e)
{
	int64_t const n = getNumber(a,e);
	CString C;
	C.a = NULL;
	C.e = NULL;
	
	if ( n < 0 )
		return C;
	
	if ( ! expect(a,e,' ') )
		return C;
	
	if ( *e - *a < n )
		return C;
	
	C.a = *a;
	C.e = *a + n;
	
	*a += n;
	
	return C;
}

void swap(uint64_t * a, uint64_t * b)
{
	uint64_t t = *a;
	*a = *b;
	*b = t;
}

void Pair_swap(Pair * A, Pair * B)
{
	swap(&A->key,&B->key);
	swap(&A->value,&B->value);
}

static int paircomp(const void * A, const void * B)
{
	Pair const * PA = (Pair const *)A;
	Pair const * PB = (Pair const *)B;
	
	if ( PA->value != PB->value )
	{
		if ( PA->value < PB->value )
			return -1;
		else
			return 1;
	}

	if ( PA->key != PB->key )
	{
		if ( PA->key < PB->key )
			return -1;
		else
			return 1;
	}

	return 0;
	
	#if 0
	if ( PA->key != PB->key )
		return PA->key < PB->key;
	else
		return PA->value < PB->value;
	#endif
}

HuffmanInnerTable * HuffmanInnerTable_allocate(size_t n)
{
	HuffmanInnerTable * H = (HuffmanInnerTable *)malloc(sizeof(HuffmanInnerTable));
	if ( ! H )
		return NULL;
	
	H->n = n;
	H->A = (HuffmanInnerNode *)malloc(n*sizeof(HuffmanInnerNode));
	
	if ( ! H->A )
	{
		free(H);
		return NULL;
	}
	
	return H;
}

void HuffmanInnerTable_deallocate(HuffmanInnerTable * H)
{
	if ( H )
	{
		free(H->A);
		free(H);
	}
}

PairTable * Table_getPairTable(Table const * T)
{
	size_t i;
	size_t n = 0;
	PairTable * PT = NULL;
	
	for ( i = 0; i < T->n; ++i )
		if ( T->A[i] )
			++n;
	
	PT = (PairTable *)malloc(sizeof(PairTable));
	if ( ! PT )
		return NULL;
	
	PT->A = (Pair *)malloc(n * sizeof(Pair));
	PT->n = n;
	if ( ! PT->A )
	{
		free(PT);
		return NULL;
	}

	n = 0;
	for ( i = 0; i < T->n; ++i )
		if ( T->A[i] )
		{
			PT->A[n].key = i;
			PT->A[n].value = T->A[i];
			n += 1;
		}
		
	assert ( n == PT->n );
	
	qsort(
		PT->A,
		PT->n,
		sizeof(Pair),
		paircomp
	);
	
	return PT;
}

void PairTable_deallocate(PairTable * PT)
{
	if ( PT )
	{
		free(PT->A);
		free(PT);
	}
}


Table * Table_allocate(size_t size)
{
	size_t i;
	Table * T = (Table *)malloc(sizeof(Table));
	if ( ! T )
		return NULL;
	
	T->A = NULL;
	T->n = size;
	
	T->A = (uint64_t *)malloc(size * sizeof(uint64_t));
	if ( ! T->A )
	{
		free(T);
		return NULL;
	}
	
	for ( i = 0; i < size; ++i )
		T->A[i] = 0;

	return T;
}

int incrementTable(Table * T, size_t i)
{
	while ( i >= T->n )
	{
		size_t newsize = T->n ? 2*T->n : 1;
		size_t j;
		Table * NT = Table_allocate(newsize);
		if ( ! NT )
			return -1;
		
		for ( j = 0; j < T->n; ++j )
			NT->A[j] = T->A[j];
		
		free(T->A);
		T->A = NT->A;
		T->n = NT->n;
		
		free(NT);
	}
	
	assert ( i < T->n );
	
	T->A[i] ++;
	
	return 0;
}

void Table_deallocate(Table * T)
{
	if ( T )
	{
		free(T->A);
		free(T);
	}
}

static int codeTableSymbolComparator(void const * A, void const * B)
{
	CodeTableEntry * CA = (CodeTableEntry *)A;
	CodeTableEntry * CB = (CodeTableEntry *)B;
	
	if ( CA->symbol < CB->symbol )
		return -1;
	else if ( CB->symbol < CA->symbol )
		return 1;
	else
		return 0;
}


CodeTable * CodeTable_allocate(size_t n)
{
	CodeTable * T = (CodeTable *)malloc(sizeof(CodeTable));
	
	if ( ! T )
		return NULL;
	
	T->n = n;
	T->A = (CodeTableEntry *)malloc(sizeof(CodeTableEntry)*n);
	
	if ( ! T->A )
	{
		free(T);
		return NULL;
	}
	
	return T;
}

CodeTable * CodeTable_sortBySymbol(CodeTable const * C)
{
	CodeTable * O = CodeTable_allocate(C->n);
	size_t i;
	if ( ! O )
		return NULL;
	
	for ( i = 0; i < C->n; ++i )
		O->A[i] = C->A[i];
		
	qsort(O->A,O->n,sizeof(CodeTableEntry),codeTableSymbolComparator);
	
	return O;
}

CodeTable * CodeTable_createSparse(CodeTable const * C)
{
	CodeTable * O = NULL;
	size_t i;
	
	if ( C->n == 0 )
		return CodeTable_allocate(0);
	
	O = CodeTable_allocate(C->A[C->n-1].symbol+1);
	if ( ! O )
		return NULL;
		
	for ( i = 0; i < O->n; ++i )
	{
		CodeTableEntry CTE;
		CTE.symbol = UINT64_MAX;
		CTE.codelength = UINT64_MAX;
		CTE.code = UINT64_MAX;
		O->A[i] = CTE;
	}
	
	for ( i = 0; i < C->n; ++i )
		O->A[C->A[i].symbol] = C->A[i];

	return O;
}

void CodeTable_deallocate(CodeTable * T)
{
	if ( T )
	{
		free(T->A);
		free(T);
	}
}

void printCodeTableEntry(FILE * out, CodeTableEntry const * C)
{
	size_t i;

	if ( C->codelength != UINT64_MAX )
	{
		fprintf(out,"sym %05d len %05d ", (int)C->symbol, (int)C->codelength);
	
		for ( i = 0; i < C->codelength; ++i )
			fprintf(out,"%d", (int)(C->code >> (C->codelength-i-1))&1);
		fprintf(out,"\n");
	}
}

void printCodeTable(FILE * out, CodeTable * C)
{
	size_t i;
	
	for ( i = 0; i < C->n; ++i )
	{
		fprintf(out,"%08d ",(int)i);
		
		if ( C->A[i].codelength != UINT64_MAX )
		{
			printCodeTableEntry(
				out,
				&(C->A[i])
			);
		}
		else
		{
			fprintf(out,"\n");
		}
	}
}

CodeTable * CodeTable_Create(PairTable const * PT)
{
	size_t low = 0;
	size_t i;
	uint64_t nextcode = 0;
	CodeTable * CT = CodeTable_allocate(PT->n);
	
	for ( i = 0; i < PT->n; ++i )
		if ( PT->A[i].value > 64 )
		{
			fprintf(stderr,"[E] code length %lu exceeds 64 bits\n", (unsigned long)PT->A[i].value);
			CodeTable_deallocate(CT);
			return NULL;
		}
	
	while ( low < PT->n )
	{
		size_t const len = PT->A[low].value;
		size_t high = low;
		
		while ( high < PT->n && PT->A[high].value == PT->A[low].value )
		{
			#if 0
			size_t i;

			fprintf(stderr,"sym %05d len %05d ", (int)PT->A[high].key, (int)PT->A[high].value);
			
			for ( i = 0; i < len; ++i )
				fprintf(stderr,"%d", (int)(nextcode >> (len-i-1))&1);
			fprintf(stderr,"\n");
			#endif
			
			CodeTableEntry CTE;
			CTE.symbol = PT->A[high].key;
			CTE.codelength = len;
			CTE.code = nextcode;
			
			CT->A[high] = CTE;
		
			nextcode += 1;
			high++;
		}
		
		low = high;
		
		if ( low < PT->n )
		{
			size_t const nlen = PT->A[low].value;
			size_t const lendif = nlen - len;
			nextcode <<= lendif;
		}
	}

	return CT;
}

int computeHuffmanCodeLength(PairTable * PT)
{
	size_t const numinner = PT->n ? (PT->n - 1) : 0;
	HuffmanInnerTable * HUFIN = NULL;
	size_t * decQ = NULL;
	size_t lq_a = 0;
	size_t lq_e = PT->n;
	size_t in_a = 0;
	size_t in_e = 0;
	size_t * dec_a = NULL;
	size_t * dec_e = NULL;
	size_t depth = 0;
	size_t i;
	
	if ( ! PT->n )
	{
		return 0;
	}
	else if ( PT->n == 1 )
	{
		PT->A[0].value = 0;
		return 0;
	}

	if ( ! (HUFIN = HuffmanInnerTable_allocate(numinner)) )
		goto error;
		
	if ( ! (decQ = (size_t *)malloc(sizeof(size_t)*(2*PT->n-1))) )
		goto error;

	while ( (lq_e-lq_a) + (in_e-in_a) > 1 )
	{
		size_t c_0, c_1;
		size_t freq_0, freq_1;
		HuffmanInnerNode HIN;
		
		if ( lq_e - lq_a && in_e - in_a )
		{
			if ( PT->A[lq_a].value <= HUFIN->A[in_a].freq )
			{
				freq_0 = PT->A[lq_a].value;
				c_0 = lq_a++;
			}
			else
			{
				freq_0 = HUFIN->A[in_a].freq;
				c_0 = in_a++ + PT->n;
			}
		}
		else
		{
			if ( lq_e - lq_a )
			{
				freq_0 = PT->A[lq_a].value;
				c_0 = lq_a++;
			}
			else
			{
				freq_0 = HUFIN->A[in_a].freq;
				c_0 = in_a++ + PT->n;			
			}
		}
		if ( lq_e - lq_a && in_e - in_a )
		{
			if ( PT->A[lq_a].value <= HUFIN->A[in_a].freq )
			{
				freq_1 = PT->A[lq_a].value;
				c_1 = lq_a++;
			}
			else
			{
				freq_1 = HUFIN->A[in_a].freq;
				c_1 = in_a++ + PT->n;
			}
		}
		else
		{
			if ( lq_e - lq_a )
			{
				freq_1 = PT->A[lq_a].value;
				c_1 = lq_a++;
			}
			else
			{
				freq_1 = HUFIN->A[in_a].freq;
				c_1 = in_a++ + PT->n;			
			}
		}
		
		HIN.child_a = c_0;
		HIN.child_b = c_1;
		HIN.freq = freq_0+freq_1;
		HUFIN->A[in_e++] = HIN;
	}
	
	dec_a = decQ;
	dec_e = decQ;
	*(dec_e++) = (in_e-1) + PT->n;
	
	for (; dec_a != dec_e; depth++ )
	{
		size_t * dec_z = dec_e;
		
		if ( depth > 8*sizeof(uint64_t) )
		{
			fprintf(stderr,"[E] huffman tree is deeper than 64 bits");
			goto error;
		}
		
		while ( dec_a != dec_e )
		{
			size_t const id = *(dec_a++);
			
			if ( id < PT->n )
			{
				PT->A[id].value = depth;
				/* fprintf(stderr,"depth %d symbol %d\n",(int)depth,(int)PT->A[id].key); */
			}
			else
			{
				*dec_z++ = HUFIN->A[id-PT->n].child_a;
				*dec_z++ = HUFIN->A[id-PT->n].child_b;
			}
		}

		dec_a = dec_e;
		dec_e = dec_z;
	}
	
	for ( i = 0; i < PT->n/2; ++i )
		Pair_swap(
			&PT->A[i],
			&PT->A[PT->n-i-1]
		);
	
	qsort(
		PT->A,
		PT->n,
		sizeof(Pair),
		paircomp
	);
	
	free(decQ);
	HuffmanInnerTable_deallocate(HUFIN);
	
	return 0;
	
	error:
	HuffmanInnerTable_deallocate(HUFIN);
	free(decQ);
	
	return -1;
}


HuffmanCode * HuffmanCode_allocate(
	PairTable * PT,
	CodeTable * CT,
	CodeTable * CTsorted,
	CodeTable * CTsortedSparse
)
{
	HuffmanCode * HC = (HuffmanCode *)malloc(sizeof(HuffmanCode));
	if ( ! HC )
		return NULL;
		
	HC->PT = PT;
	HC->CT = CT;
	HC->CTsorted = CTsorted;
	HC->CTsortedSparse = CTsortedSparse;
	HC->DQ = NULL;
	
	return HC;
}

int64_t HuffmanCode_decodeSymbol(HuffmanCode const * H, BitLevelDecoder * BLD)
{
	uint64_t nodeid = 0;
	
	while ( H->DQ[nodeid].to - H->DQ[nodeid].from > 1 )
	{
		int v;
		
		v = BitLevelDecoder_getBit(BLD);
		
		if ( v < 0 )
		{
			return -1;
		}
		else if ( v == 0 )
		{
			nodeid = H->DQ[nodeid].left;
		}
		else
		{
			nodeid = H->DQ[nodeid].right;
		}		
	}
	
	return H->CT->A[ H->DQ[nodeid].from ].symbol;
}

void HuffmanCode_deallocate(
	HuffmanCode * HC
)
{
	if ( HC )
	{
		PairTable_deallocate(HC->PT);
		CodeTable_deallocate(HC->CT);
		CodeTable_deallocate(HC->CTsorted);
		CodeTable_deallocate(HC->CTsortedSparse);
		HuffmanDecodeQueueEntry_deallocate(HC->DQ);
		free(HC);
	}
}

QualityHuffman * QualityHuffman_allocate(HuffmanCode * firstHuf, HuffmanCode * difHuf)
{
	QualityHuffman * Q = (QualityHuffman *)malloc(sizeof(QualityHuffman));
	
	if ( ! Q )
		return NULL;
	
	Q->firstHuf = firstHuf;
	Q->difHuf = difHuf;
	
	return Q;
}

void QualityHuffman_deallocate(QualityHuffman * Q)
{
	if ( Q )
	{
		HuffmanCode_deallocate(Q->firstHuf);
		HuffmanCode_deallocate(Q->difHuf);
		free(Q);
	}
}

int getQualityTable(FILE * in, int64_t const maxlines, Table ** rqualTable, Table ** rsymTable, Table ** rlengthsTable)
{
	LineBuffer * LB = NULL;
	char const * a;
	char const * e;
	char const * ft = "seq";
	CString CS;
	Table * qualTable = NULL;
	Table * symTable = NULL;
	Table * lengthsTable = NULL;
	size_t i;
	size_t rank = 0;
	size_t nr = 0;
	uint64_t fs = 0;
	int64_t ql = 0;
	double dfs;
	
	/* seek to end of file */
	fseek(in,0,SEEK_END);
	fs = ftello(in);
	fseek(in,0,SEEK_SET);
	dfs = fs;
	
	LB = LineBuffer_allocate(in,1);
	
	if ( ! LB )
	{
		fprintf(stderr,"[E] unable to allocate LineBuffer\n");
		goto cleanup;			
	}
	
	if ( ! (qualTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate qual table\n");
		goto cleanup;		
	}

	if ( ! (symTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate sym table\n");
		goto cleanup;		
	}

	if ( ! (lengthsTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate lengths table\n");
		goto cleanup;		
	}

	if ( LineBuffer_getline(LB,&a,&e) <= 0 )
	{
		fprintf(stderr,"[E] unable to get first line of file\n");
		goto cleanup;
	}
	if ( ! expect(&a,&e,'1') )
	{
		fprintf(stderr,"[E] first line of file does not start with a 1\n");
		goto cleanup;	
	}
	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		goto cleanup;	
	}
	
	CS = getString(&a,&e);
	
	if ( ! CS.a )
	{		
		fprintf(stderr,"[E] unable to read file type\n");
		goto cleanup;	
	}
	
	if ( (CS.e - CS.a) != (ptrdiff_t)(strlen(ft)) )
	{
		fprintf(stderr,"[E] file type has the wrong length\n");
		goto cleanup;		
	}
	
	if ( strncmp(CS.a,ft,CS.e-CS.a) != 0 )
	{
		fprintf(stderr,"[E] unexpected file type\n");
		goto cleanup;			
	}

	while ( LineBuffer_getline(LB,&a,&e) && ql < maxlines )
	{
		if ( e-a && isalpha(a[0]) )
		{
			if ( incrementTable(symTable, a[0]) < 0 )
			{
				fprintf(stderr,"[E] unable to increment value\n");
				goto cleanup;
			}		
		}
	
		if ( e-a && a[0] == 'S' )
		{
			char const * c = a;
			int64_t l;
			
			c += 1;
			
			if ( c != e && *c == ' ' )
				++c;
			else
			{
				fprintf(stderr,"[E] malformed S line\n");
				goto cleanup;	
			}
			
			if ( (l = getNumber(&c,&e)) < 0 )
			{
				fprintf(stderr,"[E] malformed S line\n");
				goto cleanup;	
			}
			else
			{
				if ( l < HUFFMAN_ESCAPE_CODE )
				{
					if ( incrementTable(lengthsTable, l) < 0 )
					{
						fprintf(stderr,"[E] unable to increment value\n");
						goto cleanup;
					}
				}
				else
				{				
					if ( incrementTable(lengthsTable, HUFFMAN_ESCAPE_CODE) < 0 )
					{
						fprintf(stderr,"[E] unable to increment value\n");
						goto cleanup;
					}
				}
			}
		}

		if ( e-a && a[0] == 'Q' )
		{
			CString QS;
			char const * c;
			int64_t l;
			
			a += 1;
			
			if ( ! expect(&a,&e,' ') )
			{
				fprintf(stderr,"[E] malformed Q line\n");
				goto cleanup;
			}

			QS = getString(&a,&e);
			
			if ( ! QS.a )
			{
				fprintf(stderr,"[E] malformed Q line\n");
				goto cleanup;			
			}
			
			l = QS.e - QS.a;

			if ( l < HUFFMAN_ESCAPE_CODE )
			{
				if ( incrementTable(lengthsTable, l) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
			}
			else
			{				
				if ( incrementTable(lengthsTable, HUFFMAN_ESCAPE_CODE) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
			}

			for ( c = QS.a; c < QS.e; ++c )
				if ( *c < PHREDSHIFT || *c > 93 )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					goto cleanup;
				}
				
			for ( c = QS.a; c < QS.e; ++c )
			{
				int64_t const lv = *c - PHREDSHIFT;
				
				if ( incrementTable(qualTable, lv) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
			}		
			
			nr += 1;
			
			if ( nr % (4*1024*1024) == 0 )
				fprintf(stderr,"[V] getQualityTable %lu %f\n", (unsigned long)nr, (double)ftello(in)/dfs);
				
			ql += 1;			
		}
	}
	
	if ( maxlines != INT64_MAX )
	{
		if ( incrementTable(symTable, HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}		
		if ( incrementTable(lengthsTable, HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}		
	}

	LineBuffer_deallocate(LB);
	
	for ( i = 0; i < qualTable->n; ++i )
		if ( qualTable->A[i] )
			qualTable->A[i] = rank++;
		else
			qualTable->A[i] = UINT64_MAX;

	fprintf(stderr,"[V] getQualityTable %lu %f\n", (unsigned long)nr, 1.0);
	
	*rqualTable = qualTable;
	*rsymTable = symTable;
	*rlengthsTable = lengthsTable;
	
	return 0;

	cleanup:
	LineBuffer_deallocate(LB);
	Table_deallocate(qualTable);
	Table_deallocate(symTable);
	
	return -1;
}

HuffmanCode * computeHuffmanCodeFromLengths(PairTable ** firstPT)
{
	CodeTable * firstCT = NULL;
	CodeTable * firstCTSorted = NULL;
	CodeTable * firstCTSortedSparse = NULL;
	HuffmanCode * firstHC = NULL;
	HuffmanDecodeQueueEntry * HDQE = NULL;
	HuffmanDecodeQueueEntry * H_a = NULL;
	HuffmanDecodeQueueEntry * H_e = NULL;
	unsigned int depth = 0;
	
	if ( ! (HDQE = HuffmanDecodeQueueEntry_allocate( 2 * (*firstPT)->n)) )
	{
		fprintf(stderr,"[E] unable to allocate HuffmanDecodeQueueEntry\n");
		goto cleanup;
	}

	if ( ! (firstCT = CodeTable_Create(*firstPT)) )
	{
		fprintf(stderr,"[E] unable to produce code table from firstPT\n");
		goto cleanup;			
	}
	if ( ! (firstCTSorted = CodeTable_sortBySymbol(firstCT)) )
	{
		fprintf(stderr,"[E] unable to produce sorted code table from firstPT\n");
		goto cleanup;				
	}
	if ( ! (firstCTSortedSparse = CodeTable_createSparse(firstCTSorted)) )
	{
		fprintf(stderr,"[E] unable to produce sorted sparse code table from firstPT\n");
		goto cleanup;					
	}

	firstHC = HuffmanCode_allocate(*firstPT,firstCT,firstCTSorted,firstCTSortedSparse);
	if ( ! firstHC )
	{
		fprintf(stderr,"[E] unable to produce first huffman code\n");
		goto cleanup;					
	}
	
	*firstPT = NULL;
	
	H_a = HDQE;
	H_e = HDQE;
	
	{
		HuffmanDecodeQueueEntry HD;
		HD.from = 0;
		HD.to = firstHC->CT->n;
		*(H_e++) = HD;
	}
	
	while ( H_a != H_e )
	{
		HuffmanDecodeQueueEntry * H_z = H_e;
		
		while ( H_a != H_e )
		{
			HuffmanDecodeQueueEntry HD = *(H_a);
			uint64_t low = HD.from;
			uint64_t const end = HD.to;
			HuffmanDecodeQueueEntry NHDA[2];
			uint64_t nbits = 0;
			uint64_t i;

			#if 0
			fprintf(stderr,"---- interval ----\n");
			for ( i = low; i < end; ++i )
			{
				printCodeTableEntry(stderr,&(firstHC->CT->A[i]));
			}
			#endif

			while ( low != end )
			{
				unsigned int const bit = ((firstHC->CT->A[low].code >> (firstHC->CT->A[low].codelength - 1 - depth)) & 1);
				uint64_t high = low + 1;
				HuffmanDecodeQueueEntry NHD;

				while ( high != end && ((firstHC->CT->A[high].code >> (firstHC->CT->A[high].codelength - 1 - depth)) & 1) == bit )
					++high;
					
				NHD.from = low;
				NHD.to = high;
				NHDA[bit] = NHD;

				nbits += 1;
				low = high;
			}
			
			if ( nbits > 1 )
			{
				for ( i = 0; i < nbits; ++i )
				{
					if ( i == 0 )
						H_a->left = H_z - HDQE;
					else
						H_a->right = H_z - HDQE;
					
					*(H_z++) = NHDA[i];
				}
			}
			
			++H_a;
		}
				
		H_a = H_e;
		H_e = H_z;
		
		++depth;
	}
	
	firstHC->DQ = HDQE;
	HDQE = NULL;

	return firstHC;
	
	cleanup:
	PairTable_deallocate(*firstPT);
	HuffmanDecodeQueueEntry_deallocate(HDQE);
	*firstPT = NULL;
	return NULL;
}

QualityHuffman * getQualityCode(FILE * in, Table const * qualTable, int64_t const maxlines)
{
	LineBuffer * LB = NULL;
	char const * a;
	char const * e;
	char const * ft = "seq";
	CString CS;
	Table * firstTable = NULL;
	Table * difTable = NULL;
	PairTable * firstPT = NULL;
	PairTable * difPT = NULL;
	#if 0
	CodeTable * firstCT = NULL;
	CodeTable * difCT = NULL;
	CodeTable * firstCTSorted = NULL;
	CodeTable * difCTSorted = NULL;
	CodeTable * firstCTSortedSparse = NULL;
	CodeTable * difCTSortedSparse = NULL;
	#endif
	HuffmanCode * firstHC = NULL;
	HuffmanCode * difHC = NULL;
	size_t nr = 0;
	uint64_t fs;
	double dfs;
	QualityHuffman * QH = NULL;
	int64_t ql = 0;

	fseek(in,0,SEEK_END);
	fs = ftello(in);
	fseek(in,0,SEEK_SET);
	dfs = fs;
	
	LB = LineBuffer_allocate(in,1);
	
	if ( ! LB )
	{
		fprintf(stderr,"[E] failed to instantiate line buffer\n");
		goto cleanup;
	}

	if ( ! (firstTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate firstTable\n");
		goto cleanup;		
	}

	if ( ! (difTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate firstTable\n");
		goto cleanup;		
	}
	
	
	if ( LineBuffer_getline(LB,&a,&e) <= 0 )
	{
		fprintf(stderr,"[E] unable to get first line of file\n");
		goto cleanup;
	}
	if ( ! expect(&a,&e,'1') )
	{
		fprintf(stderr,"[E] first line of file does not start with a 1\n");
		goto cleanup;	
	}
	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		goto cleanup;	
	}
	
	CS = getString(&a,&e);
	
	if ( ! CS.a )
	{		
		fprintf(stderr,"[E] unable to read file type\n");
		goto cleanup;	
	}
	
	if ( (CS.e - CS.a) != (ptrdiff_t)(strlen(ft)) )
	{
		fprintf(stderr,"[E] file type has the wrong length\n");
		goto cleanup;		
	}
	
	if ( strncmp(CS.a,ft,CS.e-CS.a) != 0 )
	{
		fprintf(stderr,"[E] unexpected file type\n");
		goto cleanup;			
	}

	while ( LineBuffer_getline(LB,&a,&e) && ql < maxlines )
	{
		if ( e-a && a[0] == 'Q' )
		{
			CString QS;
			char const * c;
			int64_t first;
			int64_t prev;
			
			a += 1;
			
			if ( ! expect(&a,&e,' ') )
			{
				fprintf(stderr,"[E] malformed Q line\n");
				goto cleanup;
			}

			QS = getString(&a,&e);
			
			if ( ! QS.a )
			{
				fprintf(stderr,"[E] malformed Q line\n");
				goto cleanup;			
			}
	
			for ( c = QS.a; c < QS.e; ++c )
				if ( *c < PHREDSHIFT || *c > 93 )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					goto cleanup;
				}
				
			first = *(QS.a) - PHREDSHIFT;
			
			assert ( first < (int64_t)qualTable->n );
			assert ( qualTable->A[first] != UINT64_MAX );

			first = qualTable->A[first];
			
			prev = first;

			if ( incrementTable(firstTable, first) < 0 )
			{
				fprintf(stderr,"[E] unable to increment value\n");
				goto cleanup;
			}

			for ( c = QS.a+1; c < QS.e; ++c )
			{
				int64_t lv = *c - PHREDSHIFT;
				int64_t dif;
				int64_t v;
				
				assert ( lv < (int64_t)qualTable->n );
				assert ( qualTable->A[lv] != UINT64_MAX );
				
				lv = qualTable->A[lv];
				
				dif = lv - prev;
				
				if ( dif >= 0 )
				{
					v = (dif+1)*2;
				}
				else
				{
					v = (-dif * 2) | 1;
				}
				
				if ( incrementTable(difTable, v) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
				
				prev = lv;
			}
		
			#if 0
			fwrite(QS.a,1,QS.e-QS.a,stdout);
			fputc('\n',stdout);
			#endif

			nr += 1;
			
			if ( nr % (4*1024*1024) == 0 )
				fprintf(stderr,"[V] getQualityCode %lu %f\n", (unsigned long)nr, (double)ftello(in)/dfs);
				
			ql += 1;
		}
	}

	fprintf(stderr,"[V] getQualityCode %lu %f\n", (unsigned long)nr, (double)1.0);
	
	/* if we (possibly) did not read all the Q lines, then add an escape code in the Huffman tables */
	if ( maxlines != INT64_MAX )
	{
		if ( incrementTable(firstTable, HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}	
		if ( incrementTable(difTable, HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}	
	}

	if ( ! (firstPT = Table_getPairTable(firstTable)) )
	{
		fprintf(stderr,"[E] unable to produce firstPT\n");
		goto cleanup;
	}
	if ( ! (difPT = Table_getPairTable(difTable)) )
	{
		fprintf(stderr,"[E] unable to produce difPT\n");
		goto cleanup;	
	}	

	if ( computeHuffmanCodeLength(firstPT) != 0 )
	{
		fprintf(stderr,"[E] unable to produce code lengths from firstPT\n");
		goto cleanup;		
	}
	
	if ( ! (firstHC = computeHuffmanCodeFromLengths(&firstPT)) )
	{
		fprintf(stderr,"[E] failed computeHuffmanCodeFromLengths\n");
		goto cleanup;		
	}
	
	if ( computeHuffmanCodeLength(difPT) != 0 )
	{
		fprintf(stderr,"[E] unable to produce code lengths from difPT\n");
		goto cleanup;		
	}

	if ( ! (difHC = computeHuffmanCodeFromLengths(&difPT)) )
	{
		fprintf(stderr,"[E] failed computeHuffmanCodeFromLengths\n");
		goto cleanup;		
	}

	QH = QualityHuffman_allocate(firstHC,difHC);
	if ( ! QH )
	{	
		fprintf(stderr,"[E] unable to produce huffman struct\n");
		goto cleanup;					
	}
	
	firstHC = NULL;
	difHC = NULL;

	Table_deallocate(firstTable);
	Table_deallocate(difTable);
	LineBuffer_deallocate(LB);
	
	return QH;
		
	cleanup:
	LineBuffer_deallocate(LB);
	Table_deallocate(firstTable);
	Table_deallocate(difTable);
	PairTable_deallocate(firstPT);
	PairTable_deallocate(difPT);
	HuffmanCode_deallocate(firstHC);
	HuffmanCode_deallocate(difHC);
	
	return NULL;
}

QualityHuffman * getQualityCodeFromFile(char const * fn, Table const * qualityTable, int64_t const maxlines)
{
	FILE * in = fopen(fn,"r");
	QualityHuffman * QH = NULL;
	
	if ( ! in )
		return NULL;
		
	QH = getQualityCode(in,qualityTable,maxlines);
	
	fclose(in);
	
	return QH;		
}

int getQualityTableFromFile(char const * fn, int64_t const maxlines, Table ** rtable, Table ** rsymtable, Table ** rlengthsTable)
{
	FILE * in = fopen(fn,"r");
	
	if ( ! in )
		return -1;
		
	if ( getQualityTable(in,maxlines,rtable,rsymtable,rlengthsTable) < 0 )
	{
		fclose(in);
		return -1;
	}
	
	fclose(in);
	
	return 0;	
}


uint64_t BitLevelEncoder_getOffset(BitLevelEncoder const * A)
{
	return CHAR_BIT * A->w + A->f;
}

BitLevelEncoder * BitLevelEncoder_allocate(FILE * out)
{
	BitLevelEncoder * enc = (BitLevelEncoder *)malloc(sizeof(BitLevelEncoder));
	if ( ! enc )
		return NULL;
	
	enc->out = out;
	enc->c = 0;
	enc->f = 0;
	enc->w = 0;
	
	return enc;
}

int BitLevelEncoder_write(BitLevelEncoder * enc)
{
	if ( 
		fwrite(
			&(enc->c),
			sizeof(enc->c),
			1,
			enc->out
		)
		!=
		1
	)
	{
		return -1;
	}
	
	enc->f = 0;
	enc->c = 0;
	enc->w += 1;
	
	return 0;
}

int BitLevelEncoder_flush(BitLevelEncoder * enc)
{
	if ( enc->f )
	{
		unsigned int const cspace = sizeof(enc->c) * CHAR_BIT;
		unsigned int space = cspace - enc->f;

		enc->c <<= space;
		enc->f += space;
		
		if ( BitLevelEncoder_write(enc) != 0 )
			return -1;
	}
	
	return 0;
}

int BitLevelEncoder_encode(BitLevelEncoder * enc, uint64_t codeword, unsigned int codelength)
{
	while ( codelength )
	{
		/* space in output word */
		unsigned int const cspace = sizeof(enc->c) * CHAR_BIT;
		/* rest of space in current word */
		unsigned int space = cspace - enc->f;
		/* number of bits to put */
		unsigned int const toput = codelength <= space ? codelength : space;
		/* number of unencoded bits in codeword */
		unsigned int const unencoded = codelength - toput;

		assert ( space );

		enc->c <<= toput;
		enc->c |= (codeword >> unencoded);
		enc->f += toput;
		
		codelength -= toput;
		
		if ( enc->f == cspace )
		{
			if ( BitLevelEncoder_write(enc) != 0 )
				return -1;
		}
	}
	
	return 0;
}

int BitLevelEncoder_encodeGamma(BitLevelEncoder * enc, uint64_t v)
{
	unsigned int l = 0;
	uint64_t vv = v;
	
	while ( vv )
	{
		vv >>= 1;
		l += 1;
	}

	/* write length of number */
	if ( BitLevelEncoder_encode(enc,1 /* code */,l+1 /* length */) != 0 )
		return -1;

	/* write number */
	if ( BitLevelEncoder_encode(enc,v /* code */,l /* length */) != 0 )
		return -1;
		
	return 0;
}

int BitLevelEncoder_deallocate(BitLevelEncoder * enc)
{
	int r = 0;
	if ( enc )
	{
		r = BitLevelEncoder_flush(enc);
		free(enc);
	}
	
	return r;
}


BitLevelDecoder * BitLevelDecoder_allocate(FILE * in)
{
	BitLevelDecoder * BLD = (BitLevelDecoder *)malloc(sizeof(BitLevelDecoder));
	if ( ! BLD )
		return NULL;
	
	BLD->in = in;
	BLD->c = 0;
	BLD->f = 0;
	
	return BLD;
}

int BitLevelDecoder_load(BitLevelDecoder * BLV)
{
	assert ( BLV->f == 0 );
	
	if ( fread(&BLV->c,sizeof(BLV->c),1,BLV->in) != 1 )
		return -1;
	
	BLV->f = sizeof(BLV->c) * CHAR_BIT;
	
	return 0;
}

int BitLevelDecoder_getBit(BitLevelDecoder * BLV)
{
	int b;

	if ( BLV->f == 0 )
	{
		if ( BitLevelDecoder_load(BLV) != 0 )
			return -1;
	}
	
	b = (BLV->c >> (--BLV->f)) & 1;
	
	return b;
}

int BitLevelDecoder_decodeGamma(BitLevelDecoder * BLV, uint64_t * v)
{
	unsigned int l = 0;
	
	
	while ( 1 )
	{
		int const r = BitLevelDecoder_getBit(BLV);
		
		if ( r < 0 )
			return -1;
		
		if ( r )
			break;
		else
			l += 1;
	}
	
	*v = 0;
	
	while ( l-- )
	{
		int const r = BitLevelDecoder_getBit(BLV);
		
		if ( r < 0 )
			return -1;
		
		*v <<= 1;
		*v |= r;	
	}
	
	return 0;
}

int BitLevelDecoder_decode(BitLevelDecoder * BLV, uint64_t * v, unsigned int l)
{
	*v = 0;
	
	while ( l-- )
	{
		int const r = BitLevelDecoder_getBit(BLV);
		
		if ( r < 0 )
			return -1;
		
		*v <<= 1;
		*v |= r;	
	}
	
	return 0;
}

void BitLevelDecoder_deallocate(BitLevelDecoder * BLV)
{
	if ( BLV )
		free(BLV);
}

char * concat(char const * a, char const * b)
{
	size_t const la = strlen(a);
	size_t const lb = strlen(b);
	size_t const l = la + lb;
	char * c = (char *)malloc(l+1);
	
	if ( ! c )
		return NULL;
	
	memcpy(c, a, la);
	memcpy(c+la, b, lb);
	c[l] = 0;
	
	return c;
}

typedef struct _ProvenanceStep
{
	char * program;
	char * version;
	char * commandline;
	char * date;
	struct _ProvenanceStep * next;
} ProvenanceStep;

char * CString_tostring(CString const * a)
{
	size_t const l = a->e - a->a;
	char * c = (char *)malloc(sizeof(char)*(l+1));
	if ( ! c )
		return NULL;
	c[l] = 0;
	memcpy(c,a->a,l);
	return c;
}

#include <time.h>

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
		
	P->program = strdup(argv[0]);
	
	if ( ! P->program )
		goto cleanup;

		
	P->version = strdup(version);
	
	if ( ! P->version )
		goto cleanup;
		
	t = time(NULL);
	p = ctime(&t);
	
	P->date = strdup(p);
	
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

int BitLevelEncoder_encodeString(BitLevelEncoder * BLE, char const * c)
{
	size_t const l = strlen(c);
	size_t i;
	
	if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
		return -1;
		
	for ( i = 0; i < l; ++i )
		if ( BitLevelEncoder_encode(BLE,c[i],8) < 0 )
			return -1;
			
	return 0;
}

char * BitLevelEncoder_decodeString(BitLevelDecoder * BLD)
{
	uint64_t v;
	uint64_t l;
	uint64_t i;
	char * c = NULL;
	
	if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		return NULL;
		
	l = v;
	
	c = (char *)malloc(sizeof(char)*(l+1));
	if ( ! c )
		return NULL;
		
	c[l] = 0;
		
	for ( i = 0; i < l; ++i )
	{
		if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		{
			free(c);
			return NULL;
		}
		else
			c[i] = v;
	}
			
	return c;
}



void ProvenanceStep_print(FILE * out, ProvenanceStep * P)
{
	while ( P )
	{
		fprintf(out,"!");
		
		fprintf(out," %lu %s",(unsigned long)strlen(P->program),P->program);
		fprintf(out," %lu %s",(unsigned long)strlen(P->version),P->version);
		fprintf(out," %lu %s",(unsigned long)strlen(P->commandline),P->commandline);
		fprintf(out," %lu %s",(unsigned long)strlen(P->date),P->date);
		fprintf(out,"\n");
	
		P = P->next;
	}
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

		program = BitLevelEncoder_decodeString(BLD);
		version = BitLevelEncoder_decodeString(BLD);
		commandline = BitLevelEncoder_decodeString(BLD);
		date = BitLevelEncoder_decodeString(BLD);

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


typedef struct _HeaderStatsLine
{
	char type;
	char subtype;
	uint64_t num;
} HeaderStatsLine;

int HeaderStatsLine_encode(BitLevelEncoder * BLE, HeaderStatsLine const * HLE)
{
	if ( BitLevelEncoder_encode(BLE,HLE->type,8) < 0 )
		return -1;
	if ( BitLevelEncoder_encode(BLE,HLE->subtype,8) < 0 )
		return -1;
	if ( BitLevelEncoder_encodeGamma(BLE,HLE->num) < 0 )
		return -1;
		
	return 0;
}

int HeaderStatsLine_decode(BitLevelDecoder * BLD, HeaderStatsLine * HLE)
{
	uint64_t v;
	
	if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		return -1;
	
	HLE->type = v;

	if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		return -1;
	
	HLE->subtype = v;

	if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		return -1;
		
	HLE->num = v;
	
	return 0;
}

int HeaderStatsLine_push(HeaderStatsLine ** PSH, uint64_t * HSLo, uint64_t * HSLn, HeaderStatsLine NHSL)
{
	if ( *HSLo == *HSLn )
	{
		uint64_t const newsize = *HSLn ? 2*(*HSLn) : 1;
		
		HeaderStatsLine * NSH = (HeaderStatsLine *)malloc(sizeof(HeaderStatsLine)*newsize);
		if ( ! NSH )
			return -1;
			
		memcpy(
			NSH,
			*PSH,
			sizeof(HeaderStatsLine) * (*HSLn)
		);
		
		free(*PSH);
		*PSH = NSH;
		*HSLn = newsize;
	}
	
	(*PSH)[(*HSLo)++] = NHSL;
	
	return 0;
}

int produceBinary(FILE * out, FILE * in, QualityHuffman const * QH, Table * qualTable, char const * tmpfn, ProvenanceStep ** insPS, HuffmanCode * symCode, HuffmanCode * lengthsCode)
{
	LineBuffer * LB = NULL;
	char const * a;
	char const * e;
	char const * ft = "seq";
	int returnvalue = 0;
	CString CS;
	BitLevelEncoder * BLE = NULL;
	size_t nr = 0;
	size_t numindex = 0;
	uint64_t fs;
	double dfs;
	size_t * reverseQualTable = NULL;
	size_t ii;
	size_t reverseQualTableSize = 0;
	uint64_t maxqualvalue = 0;
	uint64_t const indexmod = 1024;
	FILE * tmpfile = NULL;
	uint64_t indexposition = 0;
	uint64_t i;
	uint64_t numenc = 0;
	uint64_t numbits = 0;
	HuffmanCode const * firstHuf = NULL;
	HuffmanCode const * difHuf = NULL;
	CodeTable const * firstSparse = NULL;
	CodeTable const * difSparse = NULL;
	char * subtype = NULL;
	ProvenanceStep * PS = NULL;
	ProvenanceStep * PSL = NULL;
	HeaderStatsLine * HSL = NULL;
	uint64_t HSLo = 0;
	uint64_t HSLn = 0;
	char * filetype = NULL;
	int64_t fileversion = -1;
	int64_t filesubversion = -1;
	char const * cc;

	tmpfile = fopen(tmpfn,"wb");

	if ( ! tmpfile )
	{
		fprintf(stderr,"[E] unable to open tmp file\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( ii = 0; ii < qualTable->n; ++ii )
		if ( qualTable->A[ii] != UINT64_MAX )
		{
			/* ii -> qualTable->A[ii] */
			if ( qualTable->A[ii] > maxqualvalue )
				maxqualvalue = qualTable->A[ii];
		}
		
	reverseQualTableSize = maxqualvalue+1;

	reverseQualTable = (size_t *)malloc(sizeof(size_t)*reverseQualTableSize);

	for ( ii = 0; ii < qualTable->n; ++ii )
		if ( qualTable->A[ii] != UINT64_MAX )
			reverseQualTable[qualTable->A[ii]] = ii;
	
	#if 0
	for ( ii = 0; ii < reverseQualTableSize; ++ii )
		fprintf(stderr,"reverseQualTable[%d]=%d\n", (int)ii, (int)reverseQualTable[ii]);
	#endif
	
	fseek(in,0,SEEK_END);
	fs = ftello(in);
	fseek(in,0,SEEK_SET);
	dfs = fs;

	/* allocate line buffer */
	if ( ! (LB = LineBuffer_allocate(in,1)) )
	{
		fprintf(stderr,"[E] unable to instantiate line buffer\n");
		returnvalue = -1;
		goto cleanup;		
	}

	/* read line */
	if ( LineBuffer_getline(LB,&a,&e) <= 0 )
	{
		fprintf(stderr,"[E] unable to get first line of file\n");
		returnvalue = -1;
		goto cleanup;
	}
	if ( ! expect(&a,&e,'1') )
	{
		fprintf(stderr,"[E] first line of file does not start with a 1\n");
		returnvalue = -1;
		goto cleanup;	
	}
	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		returnvalue = -1;
		goto cleanup;	
	}
	
	CS = getString(&a,&e);
	
	if ( ! CS.a )
	{		
		fprintf(stderr,"[E] unable to read file type\n");
		returnvalue = -1;
		goto cleanup;	
	}
	
	if ( (CS.e - CS.a) != (ptrdiff_t)(strlen(ft)) )
	{
		fprintf(stderr,"[E] file type has the wrong length\n");
		returnvalue = -1;
		goto cleanup;		
	}

	if ( ! (filetype = CString_tostring(&CS)) )
	{
		fprintf(stderr,"[E] unable to copy file type\n");
		returnvalue = -1;
		goto cleanup;			
	}
	
	/* check file type */
	if ( strcmp(filetype,ft) != 0 )
	{
		fprintf(stderr,"[E] unexpected file type\n");
		returnvalue = -1;
		goto cleanup;			
	}

	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		returnvalue = -1;
		goto cleanup;	
	}

	fileversion = getNumber(&a,&e);
	if ( fileversion < 0 )
	{
		fprintf(stderr,"[E] failed to get version number after file type\n");
		returnvalue = -1;
		goto cleanup;		
	}

	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after version\n");
		returnvalue = -1;
		goto cleanup;	
	}

	filesubversion = getNumber(&a,&e);
	if ( filesubversion < 0 )
	{
		fprintf(stderr,"[E] failed to get version number after file type\n");
		returnvalue = -1;
		goto cleanup;		
	}

	firstHuf = QH->firstHuf;
	difHuf = QH->difHuf;
	firstSparse = firstHuf->CTsortedSparse;
	difSparse = difHuf->CTsortedSparse;

	while ( LineBuffer_getline(LB,&a,&e) )
	{
		if ( (e-a) && isalpha(a[0]) )
		{
			LineBuffer_putback(LB,a);
			break;
		}
		else
		{
			if ( 
				e-a &&
				(
					a[0] == '#'
					||
					a[0] == '+'
					||
					a[0] == '@'
				)
			)
			{
				char type, subtype;
				int64_t num;
				
				if ( e-a < 5 || a[1] != ' ' || a[3] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(a,e-a,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;			
				}
				
				type = a[0];
				subtype = a[2];
				
				a += 4;
				
				num = getNumber(&a,&e);

				HeaderStatsLine LHSL;
				LHSL.type = type;
				LHSL.subtype = subtype;
				LHSL.num = num;

				if ( HeaderStatsLine_push(&HSL, &HSLo, &HSLn, LHSL) < 0 )
				{				
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(a,e-a,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;			
				}

				fprintf(stderr,"[V] header line type %c subtype %c num %ld\n",type,subtype,num);
			}
			else if (
				e-a
				&&
				a[0] == '2'
			)
			{
				char const * linestart = a;
				CString subtypeCS;
				
				if ( e-a < 2 || a[1] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				a += 2;

				subtypeCS = getString(&a,&e);
	
				if ( ! subtypeCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}
				
				subtype = (char *)malloc(sizeof(char) * ((subtypeCS.e - subtypeCS.a) + 1));
				if ( ! subtype )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}
				
				subtype[subtypeCS.e - subtypeCS.a] = 0;
				memcpy(subtype,subtypeCS.a,subtypeCS.e - subtypeCS.a);
				
				fprintf(stderr,"[V] header line found subtype %s\n",subtype);
			}
			else if (
				e-a
				&&
				a[0] == '!'
			)
			{
				char const * linestart = a;
				CString progCS;
				CString progverCS;
				CString comlinCS;
				CString dateCS;
				ProvenanceStep * LPS = NULL;

				fprintf(stderr,"[V] header provenance line: ");
				fwrite(linestart,e-linestart,1,stderr);
				fprintf(stderr,"\n");
				
				if ( e-a < 2 || a[1] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				a += 2;
				
				progCS = getString(&a,&e);
				
				if ( ! progCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}

				if ( e-a < 1 || a[0] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				a += 1;

				progverCS = getString(&a,&e);
				
				if ( ! progverCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}

				if ( e-a < 1 || a[0] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				a += 1;

				comlinCS = getString(&a,&e);
				
				if ( ! comlinCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}

				if ( e-a < 1 || a[0] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				a += 1;

				dateCS = getString(&a,&e);
				
				if ( ! dateCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}
				
				LPS = ProvenanceStep_allocate(&progCS,&progverCS,&comlinCS,&dateCS);
				
				if ( ! LPS )
				{				
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;				
				}
				
				ProvenanceStep_add(&PS,&PSL,LPS);
			}
			else
			{
				char const * linestart = a;

				fprintf(stderr,"[V] header line: ");
				fwrite(linestart,e-linestart,1,stderr);
				fprintf(stderr,"\n");
			}
		}
	}
	
	ProvenanceStep_add(&PS,&PSL,*insPS);
	*insPS = NULL;

	for ( i = 0; i < HSLo; ++i )
		if ( HSL[i].type == '#' && HSL[i].subtype == '!' )
			HSL[i].num += 1;
		
	for ( i = 0; i < HSLo; ++i )
	{
		fprintf(stderr,"%c %c %lu\n", HSL[i].type, HSL[i].subtype, HSL[i].num);
	}

	ProvenanceStep_print(stderr, PS);

	if ( ! (BLE = BitLevelEncoder_allocate(out)) )
	{
		fprintf(stderr,"[E] unable to instantiate BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;			
	}
	
	for ( cc = binfiletype; *cc; ++cc )
		if ( BitLevelEncoder_encode(BLE,*cc,8) < 0 )
		{
			fprintf(stderr,"[E] unable to add version line in BitLevelEncoder\n");
			returnvalue = -1;
			goto cleanup;				
		}

	if ( BitLevelEncoder_encode(BLE,0,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add padding in BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;				
	}

	if ( BitLevelEncoder_encode(BLE,HSLo,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add number of header objects\n");
		returnvalue = -1;
		goto cleanup;					
	}
	
	for ( i = 0; i < HSLo; ++i )
		if ( HeaderStatsLine_encode(BLE,&(HSL[i])) < 0 )
		{
			fprintf(stderr,"[E] unable to add header objects\n");
			returnvalue = -1;
			goto cleanup;				
		}

	if ( ProvenanceStep_encode(BLE, PS) < 0 )
	{
		fprintf(stderr,"[E] unable to encode provenance\n");
		returnvalue = -1;
		goto cleanup;					
	}

	if ( BitLevelEncoder_encode(BLE,indexmod,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add padding in BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;				
	}

	if ( QualityHuffman_encode(BLE,QH) < 0 )
	{
		fprintf(stderr,"[E] unable to write Huffman code tables to output file\n");
		returnvalue = -1;
		goto cleanup;
	}
	
	if ( BitLevelEncoder_encodeGamma(BLE,reverseQualTableSize) < 0 )
	{
		fprintf(stderr,"[E] unable to write size of reverse quality table to output file\n");
		returnvalue = -1;
		goto cleanup;	
	}
	
	for ( ii = 0; ii < reverseQualTableSize; ++ii )
		if ( BitLevelEncoder_encodeGamma(BLE,reverseQualTable[ii]) < 0 )
		{
			fprintf(stderr,"[E] unable to write reverse quality table to output file\n");
			returnvalue = -1;
			goto cleanup;	
		}

	if ( HuffmanCode_encode(BLE, symCode) < 0 )
	{
		fprintf(stderr,"[E] unable to write sym huffman tables to output file\n");
		returnvalue = -1;
		goto cleanup;		
	}

	if ( HuffmanCode_encode(BLE, lengthsCode) < 0 )
	{
		fprintf(stderr,"[E] unable to write lengths huffman tables to output file\n");
		returnvalue = -1;
		goto cleanup;		
	}

	while ( LineBuffer_getline(LB,&a,&e) )
	{
		if ( e-a )
		{
			char const linetype = a[0];
			
			if ( linetype == 'P' )
			{
				#if 0
				if ( BitLevelEncoder_flush(BLE) < 0 )
				{
					fprintf(stderr,"[E] BitLevelEncoder_flush failed\n");
					returnvalue = -1;
					goto cleanup;
				}
				#endif
				
				uint64_t const offset = BitLevelEncoder_getOffset(BLE);
				
				if ( nr % indexmod == 0 )
				{
					if ( fwrite(
						&offset,
						sizeof(offset),
						1,
						tmpfile
						) != 1
					)
					{
						fprintf(stderr,"[E] unable to write to tmp file\n");
						returnvalue = -1;
						goto cleanup;
					}
					
					numindex += 1;
				}

				nr += 1;
			
				if ( nr % (4*1024*1024) == 0 )
					fprintf(stderr,"[V] produceBinary number of pairs %lu input processed %f bits per symbol %f\n",
						(unsigned long)nr, (double)ftello(in)/dfs,
						BitLevelEncoder_getOffset(BLE) / (double)numenc
					);
				
				if ( HuffmanCode_encodeSymbol(BLE, symCode, 'P' ) < 0 )
				{
					fprintf(stderr,"[E] HuffmanCode_encode(P) failed\n");
					returnvalue = -1;
					goto cleanup;				
				}
			}
			else if ( linetype == 'Q' )
			{
				CString QS;
				char const * c;
				int64_t first;
				int64_t prev;
				CodeTableEntry * firstcode;
				int needfirstescape;
				int64_t l;
				
				a += 1;
				
				if ( ! expect(&a,&e,' ') )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					returnvalue = -1;
					goto cleanup;
				}

				QS = getString(&a,&e);
				
				if ( ! QS.a )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					returnvalue = -1;
					goto cleanup;			
				}
		
				for ( c = QS.a; c < QS.e; ++c )
					if ( *c < PHREDSHIFT || *c > 93 )
					{
						fprintf(stderr,"[E] malformed Q line\n");
						returnvalue = -1;
						goto cleanup;
					}

				if ( HuffmanCode_encodeSymbol(BLE, symCode, 'Q' ) < 0 )
				{
					fprintf(stderr,"[E] HuffmanCode_encodeSymbol(Q) failed\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				l = QS.e - QS.a;

				if ( l < HUFFMAN_ESCAPE_CODE )
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, l) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(Q_length) failed\n");
						returnvalue = -1;
						goto cleanup;		
					}
				}
				else
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, HUFFMAN_ESCAPE_CODE) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(Q_length) failed\n");
						returnvalue = -1;
						goto cleanup;		
					}
					if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(Q_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}
				
				first = *(QS.a) - PHREDSHIFT;

				assert ( first < (int64_t)qualTable->n );
				assert ( qualTable->A[first] != UINT64_MAX );

				/* fprintf(stderr,"mapping %d to %d\n", first, qualTable->A[first]); */
				
				first = qualTable->A[first];
				
				needfirstescape = (firstSparse->A[first].codelength == UINT64_MAX);
				
				firstcode = needfirstescape ?
					&(firstSparse->A[HUFFMAN_ESCAPE_CODE])
					:
					&(firstSparse->A[first]);
				assert ( firstcode->codelength != UINT64_MAX );
				
				if ( BitLevelEncoder_encode(BLE,firstcode->code,firstcode->codelength) < 0 )
				{
					fprintf(stderr,"[E] BitLevelEncoder_encode(first) failed\n");
					returnvalue = -1;
					goto cleanup;
				}
				if ( needfirstescape )
				{
					if ( BitLevelEncoder_encodeGamma(BLE,first) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(first) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}
				numenc += 1;
				numbits += firstcode->codelength;
				
				prev = first;

				/* handle first */

				for ( c = QS.a+1; c < QS.e; ++c )
				{
					int64_t lv = *c - PHREDSHIFT;
					CodeTableEntry * difcode;
					int64_t dif;
					int64_t v;
					int needescape;
					
					assert ( lv < (int64_t)qualTable->n );
					assert ( qualTable->A[lv] != UINT64_MAX );
					
					/* fprintf(stderr,"mapping %d to %d\n", lv, qualTable->A[lv]); */
					
					lv = qualTable->A[lv];

					dif = lv - prev;
					
					/* lv = dif + prev */
					
					
					if ( dif >= 0 )
					{
						v = (dif+1)*2;
					}
					else
					{
						v = (-dif * 2) | 1;
					}

					/* fprintf(stderr,"odifsym=%d odif=%d\n",(int)v,(int)dif); */
					
					needescape = (difSparse->A[v].codelength == UINT64_MAX);

					difcode = needescape ? &(difSparse->A[HUFFMAN_ESCAPE_CODE]) : &(difSparse->A[v]);
					assert ( difcode->codelength != UINT64_MAX );

					if ( BitLevelEncoder_encode(BLE,difcode->code,difcode->codelength) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encode(dif) failed\n");
						returnvalue = -1;
						goto cleanup;		
					}
					if ( needescape )
					{
						if ( BitLevelEncoder_encodeGamma(BLE,v) < 0 )
						{
							fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(dif) failed\n");
							returnvalue = -1;
							goto cleanup;					
						}
					}
					
					numenc += 1;
					numbits += difcode->codelength;

					/* printCodeTableEntry(stderr,difcode); */
					
					/* handle v */
					
					prev = lv;
				}		

				#if 0
				nr += 1;
			
				if ( nr % (4*1024*1024) == 0 )
					fprintf(stderr,"[V] produceBinary %lu %f\n", (unsigned long)nr, (double)ftello(in)/dfs);
				#endif

			}
			else if ( linetype == 'S' )
			{
				CString QS;
				char const * c;
				int64_t l;
				
				a += 1;
				
				if ( ! expect(&a,&e,' ') )
				{
					fprintf(stderr,"[E] malformed S line\n");
					returnvalue = -1;
					goto cleanup;
				}

				QS = getString(&a,&e);
				
				if ( ! QS.a )
				{
					fprintf(stderr,"[E] malformed S line\n");
					returnvalue = -1;
					goto cleanup;			
				}

				if ( HuffmanCode_encodeSymbol(BLE, symCode, 'S' ) < 0 )
				{
					fprintf(stderr,"[E] HuffmanCode_encodeSymbol(S) failed\n");
					returnvalue = -1;
					goto cleanup;
				}
				
				l = QS.e - QS.a;
				
				if ( l < HUFFMAN_ESCAPE_CODE )
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, l) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(S_length) failed\n");
						returnvalue = -1;
						goto cleanup;		
					}
				}
				else
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, HUFFMAN_ESCAPE_CODE) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(S_length) failed\n");
						returnvalue = -1;
						goto cleanup;		
					}
					if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(S_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}

		
				for ( c = QS.a; c < QS.e; ++c )
				{
					unsigned int sym = 0;
					
					switch ( *c )
					{
						case 'a':
						case 'A':
							sym = 0;
							break;
						case 'c':
						case 'C':
							sym = 1;
							break;
						case 'g':
						case 'G':
							sym = 2;
							break;
						case 't':
						case 'T':
							sym = 3;
							break;
					}
					
					assert ( sym < 4 );

					if ( BitLevelEncoder_encode(BLE,sym,2) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encode(S data) failed\n");
						returnvalue = -1;
						goto cleanup;			
					}
				}

				numbits += 2*(QS.e-QS.a);				
			}
		}
	}
	
	if ( BitLevelEncoder_flush(BLE) < 0 )
	{
		fprintf(stderr,"[E] BitLevelEncoder_flush() failed\n");
		returnvalue = -1;
		goto cleanup;			
	}
	
	if ( fclose(tmpfile) != 0 )
	{
		fprintf(stderr,"[E] unable correctly close tmp file\n");
		returnvalue = -1;
		goto cleanup;				
	}
	
	indexposition = BitLevelEncoder_getOffset(BLE);

	tmpfile = fopen(tmpfn,"rb");
	
	if ( ! tmpfile )
	{
		fprintf(stderr,"[E] unable re open tmp file\n");
		returnvalue = -1;
		goto cleanup;					
	}
	
	for ( i = 0; i < numindex; ++i )
	{
		uint64_t v;
		if ( fread(&v,sizeof(uint64_t),1,tmpfile) != 1 )
		{
			fprintf(stderr,"[E] unable read tmp file\n");
			returnvalue = -1;
			goto cleanup;					
		}
		
		if ( BitLevelEncoder_encode(BLE,v,64) < 0 )
		{
			fprintf(stderr,"[E] unable write index dara\n");
			returnvalue = -1;
			goto cleanup;							
		}
	}

	if ( BitLevelEncoder_flush(BLE) < 0 )
	{
		fprintf(stderr,"[E] BitLevelEncoder_flush() failed\n");
		returnvalue = -1;
		goto cleanup;			
	}
	
	for ( i = 0; i < sizeof(uint64_t); ++i )
	{
		unsigned char const u = indexposition >> ((sizeof(uint64_t) - i - 1)*8) & 0xFF;
		if ( fwrite(&u,1,1,out) != 1 )
		{
			fprintf(stderr,"[E] fwrite index failed\n");
			returnvalue = -1;
			goto cleanup;
		}
	}
	
	fseek(out,0,SEEK_SET);

	for ( cc = binfiletype; *cc; ++cc )
		if ( BitLevelEncoder_encode(BLE,*cc,8) < 0 )
		{
			fprintf(stderr,"[E] unable to add version line in BitLevelEncoder\n");
			returnvalue = -1;
			goto cleanup;				
		}

	if ( BitLevelEncoder_encode(BLE,nr,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add number of reads in BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;				
	}
	
	fprintf(stderr,"[V] produceBinary number of sequences %lu input processed %f bits per symbol %f\n",
		(unsigned long)nr, (double)ftello(in)/dfs,
		BitLevelEncoder_getOffset(BLE) / (double)numenc
	);
	/* fprintf(stderr,"numenc %lu bits %lu bits per symbol %f\n", (unsigned long)numenc, (unsigned long)numbits, (double)numbits/numenc); */

	remove(tmpfn);

	cleanup:
	ProvenanceStep_deallocate(PS);
	if ( tmpfile )
	{
		fclose(tmpfile);
		tmpfile = NULL;
	}
	LineBuffer_deallocate(LB);
	BitLevelEncoder_deallocate(BLE);
	free(reverseQualTable);
	free(subtype);
	free(HSL);
	free(filetype);
	
	return returnvalue;
}

int produceBinaryFromFile(char const * outfn, char const * fn, QualityHuffman const * QH, Table * qualTable, ProvenanceStep ** insPS, HuffmanCode * symCode, HuffmanCode * lengthsCode)
{
	char * tmpfn = concat(outfn,".tmp");
	FILE * in = NULL;
	FILE * out = NULL;
	int r;

	if ( ! tmpfn )
		return -1;
	
	in = fopen(fn,"r");
	
	if ( ! in )
	{
		free(tmpfn);
		return -1;
	}

	out = fopen(outfn,"wb");
	
	if ( ! out )
	{
		free(tmpfn);
		fclose(in);
		return -1;
	}
		
	r = produceBinary(out,in,QH,qualTable,tmpfn,insPS,symCode,lengthsCode);
	
	free(tmpfn);
	fclose(in);
	fclose(out);
	
	return r;		
}

int produceBinaryFile(char const * outfn, char const * infn, int64_t const maxstatlines, ProvenanceStep ** insPS)
{
	Table * QT = NULL;
	Table * symTable = NULL;
	Table * lengthsTable = NULL;
	PairTable * symPT = NULL;
	PairTable * lengthsPT = NULL;
	QualityHuffman * QH = NULL;
	HuffmanCode * symCode = NULL;
	HuffmanCode * lengthsCode = NULL;
	int r;
	size_t i;
	int returnvalue = 0;

	if ( getQualityTableFromFile(infn,maxstatlines,&QT,&symTable,&lengthsTable) < 0 )
	{
		fprintf(stderr,"[E] failed to compute quality table\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( i = 0; i < QT->n; ++i )
		if ( QT->A[i] != UINT64_MAX )
			fprintf(stderr,"QT[%d]=%d\n", (int)i, (int)QT->A[i]);

	symPT = Table_getPairTable(symTable);
	
	if ( ! symPT )
	{	
		fprintf(stderr,"[E] failed to compute sym pair table\n");
		returnvalue = -1;
		goto cleanup;
	}
	
	Table_deallocate(symTable);
	symTable = NULL;
	
	if ( computeHuffmanCodeLength(symPT) < 0 )
	{
		fprintf(stderr,"[E] failed to compute code for symbol information\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] symbol table\n");
	for ( i = 0; i < symPT->n; ++i )
	{
		uint64_t const key = symPT->A[i].key;
		uint64_t const value = symPT->A[i].value;
		
		if ( key < HUFFMAN_ESCAPE_CODE && isprint(key) )
			fprintf(stderr,"%c %d\n",(char)key,(int)value);
		else
			fprintf(stderr,"%d %d\n",(int)key,(int)value);
	}


	/* lenghts */
	lengthsPT = Table_getPairTable(lengthsTable);
	
	if ( ! lengthsPT )
	{	
		fprintf(stderr,"[E] failed to compute lenghts pair table\n");
		returnvalue = -1;
		goto cleanup;
	}
	
	Table_deallocate(lengthsTable);
	lengthsTable = NULL;
	
	if ( computeHuffmanCodeLength(lengthsPT) < 0 )
	{
		fprintf(stderr,"[E] failed to compute code for lenghtsbol information\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] lengths table\n");
	for ( i = 0; i < lengthsPT->n; ++i )
	{
		uint64_t const key = lengthsPT->A[i].key;
		uint64_t const value = lengthsPT->A[i].value;
		
		if ( key < HUFFMAN_ESCAPE_CODE && isprint(key) )
			fprintf(stderr,"%c %d\n",(char)key,(int)value);
		else
			fprintf(stderr,"%d %d\n",(int)key,(int)value);
	}
	
	if ( !(symCode = computeHuffmanCodeFromLengths(&symPT)) )
	{
		fprintf(stderr,"[E] unable to construct huffman code for symbols");
		returnvalue = -1;
		goto cleanup;
	}

	if ( !(lengthsCode = computeHuffmanCodeFromLengths(&lengthsPT)) )
	{
		fprintf(stderr,"[E] unable to construct huffman code for lenghts");
		returnvalue = -1;
		goto cleanup;
	}
	/* HuffmanCode * computeHuffmanCodeFromLengths(PairTable ** firstPT); */

	QH = getQualityCodeFromFile(infn,QT,maxstatlines);

	if ( ! QH )
	{
		fprintf(stderr,"[E] failed to compute code for quality information\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] first huffman table\n");	
	printCodeTable(stderr,QH->firstHuf->CT);
	fprintf(stderr,"[V] dif huffman table\n");	
	printCodeTable(stderr,QH->difHuf->CT);

	r = produceBinaryFromFile(outfn,infn, QH, QT, insPS, symCode, lengthsCode);
	
	if ( r < 0 )
	{
		Table_deallocate(QT);
		fprintf(stderr,"[E] produceBinaryFromFile failed\n");
		return -1;		
	}
	
	cleanup:
	HuffmanCode_deallocate(symCode);
	HuffmanCode_deallocate(lengthsCode);
	Table_deallocate(QT);
	Table_deallocate(symTable);
	Table_deallocate(lengthsTable);
	PairTable_deallocate(symPT);
	PairTable_deallocate(lengthsPT);
	QualityHuffman_deallocate(QH);
	return returnvalue;
}


typedef struct _DecodeResult
{
	char * S;
	uint64_t S_o;
	uint64_t S_l;
	
	char * Q;
	uint64_t Q_o;
	uint64_t Q_l;
} DecodeResult;

int decodeSequenceAndQuality(
	BitLevelDecoder * BLD,
	QualityHuffman * QH,
	HuffmanCode * symCode,
	HuffmanCode * lengthsCode,
	uint64_t * reverseQualityTable,
	DecodeResult * D
)
{
	/* uint64_t v; */
	uint64_t seqlen;
	uint64_t qlen;
	uint64_t i;
	int returncode = 0;
	int64_t firstqual;
	int64_t prevqual;
	int64_t firstqualsym;
	int64_t llv;

	if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'S' )
	{
		fprintf(stderr,"[E] failed to find sequence after P marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( (llv = HuffmanCode_decodeSymbol(lengthsCode, BLD)) < 0 )
	{
		fprintf(stderr,"[E] failed to decode length of sequence after S marker\n");
		returncode = -1;
		goto cleanup;
	}
	
	if ( llv < HUFFMAN_ESCAPE_CODE )
	{
		seqlen = llv;
	}
	else
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&seqlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read sequence length after P marker\n");
			returncode = -1;
			goto cleanup;		
		}
	}
	
	while ( seqlen > D->S_o )
	{
		size_t nlen = D->S_o ? 2 * D->S_o : 1;
		free(D->S);

		D->S = NULL;
		D->S_o = 0;		
		D->S = (char *)malloc(nlen);
		
		if ( ! D->S )
		{
			fprintf(stderr,"[E] failed to allocate read buffer\n");
			returncode = -1;
			goto cleanup;
		}
		
		D->S_o = nlen;
	}
	
	/* fprintf(stderr,"seqlen %lu\n", (unsigned long)seqlen); */
	
	for ( i = 0; i < seqlen; ++i )
	{
		uint64_t sym;
		if ( BitLevelDecoder_decode(BLD,&sym,2) < 0 )
		{
			fprintf(stderr,"[E] failed to decode symbol\n");
			returncode = -1;
			goto cleanup;
		}
		
		switch ( sym )
		{
			case 0: sym = 'A'; break;
			case 1: sym = 'C'; break;
			case 2: sym = 'G'; break;
			case 3: sym = 'T'; break;
		}
		
		/* fprintf(stderr,"%c", (char)sym); */
		D->S[i] = sym;
	}
	
	D->S_l = seqlen;
	
	/* fprintf(stderr,"\n"); */

	if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'Q' )
	{
		fprintf(stderr,"[E] failed to find Q marker after sequence\n");
		returncode = -1;
		goto cleanup;
	}

	if ( (llv = HuffmanCode_decodeSymbol(lengthsCode, BLD)) < 0 )
	{
		fprintf(stderr,"[E] failed to decode length of sequence after Q marker\n");
		returncode = -1;
		goto cleanup;
	}
	
	if ( llv < HUFFMAN_ESCAPE_CODE )
	{
		qlen = llv;
	}
	else
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&qlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read q length after P marker\n");
			returncode = -1;
			goto cleanup;		
		}
	}


	while ( qlen > D->Q_o )
	{
		size_t nlen = D->Q_o ? 2 * D->Q_o : 1;
		free(D->Q);

		D->Q = NULL;
		D->Q_o = 0;		
		D->Q = (char *)malloc(nlen);
		
		if ( ! D->Q )
		{
			fprintf(stderr,"[E] failed to allocate read buffer\n");
			returncode = -1;
			goto cleanup;
		}
		
		D->Q_o = nlen;
	}

	/* fprintf(stderr,"qlen %lu\n", (unsigned long)qlen); */
	
	firstqualsym = HuffmanCode_decodeSymbol(QH->firstHuf, BLD);
		
	if ( firstqualsym < 0 )
	{
		fprintf(stderr,"[E] failed to read first quality value\n");
		returncode = -1;
		goto cleanup;		
	}
	
	if ( firstqualsym == HUFFMAN_ESCAPE_CODE )
	{
		uint64_t v;
		
		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			fprintf(stderr,"[E] failed to read first quality value\n");
			returncode = -1;
			goto cleanup;
		}
		
		firstqualsym = v;
	}
	
	firstqual = reverseQualityTable[firstqualsym];
	prevqual = firstqualsym;

	/* fprintf(stderr,"firstqualsym %d firstqual %d\n", (int)firstqualsym, (int)firstqual); */
	/* fprintf(stderr,"%c",(char)(firstqual + PHREDSHIFT)); */
	D->Q[0] = firstqual + PHREDSHIFT;
	
	for ( i = 1; i < qlen; ++i )
	{
		int64_t difsym;
		int64_t dif;
		int64_t nextqual;
		int64_t nextqualvalue;

		difsym = HuffmanCode_decodeSymbol(QH->difHuf, BLD);
		
		if ( difsym < 0 )
		{
			fprintf(stderr,"[E] failed to read quality dif value\n");
			returncode = -1;
			goto cleanup;
		}
		
		if ( difsym == HUFFMAN_ESCAPE_CODE )
		{
			uint64_t v;
			
			if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
			{
				fprintf(stderr,"[E] failed to read quality dif value\n");
				returncode = -1;
				goto cleanup;
			}

			difsym = v;
		}

		if ( (difsym & 1) )
			dif = -(difsym ^ 1)/2;
		else
			dif =  (difsym/2) - 1;
			
		nextqual      = prevqual + dif;
		nextqualvalue = reverseQualityTable[nextqual];

		/* fprintf(stderr,"nextqualsym %d nextqual %d sym %c\n", (int)nextqual, (int)nextqualvalue, (char)(nextqualvalue+33)); */
		/* fprintf(stderr,"%c",(int)(nextqualvalue+33)); */
		D->Q[i] = nextqualvalue + PHREDSHIFT;
		
		prevqual = nextqual;
	}
	
	/* fprintf(stderr,"\n"); */
	
	D->Q_l = qlen;

	cleanup:
	
	return returncode;
}

int checkBinaryFile(char const * fn, ProvenanceStep ** insPS)
{
	FILE * in = NULL;
	int returncode = 0;
	QualityHuffman * QH = NULL;
	BitLevelDecoder * BLD = NULL;
	/* uint64_t v; */
	uint64_t ii;
	uint64_t reverseQualityTableSize;
	uint64_t * reverseQualityTable = NULL;
	uint64_t nr;
	uint64_t iii;
	uint64_t indexmod;
	uint64_t indexpos = 0;
	DecodeResult DF;
	DecodeResult DR;
	char const * cc = NULL;
	uint64_t HSLo;
	HeaderStatsLine * HSL = NULL;
	ProvenanceStep * PS = NULL;
	HuffmanCode * symCode = NULL;
	HuffmanCode * lengthsCode = NULL;

	memset(&DF,0,sizeof(DF));
	memset(&DR,0,sizeof(DR));
	
	in = fopen(fn,"rb");
	
	if ( ! in )
	{
		fprintf(stderr,"[E] failed to open file %s\n",fn);
		returncode = -1;
		goto cleanup;
	}
	
	/* read position of index */
	fseek(in,-(int)sizeof(uint64_t),SEEK_END);
	
	for ( iii = 0; iii < sizeof(uint64_t); ++iii )
	{
		unsigned char c;
		if ( fread(&c,1,1,in) != 1 )
		{
			fprintf(stderr,"[E] failed to read position of index %s\n",fn);
			returncode = -1;
			goto cleanup;
		}
		
		indexpos <<= 8;
		indexpos |= c;
	}
	
	#if 0
	fprintf(stderr,"index position at %lu\n", (unsigned long)indexpos);
	#endif
	
	fseek(in,0,SEEK_SET);
	
	if ( ! (BLD = BitLevelDecoder_allocate(in)) )
	{
		fprintf(stderr,"[E] failed to instantiate bit level decoder for %s\n",fn);
		returncode = -1;
		goto cleanup;
	}
	
	/* read file type */
	for ( cc = binfiletype; *cc; ++cc )
	{
		uint64_t v;	
		int c;

		if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		{
			fprintf(stderr,"[E] failed to read number of reads in file\n");
			returncode = -1;
			goto cleanup;
		}
		
		c = v;
		
		if ( c != *cc )
		{
			fprintf(stderr,"[E] file type mismatch\n");
			returncode = -1;
			goto cleanup;		
		}
	}

	/* read number of pairs in file */
	if ( BitLevelDecoder_decode(BLD,&nr,64) < 0 )
	{
		fprintf(stderr,"[E] failed to read number of reads in file\n");
		returncode = -1;
		goto cleanup;
	}

	/* read size of HeaderStatsLine table */
	if ( BitLevelDecoder_decode(BLD,&HSLo,64) < 0 )
	{
		fprintf(stderr,"[E] failed to read number of header objects in file\n");
		returncode = -1;
		goto cleanup;
	}
	
	/* allocate HeaderStatsLine table */
	HSL = (HeaderStatsLine *)malloc(HSLo * sizeof(HeaderStatsLine));
	
	if ( ! HSL )
	{
		fprintf(stderr,"[E] unable to read header objects\n");
		returncode = -1;
		goto cleanup;	
	}

	/* read HeaderStatsLine table */
	for ( ii = 0; ii < HSLo; ++ii )
		if ( HeaderStatsLine_decode(BLD,&(HSL[ii])) < 0 )
		{
			fprintf(stderr,"[E] unable to read header objects\n");
			returncode = -1;
			goto cleanup;				
		}

	/* decode provenance lines */
	if ( !(PS = ProvenanceStep_decode(BLD)) )
	{
		fprintf(stderr,"[E] unable to decode provenance\n");
		returncode = -1;
		goto cleanup;					
	}
	
	assert ( PS );
	
	{
		ProvenanceStep * PP = PS;
		assert ( PP );
		
		while ( PP->next )
			PP = PP->next;
			
		PP->next = *insPS;
		*insPS = NULL;
	}

	for ( ii = 0; ii < HSLo; ++ii )
		if ( HSL[ii].type == '#' && HSL[ii].subtype == '!' )
			HSL[ii].num += 1;

	/* read index mod */
	if ( BitLevelDecoder_decode(BLD,&indexmod,64) < 0 )
	{
		fprintf(stderr,"[E] failed to indexmod from file\n");
		returncode = -1;
		goto cleanup;
	}

	/* read huffman tables  */
	if ( ! (QH = QualityHuffman_decode(BLD)) )
	{
		fprintf(stderr,"[E] failed to decode Huffman tables in %s\n",fn);
		returncode = -1;
		goto cleanup;	
	}

	#if 0	
	fprintf(stderr,"[V] decoding\n");

	fprintf(stderr,"[V] first huffman table\n");	
	printCodeTable(stderr,QH->firstHuf->CT);
	fprintf(stderr,"[V] dif huffman table\n");	
	printCodeTable(stderr,QH->difHuf->CT);
	#endif
	
	/* read size of reverse quality table */
	if ( BitLevelDecoder_decodeGamma(BLD,&reverseQualityTableSize) < 0 )
	{
		fprintf(stderr,"[E] unable to decode size of reverse quality table from %s\n",fn);
		returncode = -1;
		goto cleanup;		
	}
	
	/* allocate memory for reverse quality table */
	if ( ! (reverseQualityTable = (uint64_t *)malloc(sizeof(uint64_t)*reverseQualityTableSize)) )
	{
		fprintf(stderr,"[E] unable to allocate memory for reverse quality table from %s\n",fn);
		returncode = -1;
		goto cleanup;		
	}
	
	/* decode reverse quality table */
	for ( ii = 0; ii < reverseQualityTableSize; ++ii )
		if ( BitLevelDecoder_decodeGamma(BLD,&reverseQualityTable[ii]) < 0 )
		{
			fprintf(stderr,"[E] unable to decode reverse quality table from %s\n",fn);
			returncode = -1;
			goto cleanup;		
		}
		
	if ( ! (symCode = HuffmanCode_decode(BLD)) )
	{
		fprintf(stderr,"[E] unable to decode sym huffman table from %s\n",fn);
		returncode = -1;
		goto cleanup;			
	}

	if ( ! (lengthsCode = HuffmanCode_decode(BLD)) )
	{
		fprintf(stderr,"[E] unable to decode sym huffman table from %s\n",fn);
		returncode = -1;
		goto cleanup;			
	}

	fprintf(stderr,"[V] sym huffman table\n");	
	printCodeTable(stderr,symCode->CT);
	fprintf(stderr,"[V] lengths huffman table\n");	
	printCodeTable(stderr,lengthsCode->CT);

	DF.S = DF.Q = NULL;
	DF.S_o = DF.Q_o = 0;
	DR.S = DR.Q = NULL;
	DR.S_o = DR.Q_o = 0;

	fprintf(stdout,"1 3 seq 1 0\n");
	fprintf(stdout,"2 3 irp\n");

	for ( ii = 0; ii < HSLo; ++ii )
	{
		fprintf(stdout,"%c %c %lu\n", HSL[ii].type, HSL[ii].subtype, (unsigned long)HSL[ii].num);
	}

	ProvenanceStep_print(stdout, PS);
	
	/* while ( BitLevelDecoder_decode(BLD,&v,8) == 0 && v == 'P' ) */
	for ( iii = 0; iii < nr; ++iii )
	{
		int64_t llv;
		if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'P' )
		{
			fprintf(stderr,"[E] unable to read P marker\n");
			returncode = -1;
			goto cleanup;		
		}
	
		if ( decodeSequenceAndQuality(BLD,QH,symCode,lengthsCode,reverseQualityTable,&DF) < 0 )
		{
			fprintf(stderr,"[E] unable to read forward data\n");
			returncode = -1;
			goto cleanup;		
		}

		if ( decodeSequenceAndQuality(BLD,QH,symCode,lengthsCode,reverseQualityTable,&DR) < 0 )
		{		
			fprintf(stderr,"[E] unable to read reverse data\n");
			returncode = -1;
			goto cleanup;		
		}
		
		fprintf(stdout,"P\n");

		fprintf(stdout,"S %lu ",(unsigned long)DF.S_l);
		fwrite(DF.S,DF.S_l,1,stdout);
		fprintf(stdout,"\n");

		fprintf(stdout,"Q %lu ",(unsigned long)DF.Q_l);
		fwrite(DF.Q,DF.Q_l,1,stdout);
		fprintf(stdout,"\n");

		fprintf(stdout,"S %lu ",(unsigned long)DR.S_l);
		fwrite(DR.S,DR.S_l,1,stdout);
		fprintf(stdout,"\n");

		fprintf(stdout,"Q %lu ",(unsigned long)DR.Q_l);
		fwrite(DR.Q,DR.Q_l,1,stdout);
		fprintf(stdout,"\n");

		#if 0
		uint64_t seqlen;
		uint64_t qlen;
		uint64_t i;
		
		if ( BitLevelDecoder_decode(BLD,&v,8) < 0 && v != 'S' )
		{
			fprintf(stderr,"[E] failed to find sequence after P marker\n");
			returncode = -1;
			goto cleanup;
		}

		if ( BitLevelDecoder_decodeGamma(BLD,&seqlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read sequence length after P marker\n");
			returncode = -1;
			goto cleanup;		
		}
		
		/* fprintf(stderr,"seqlen %lu\n", (unsigned long)seqlen); */
		
		for ( i = 0; i < seqlen; ++i )
		{
			uint64_t sym;
			if ( BitLevelDecoder_decode(BLD,&sym,2) < 0 )
			{
				fprintf(stderr,"[E] failed to decode symbol\n");
				returncode = -1;
				goto cleanup;
			}
			
			fprintf(stderr,"%d", (int)sym);
		}
		
		fprintf(stderr,"\n");

		if ( BitLevelDecoder_decode(BLD,&v,8) < 0 && v != 'Q' )
		{
			fprintf(stderr,"[E] failed to find Q marker after sequence\n");
			returncode = -1;
			goto cleanup;
		}

		if ( BitLevelDecoder_decodeGamma(BLD,&qlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read Q length after Q marker\n");
			returncode = -1;
			goto cleanup;
		}
		
		/* fprintf(stderr,"qlen %lu\n", (unsigned long)qlen); */
		
		int64_t const firstqualsym =
			HuffmanCode_decodeSymbol(QH->firstHuf, BLD);
			
		if ( firstqualsym < 0 )
		{
			fprintf(stderr,"[E] failed to read first quality value\n");
			returncode = -1;
			goto cleanup;		
		}
		
		int64_t const firstqual = reverseQualityTable[firstqualsym];
		int64_t prevqual = firstqualsym;

		/* fprintf(stderr,"firstqualsym %d firstqual %d\n", (int)firstqualsym, (int)firstqual); */
		fprintf(stderr,"%c",(char)(firstqual + PHREDSHIFT));
		
		for ( i = 1; i < qlen; ++i )
		{
			int64_t const difsym = HuffmanCode_decodeSymbol(QH->difHuf, BLD);
			int64_t dif;
			int64_t nextqual;
			int64_t nextqualvalue;

			if ( (difsym & 1) )
				dif = -(difsym ^ 1)/2;
			else
				dif =  (difsym/2) - 1;
				
			nextqual      = prevqual + dif;
			nextqualvalue = reverseQualityTable[nextqual];

			/* fprintf(stderr,"nextqualsym %d nextqual %d sym %c\n", (int)nextqual, (int)nextqualvalue, (char)(nextqualvalue+33)); */
			fprintf(stderr,"%c",(int)(nextqualvalue+PHREDSHIFT));
			
			prevqual = nextqual;
		}
		
		fprintf(stderr,"\n");
		#endif
	}

	cleanup:
	if ( in )
		fclose(in);
	QualityHuffman_deallocate(QH);
	BitLevelDecoder_deallocate(BLD);
	HuffmanCode_deallocate(symCode);
	HuffmanCode_deallocate(lengthsCode);
	free(reverseQualityTable);
	free(DF.S);
	free(DF.Q);
	free(DR.S);
	free(DR.Q);
	free(HSL);
	ProvenanceStep_deallocate(PS);
	
	return returncode;
}

typedef struct _Arguments
{
	char * progname;
	char ** posArgs;
	int numpos;
	char ** nonPosArgs;
	int numnonpos;
} Arguments;

void Arguments_deallocate(Arguments * A)
{
	if ( A )
	{
		free(A->posArgs);
		free(A->nonPosArgs);
		free(A);
	}
}

int argStringCompare(void const * A, void const * B)
{
	char const * CA = *((char const **)A);
	char const * CB = *((char const **)B);

	return strcmp(CA,CB);
}

Arguments * Arguments_parse(int const argc, char * argv[])
{
	Arguments * A = NULL;
	int i = 0;
	int parse;
	int numpos;
	int numnonpos;
	
	A = (Arguments *)malloc(sizeof(Arguments));
	
	if ( ! A )
		return NULL;
		
	memset(A,0,sizeof(Arguments));
	
	if ( argc )
		A->progname = argv[0];
	
	parse = 1;	
	numpos = 0;
	numnonpos = 0;
	for ( i = 1; i < argc; ++i )
	{
		if ( (! parse) || argv[i][0] != '-' )
		{
			numpos += 1;
		}
		else if ( argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 )
		{
			parse = 0;
		}
		else
		{
			numnonpos += 1;
		}
	}
	
	A->numpos = numpos;
	A->numnonpos = numnonpos;
	
	A->posArgs = (char **)malloc(numpos * sizeof(char *));
	A->nonPosArgs = (char **)malloc(numnonpos * sizeof(char *));
	
	parse = 1;
	numpos = 0;
	numnonpos = 0;

	for ( i = 1; i < argc; ++i )
	{
		if ( (! parse) || argv[i][0] != '-' )
		{
			A->posArgs[numpos++] = argv[i];
		}
		else if ( argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] == 0 )
		{
			parse = 0;
		}
		else
		{
			A->nonPosArgs[numnonpos++] = argv[i];
		}
	}

	/* sort non positional arguments */
	qsort(A->nonPosArgs,A->numnonpos,sizeof(char const *),argStringCompare);
	
	for ( i = 0; i < A->numnonpos; ++i )
	{
		fprintf(stderr,"nonpos[%d]=%s\n",i,A->nonPosArgs[i]);
	}
				
	return A;
}

int64_t Arguments_getNonPosInteger(Arguments const * A, char const * prefix, int64_t const def)
{
	int i;
	size_t const lprefix = strlen(prefix);
	
	for ( i = 0; i < A->numnonpos; ++i )
		if ( strncmp(prefix,A->nonPosArgs[i],lprefix) == 0 )
		{
			char const * c = A->nonPosArgs[i] + lprefix;
			int64_t n = 0;
			unsigned int ni = 0;
			
			while ( isdigit(*c) )
			{
				n *= 10;
				n += *c - '0';
				++c;
				++ni;
			}
			
			if ( *c == 0 && ni != 0 )
				return n;
				
			fprintf(stderr,"[E] ignoring unparsable argument %s\n", A->nonPosArgs[i]);
		}
		
	return def;
}

int Arguments_haveNonPosInteger(Arguments const * A, char const * prefix)
{
	return Arguments_getNonPosInteger(A,prefix,-1) != -1;
}

int main(int argc, char * argv[])
{
	char const * infn;
	char const * outfn;
	char const * command;
	int64_t maxstatlines = INT64_MAX;
	ProvenanceStep * PS = NULL;
	Arguments * A = NULL;
		
	if ( !(PS = ProvenanceStep_create(argc,argv,version)) )
	{
		fprintf(stderr,"[E] unable to create ProvenanceStep object\n");
		return EXIT_FAILURE;
	}
	
	/* ProvenanceStep_print(stderr,PS); */

	if ( !(A = Arguments_parse(argc,argv)) )
	{
		fprintf(stderr,"[E] unable to parse arguments\n");
		ProvenanceStep_deallocate(PS);
		return EXIT_FAILURE;
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
		ProvenanceStep_deallocate(PS);
		Arguments_deallocate(A);
		return EXIT_FAILURE;
	}
	
	command = A->posArgs[0];
	
	if ( strcmp(command,"a2b") == 0 )
	{
		if ( A->numpos < 3 )
		{
			fprintf(stderr,"usage: %s a2b <out> <in>\n", argv[0]);
			ProvenanceStep_deallocate(PS);
			Arguments_deallocate(A);
			return EXIT_FAILURE;
		}
		
		outfn = A->posArgs[1];
		infn  = A->posArgs[2];
		maxstatlines = Arguments_haveNonPosInteger(A,"--maxstatlines") ? Arguments_getNonPosInteger(A,"--maxstatlines",-1) : INT64_MAX;
		
		fprintf(stderr,"[V] output file name %s\n", outfn);
		fprintf(stderr,"[V] input file name %s\n", infn);
		fprintf(stderr,"[V] maxstatlines %ld\n", (long)maxstatlines);
		
		if ( produceBinaryFile(outfn,infn,maxstatlines,&PS) < 0 )
		{
			fprintf(stderr,"[E] failed to produce binary file\n");
			ProvenanceStep_deallocate(PS);
			Arguments_deallocate(A);
			return EXIT_FAILURE;
		}
	}
	else if ( strcmp(command,"b2a") == 0 )
	{
		if ( A->numpos < 2 )
		{
			fprintf(stderr,"usage: %s b2a <in>\n", argv[0]);
			ProvenanceStep_deallocate(PS);
			Arguments_deallocate(A);
			return EXIT_FAILURE;		
		}
		
		infn = A->posArgs[1];
		
		if ( checkBinaryFile(infn,&PS) < 0 )
		{
			fprintf(stderr,"[E] failed to check binary file\n");
			ProvenanceStep_deallocate(PS);
			Arguments_deallocate(A);
			return EXIT_FAILURE;
		}
	}
	else
	{
		fprintf(stderr,"[E] unknown command %s\n",command);
		ProvenanceStep_deallocate(PS);
		Arguments_deallocate(A);
		return EXIT_FAILURE;		
	}

	Arguments_deallocate(A);
	ProvenanceStep_deallocate(PS);
	
	return EXIT_SUCCESS;
}
