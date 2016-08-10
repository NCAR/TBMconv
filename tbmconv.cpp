
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
 * @file
 * @author Nicholas DeCicco <nsd.cicco@gmail.com>
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "gbytes.cpp"
#include "cdc.hpp"
#include "tbm.hpp"

#define OUT_FILE_NAME_LEN 100

int main(int argc, char **argv)
{
	SYSLBN_Data syslbn_data;
	SYSLBN_Text syslbn_text;
	FileHistoryWord_Data fhw_data;
	FileHistoryWord_Text fhw_text;
	FileControlPointer fcp;
	DataBufferFlags dbf;
	FILE *fp;                       /* Handle to the input/output files. */
	char *inFileName;               /* Name of the input file. */
	char outFileName[OUT_FILE_NAME_LEN]; /* Name of the output file. */
	char *outFileNameFormatStr;     /* */
	uint8_t *inBuf;                 /* */
	size_t fileSize;                /* */
	size_t offset;                  /* */
	uint8_t *decodeBuf = NULL;      /* */
	int first;                      /* */
	int i;                          /* */
	int numFiles = 0;               /* Number of files in the TBM archive. */
	size_t outFileNameFormatStrLen, newLen;
	const char fileIndexFormatStr[] = "%d";
	size_t writeOffset = 0;
	TBMFile *files;
	int filesWritten = 0;

	if (argc != 3) {
		fprintf(stderr, "Error: Require exactly two arguments.\n");
		printf("Usage:\n"
		       "\n"
		       "    tbmconv INFILE OUTFILE\n");
		return 1;
	}

	inFileName = argv[1];

	outFileNameFormatStrLen = strlen(argv[2]);
	if (!(outFileNameFormatStr = (char*)
	      malloc(sizeof(char) * (outFileNameFormatStrLen+1))))
	{
		goto mallocfail;
	}
	strcpy(outFileNameFormatStr, argv[2]);

	if (!(fp = fopen(inFileName, "r"))) {
		fprintf(stderr, "Error: Failed to open \"%s\" for reading.\n",
		        inFileName);
		return 1;
	}

	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (!(inBuf = (uint8_t*) malloc(sizeof(uint8_t)*fileSize))) {
		goto mallocfail;
	}

	if (fread(inBuf, sizeof(uint8_t), fileSize, fp) != fileSize) {
		fprintf(stderr, "Error: failed to read contents of \"%s\".\n",
		        inFileName);
		return 1;
	}

	read_syslbn(inBuf, &syslbn_text, &syslbn_data, 0);
	print_syslbn(&syslbn_text, &syslbn_data, 0);

	/* Sanity check the SYSLBN header. */
	assert(syslbn_data.vol1.vol1 == MAGIC_VOL1);
	assert(syslbn_data.hdr1.hdr1 == MAGIC_HDR1);
	assert(syslbn_data.hdr1.dataSetID_1_6 == MAGIC_NCARSY);
	assert(syslbn_data.hdr1.dataSetID_7_12 == MAGIC_STEMHD);
	assert(syslbn_data.hdr1.dataSetID_13_16 == MAGIC_1000);
	assert(syslbn_data.hdr1.dataSetID_17 == MAGIC_1);
	assert(syslbn_data.hdr2.hdr2 == MAGIC_HDR2);

	/* Sanity check the length of the file. */
	assert(fileSize == (size_t) (syslbn_data.numBKBlocks+1)*
	                            syslbn_data.bk*BK_BLOCK_SIZE_BYTES);

	/* The location of the first file control pointer is specified in the
	 * SYSLBN.
	 */
	offset = syslbn_data.firstFCPOff * 60;

	/* Read file control pointers. */
	do {
		read_fileControlPointer(inBuf, &fcp, offset);

		/* Each file control pointer is immediately followed by a set of file
		 * history words.
		 */
		read_fileHistoryWord(inBuf, &fhw_text, &fhw_data, offset+60);

		print_fileControlPtr(&fcp, offset, 0, 0);
		print_fileHistoryWord(&fhw_text, &fhw_data, offset+60);

		offset += fcp.nextFCPOff*60;

		if (!fcp.isEOF && fcp.dataBlkNum != syslbn_data.numBKBlocks-1) {
			numFiles++;
		}
	} while (!fcp.isEOF);

	if (numFiles > 1) {
		if (!strstr(outFileNameFormatStr, "%d")) {
			fprintf(stderr, "Info: \"%s\" is being appended to the output "
			                "file name format string as there are multiple "
			                "files in this TBM archive.\n", fileIndexFormatStr);
			newLen = outFileNameFormatStrLen + strlen(fileIndexFormatStr);
			if (!(outFileNameFormatStr = (char*) realloc(outFileNameFormatStr,
			                                             sizeof(char)*(newLen+1)))) {
				goto mallocfail;
			}
			strcpy(outFileNameFormatStr+outFileNameFormatStrLen, "%d");
		}
	}

	if (!(files = (TBMFile*) malloc(sizeof(TBMFile)*numFiles))) {
		goto mallocfail;
	}

	/* TODO: Sanity check that number of BK blocks adds up. */

	tbm_read(inBuf, syslbn_data.bk, files, numFiles);

	for (i = 0; i < numFiles; i++) {
		if (files[i].size == 0) {
			fprintf(stderr, "Info: file %d has zero size, skipping\n", i);
			continue;
		}

		snprintf(outFileName, OUT_FILE_NAME_LEN, outFileNameFormatStr, i);
		if (!(fp = fopen(outFileName, "w"))) {
			fprintf(stderr, "failed to open \"%s\" for writing\n",
			        outFileName);
			return 1;
		}
		printf("Info: writing to \"%s\"\n", outFileName);

		if (!(decodeBuf = (uint8_t*) realloc(decodeBuf,
		                                     DIV_CEIL(files[i].size,8))))
		{
			goto mallocfail;
		}
		memset(decodeBuf, 0, sizeof(uint8_t)*DIV_CEIL(files[i].size,8));

		first = 1;
		offset = files[i].offsetToDataStart;
		writeOffset = 0;
		do {
			read_dataBufferFlags(inBuf, &dbf, offset);
			offset += 60;
			gbytes<uint8_t,uint8_t>(inBuf+(offset/8),
			                        decodeBuf+(writeOffset/8),
			                        offset%8, 8, 0,
			                        DIV_CEIL((dbf.nextPtrOffset-1)*60,8));

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

		fwrite(decodeBuf, sizeof(uint8_t), DIV_CEIL(files[i].size, 8), fp);
		fclose(fp);

		filesWritten++;
	}

	printf("Info: Wrote %d files\n", filesWritten);

	free(inBuf);
	free(decodeBuf);
	free(outFileNameFormatStr);

	return 0;

mallocfail:
	fprintf(stderr, "Error: memory allocation failed\n");
	return 1;
}

