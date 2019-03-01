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
#include "VGPirpbin_QualityHuffman.h"
#include <stdlib.h>

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
