
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
 * @author: Nicholas DeCicco <nsd.cicco@gmail.com>
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

#define LINE_LENGTH 100

void print_bin(uint64_t bin, unsigned numDigits);
int prompt_yesno(const char prompt[]);

int main(int argc, char **argv)
{
	FILE *fp;
	char *s;
	char *inFileName;
	SYSLBN_Data syslbn_data;
	SYSLBN_Text syslbn_text;
	uint8_t *inBuf;
	size_t readAmount;
	BlockControlPointer bcp;
	FileHistoryWord_Data fhw_data;
	FileHistoryWord_Text fhw_text;
	FileControlPointer fcp;
	size_t decodeAmount;
	size_t offset;
	DataBufferFlags dbf;
	char *decodeBuf = NULL;
	char *responseText = NULL;
	size_t responseTextLen = 0;
	int responseValue;
	int i, j;
	int first;

	if (argc != 2) {
		fprintf(stderr, "Error: Require one argument.\n");
		printf("Usage:\n"
		       "\n"
		       "    tbmconv INFILE\n");
		exit(1);
	}

	inFileName = argv[1];

	if (!(fp = fopen(inFileName, "r"))) {
		fprintf(stderr, "Error: Failed to open \"%s\" for reading.\n", inFileName);
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	readAmount = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	decodeAmount = (readAmount*8)/6;

	if (!(inBuf = (uint8_t*) malloc(sizeof(uint8_t)*readAmount)) ||
	    !(decodeBuf = (char*) malloc(sizeof(char)*decodeAmount)))
	{
		fprintf(stderr, "Error: memory allocation failed\n");
		exit(1);
	}

	if (fread(inBuf, sizeof(uint8_t), readAmount, fp) != readAmount) {
		fprintf(stderr, "read fail\n");
		exit(1);
	}

	while (1) {
		fprintf(stderr, "Enter an offset or type `quit': ");
		getline(&responseText, &responseTextLen, stdin);
		if ((s = strchr(responseText, '\n'))) {
			*s = '\0';
		}
		if (!strncmp(responseText, "quit", responseTextLen)) {
			break;
		}
		offset = atoi(responseText);
		if (offset == 0) goto getDisplay;

getUnits:
		fprintf(stderr,
		        "Please select units from the list below:\n"
		        "\n"
		        "\t1) Bits\n"
		        "\n"
		        "\t2) 6-bit bytes\n"
		        "\t3) 60-bit words\n"
		        "\n"
		        "\t4) 8-bit bytes\n"
		        "\t5) 64-bit words\n"
				"\n"
		        "Units? [1-5] ");

		getline(&responseText, &responseTextLen, stdin);
		responseValue = atoi(responseText);

		switch (responseValue) {
			case 1: break;
			case 2: offset *= 6;  break;
			case 3: offset *= 60; break;
			case 4: offset *= 8;  break;
			case 5: offset *= 64; break;
			default:
				fprintf(stderr, "Invalid response!\n");
				goto getUnits;
				break;
		}

getDisplay:
		fprintf(stderr,
		        "How would you like to display the data?\n"
		        "\n"
		        "\t 1) 6-bit DPC\n"
		        "\t 2) Data Buffer Flags (aka ``Record Control Word\")\n"
		        "\t 3) Block Control Pointer\n"
		        "\t 4) File Control Pointer\n"
		        "\t 5) File History Word\n"
		        "\t 6) SYSLBN\n"
		        "\t 7) VOL1\n"
		        "\t 8) HDR1\n"
		        "\t 9) HDR2\n"
		        "\t10) 20-bit integers\n"
		        "\t11) 60-bit integers\n"
		        "\n"
		        "Enter a choice [1-11]: ");
				
		getline(&responseText, &responseTextLen, stdin);
		responseValue = atoi(responseText);

		switch (responseValue) {
			case 1: /* 6-bit DPC */
				while (1) {
					fprintf(stderr, "How many characters? [1-%ld] ", decodeAmount);
					getline(&responseText, &responseTextLen, stdin);
					responseValue = atoi(responseText);
					if ((size_t) responseValue > decodeAmount) {
						fprintf(stderr, "Value too large.");
					} else {
						break;
					}
				}
				gbytes<uint8_t,char>(inBuf+(offset/8), decodeBuf, offset%8,
				                     6, 0, responseValue);
				cdc_decode(decodeBuf, responseValue);
				for (i = 0; i < responseValue; i += LINE_LENGTH) {
					fwrite(decodeBuf+i, sizeof(char), LINE_LENGTH, stdout);
#if 0
					for (j = 0; j < LINE_LENGTH; j+=2) {
						fprintf(stdout, "%02X%02X ", out_buffer[i+j], out_buffer[i+j+1]);
					}
					for (j = 0; j < LINE_LENGTH; j++) {
						fputc(ascii[(int) out_buffer[i+j]], stdout);
					}
#endif
					fputc('\n', stdout);
				}
				break;
			case 2: /* Data Buffer Flag */
				responseValue = prompt_yesno("Follow Data Buffer Flags until EOF?");
				first = 1;
				do {
					gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &dbf,
					                         offset%8, 60, 0,
					                         sizeof(DataBufferFlags)/8);
					print_dataBufferFlags(&dbf, offset, responseValue, first);
					offset += dbf.nextPtrOffset*60;
					first = 0;
				} while (responseValue && !dbf.isEOF);
				break;
			case 3: /* Block Control Pointer */
				while (1) {
					fprintf(stderr, "How many BCPs? [1-%ld] ",
					        decodeAmount/sizeof(BlockControlPointer));
					getline(&responseText, &responseTextLen, stdin);
					responseValue = atoi(responseText);
					if ((size_t) responseValue > decodeAmount/sizeof(BlockControlPointer)) {
						fprintf(stderr, "Value too large.");
					} else {
						break;
					}
				}
				for (i = 0; i < responseValue; i++) {
					gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &bcp,
					                         offset%8, 60, 0,
					                         sizeof(BlockControlPointer)/8);
					print_blockControlPointer(&bcp, offset, responseValue, i == 0);
					offset += 60;
				}
				break;
			case 4: /* File Control Pointer */
				responseValue = prompt_yesno("Follow File Control Pointers until EOF?");
				first = 1;
				do {
					gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &fcp,
					                         offset%8, 60, 0,
					                         sizeof(FileControlPointer)/8);
					print_fileControlPtr(&fcp, offset, responseValue, first);
					offset += fcp.nextFCPOff*60;
					first = 0;
				} while (responseValue && !fcp.isEOF);
				break;
			case 5: /* File History Word */
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8),
				                         (uint64_t*) &fhw_data,
				                         offset%8, 60, 0,
				                         sizeof(FileHistoryWord_Data)/8);
				gbytes<uint8_t,uint8_t>(inBuf+(offset/8),
				                        (uint8_t*) &fhw_text,
				                        offset%8, 6, 0,
				                         sizeof(FileHistoryWord_Text));
				cdc_decode((char*) &fhw_text, sizeof(FileHistoryWord_Text));
				print_fileHistoryWord(&fhw_text, &fhw_data, offset);
				break;
			case 6: /* SYSLBN */
				gbytes<uint8_t,uint8_t>(inBuf+(offset/8),
				                        (uint8_t*) &syslbn_text, offset%8,
				                        6, 0, sizeof(SYSLBN_Text));
				cdc_decode((char*) &syslbn_text, sizeof(SYSLBN_Text));
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8),
				                         (uint64_t*) &syslbn_data, offset%8, 
				                         60, 0, sizeof(SYSLBN_Data)/8);
				print_syslbn(&syslbn_text, &syslbn_data, offset);
				break;
			case 7: /* VOL1 */
				gbytes<uint8_t,uint8_t>(inBuf+(offset/8),
				                        (uint8_t*) &(syslbn_text.vol1), offset%8,
				                        6, 0, sizeof(HDR1_Text));
				cdc_decode((char*) &(syslbn_text.vol1), sizeof(HDR1_Text));
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8),
				                         (uint64_t*) &(syslbn_data.vol1), offset%8,
				                         60, 0, sizeof(HDR1_Data)/8);
				print_vol1(&(syslbn_text.vol1), &(syslbn_data.vol1), offset);
				break;
			case 8: /* HDR1 */
				gbytes<uint8_t,uint8_t>(inBuf+(offset/8),
				                        (uint8_t*) &(syslbn_text.hdr1), offset%8,
				                        6, 0, sizeof(HDR1_Text));
				cdc_decode((char*) &(syslbn_text.hdr1), sizeof(HDR1_Text));
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8),
				                         (uint64_t*) &(syslbn_data.hdr1), offset%8,
				                         60, 0, sizeof(HDR1_Data)/8);
				print_hdr1(&(syslbn_text.hdr1), &(syslbn_data.hdr1), offset);
				break;
			case 9: /* HDR2 */
				gbytes<uint8_t,uint8_t>(inBuf+(offset/8),
				                        (uint8_t*) &(syslbn_text.hdr2), offset%8,
				                        6, 0, sizeof(HDR1_Text));
				cdc_decode((char*) &(syslbn_text.hdr2), sizeof(HDR1_Text));
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8),
				                         (uint64_t*) &(syslbn_data.hdr2), offset%8,
				                         60, 0, sizeof(HDR1_Data)/8);
				print_hdr2(&(syslbn_text.hdr2), &(syslbn_data.hdr2), offset);
				break;
			case 10: /* 20-bit integers */
				while (1) {
					fprintf(stderr, "How many values? [1-%ld] ",
					        (readAmount*8)/20);
					getline(&responseText, &responseTextLen, stdin);
					responseValue = atoi(responseText);
					if ((size_t) responseValue > (readAmount*8)/20) {
						fprintf(stderr, "Value too large.");
					} else {
						break;
					}
				}
				gbytes<uint8_t,uint32_t>(inBuf+(offset/8),
				                         (uint32_t*) decodeBuf, offset%8,
				                         20, 0, responseValue);
				for (i = 0; i < responseValue; i += 3) {
					for (j = 0; j < 3; j++) {
						fprintf(stdout, "%7d ", ((uint32_t*) decodeBuf)[i+j]);
					}
					fputc('\n', stdout);
				}
				break;
			case 11: /* 20-bit integers */
				while (1) {
					fprintf(stderr, "How many values? [1-%ld] ",
					        (readAmount*8)/20);
					getline(&responseText, &responseTextLen, stdin);
					responseValue = atoi(responseText);
					if ((size_t) responseValue > (readAmount*8)/60) {
						fprintf(stderr, "Value too large.");
					} else {
						break;
					}
				}
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8),
				                         (uint64_t*) decodeBuf, offset%8,
				                         60, 0, responseValue);
				for (i = 0; i < responseValue; i++) {
					fprintf(stdout, "%19ld ", ((uint64_t*) decodeBuf)[i]);
					print_bin(((uint64_t*) decodeBuf)[i], 60);
					fputc('\n', stdout);
				}
				break;
			default:
				fprintf(stderr, "Invalid selection.\n");
				goto getDisplay;
				break;
		}
	}

	free(responseText);
	free(inBuf);

	return 0;
}

void print_bin(uint64_t bin, unsigned numDigits)
{
	int i = numDigits;

	while (i > 0) {
		i--;
		fputc(((bin >> i) & 1) + '0', stdout);
	}
}

int prompt_yesno(const char prompt[])
{
	int responseValue;
	char *responseText = NULL;
	size_t responseTextLen = 0;
	char *s;

	while (1) {
		fprintf(stderr, "%s [yn] ", prompt);
		getline(&responseText, &responseTextLen, stdin);
		if ((s = strchr(responseText, '\n'))) {
			*s = '\0';
		}
		if (!strcmp(responseText, "n")) {
			responseValue = 0;
			break;
		} else if (!strcmp(responseText, "y")) {
			responseValue = 1;
			break;
		}
		// fall through
		fprintf(stderr, "Invalid response!\n");
	}

	free(responseText);

	return responseValue;
}
