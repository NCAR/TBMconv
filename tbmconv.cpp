
/**
 * Copyright (c) 2016, University Corporation for Atmospheric Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Author: Nicholas DeCicco <nsd.cicco@gmail.com>
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "gbytes.cpp"
#include "cdc.hpp"
#include "tbm.hpp"

// Rounds up division.
#define DIV_CEIL(n,d) (((n)-1)/(d)+1)

#define BK_BLOCK_SIZE ((2048*60)/8)

int main(int argc, char **argv)
{
	FILE *fp;
	char *inFileName;
	char *outFileName;
	SYSLBN_Data syslbn_data;
	SYSLBN_Text syslbn_text;
	uint8_t *inBuf;
	size_t readAmount;
	FileHistoryWord_Data fhw_data;
	FileHistoryWord_Text fhw_text;
	FileControlPointer fcp;
	size_t offset, decodeAmount;
	DataBufferFlags dbf;
	char eofStr[] = "EOF1NCARSYSTEMHD";
	char eofStrLen = strlen(eofStr);
	uint8_t *eofStart, *decodeBuf;
	uint8_t tmp[10000];
	int first;

	if (argc != 3) {
		fprintf(stderr, "Error: Require exactly two arguments.\n");
		printf("Usage:\n"
		       "\n"
		       "    tbmconv INFILE OUTFILE\n");
		exit(1);
	}

	inFileName = argv[1];
	outFileName = argv[2];

	if (!(fp = fopen(inFileName, "r"))) {
		fprintf(stderr, "Error: Failed to open \"%s\" for reading.\n", inFileName);
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	readAmount = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (!(inBuf = (uint8_t*) malloc(sizeof(uint8_t)*readAmount))) {
		fprintf(stderr, "Error: \n");
		exit(1);
	}

	if (fread(inBuf, sizeof(uint8_t), readAmount, fp) != readAmount) {
		fprintf(stderr, "read fail\n");
		exit(1);
	}

	gbytes<uint8_t,uint8_t>(inBuf, (uint8_t*) &syslbn_text, 0, 6, 0, sizeof(SYSLBN_Text));

	cdc_decode((char*) &syslbn_text, sizeof(SYSLBN_Text));

	gbytes<uint8_t,uint64_t>(inBuf, (uint64_t*) &syslbn_data, 0, 60, 0, sizeof(SYSLBN_Data)/8);

	print_syslbn(&syslbn_text, &syslbn_data, 0);

	assert(syslbn_data.vol1.vol1 == MAGIC_VOL1);
	assert(syslbn_data.hdr1.hdr1 == MAGIC_HDR1);
	assert(syslbn_data.hdr1.dataSetID_1_6 == MAGIC_NCARSY);
	assert(syslbn_data.hdr1.dataSetID_7_12 == MAGIC_STEMHD);
	assert(syslbn_data.hdr1.dataSetID_13_16 == MAGIC_1000);
	assert(syslbn_data.hdr1.dataSetID_17 == MAGIC_1);
	assert(syslbn_data.hdr2.hdr2 == MAGIC_HDR2);
	assert(readAmount == (size_t) (syslbn_data.numBKBlocks+1)*
	                     syslbn_data.bk*BK_BLOCK_SIZE);

	/* The location of the first file control pointer is specified in the
	 * SYSLBN.
	 */

	offset = syslbn_data.firstFCPOff * 60;

	first = 1;
	do {
		gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &fcp, offset%8,
		                         60, 0, sizeof(FileControlPointer)/8);
		print_fileControlPtr(&fcp, offset);

		if (first) {
			assert(fcp.nextFCPOff - 9 == syslbn_data.numBKBlocks);
			first = 0;
		}

		// file history word always follows the FCP?
		gbytes<uint8_t,uint64_t>(inBuf+((offset+60)/8), (uint64_t*) &fhw_data,
		                         (offset+60)%8, 60, 0,
		                         sizeof(FileHistoryWord_Data)/8);
		gbytes<uint8_t,uint8_t>(inBuf+((offset+60)/8), (uint8_t*) &fhw_text,
		                        (offset+60)%8, 6, 0,
		                         sizeof(FileHistoryWord_Text));
		cdc_decode((char*) &fhw_text, sizeof(FileHistoryWord_Text));

		print_fileHistoryWord(&fhw_text, &fhw_data, offset+60);

		offset += fcp.nextFCPOff*60;
	} while (!fcp.isEOF);


	int numDBF = 0;

	printf("=== After header ===\n");
	#define DBF_START_BITS 984720
	offset = DBF_START_BITS;
	do {
		numDBF++;
		gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &dbf,
		                         offset%8, 60, 0,
		                         sizeof(DataBufferFlags)/8);
		print_dataBufferFlags(&dbf, offset, 0, 0);

		/* Advance to the next pointer. */
		offset += 60*dbf.nextPtrOffset;
	} while (!dbf.isEOF);

// Look for the EOF marker
	decodeAmount = (readAmount*8)/6;
	if (!(decodeBuf = (uint8_t*) malloc(sizeof(uint8_t)*(decodeAmount + 8*numDBF /* approximate */)))) {
		return 1;
	}
	gbytes<uint8_t,uint8_t>(inBuf, decodeBuf, 0, 6, 0, decodeAmount);
	cdc_decode((char*) decodeBuf, decodeAmount);
	if (!(eofStart = (uint8_t*) memmem(decodeBuf, decodeAmount, eofStr, eofStrLen))) {
		fprintf(stderr, "Failed to locate EOF marker\n");
		return 1;
	}

#define DATA_START_BIT_OFFSET (16412*60) /* + 0.5 */
	readAmount = DIV_CEIL((eofStart-decodeBuf)*6 - DATA_START_BIT_OFFSET - 12 /* 12=2*6; EOF starts slightly sooner */,8);
	if (!(fp = fopen(outFileName, "w"))) {
		fprintf(stderr, "failed to open \"%s\" for writing\n", outFileName);
		return 1;
	}
	offset = DATA_START_BIT_OFFSET;
	size_t writeOffset = 0;
	memset(decodeBuf, 0, sizeof(uint8_t)*(decodeAmount + 8*numDBF /* approximate */));
	first = 1;
	do {
		gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &dbf,
		                         offset%8, 60, 0,
		                         sizeof(DataBufferFlags)/8);
		offset += 60;
		gbytes<uint8_t,uint8_t>(inBuf+(offset/8), tmp, offset%8, 8, 0, DIV_CEIL((dbf.nextPtrOffset-1)*60,8));
		memcpy(decodeBuf+(writeOffset/8), tmp, DIV_CEIL((dbf.nextPtrOffset-1)*60,8));
		writeOffset += 60*(dbf.nextPtrOffset-1);
		/* Align writeOffset to 64-bit boundaries, but not immediately after
		 * the GENPRO-I header.
		 */
		if (!first && !dbf.isEOF && (writeOffset % 64) == 0) {
			writeOffset += 64;
		} else {
			writeOffset = 64*DIV_CEIL(writeOffset,64);
		}
		first = 0;
		offset += 60*(dbf.nextPtrOffset-1);
	} while (!dbf.isEOF);
	fwrite(decodeBuf, sizeof(uint8_t), DIV_CEIL(writeOffset,8), fp);
	fclose(fp);

	free(inBuf);

	return 0;
}
