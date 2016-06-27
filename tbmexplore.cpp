
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

#define LINE_LENGTH 100

int main(int argc, char **argv)
{
	FILE *fp;
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
//	char eofStr[] = "EOF1NCARSYSTEMHD";
//	char eofStrLen = strlen(eofStr);
//	uint8_t *eofStart, *decodeBuf;
//	uint8_t tmp[10000];
	char *decodeBuf = NULL;
	char *responseText = NULL;
	size_t responseTextLen = 0;
	int responseValue;
	int i;

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

//	assert(syslbn_data.vol1 == MAGIC_VOL1);
//	assert(syslbn_data.hdr1 == MAGIC_HDR1);
//	assert(syslbn_data.dataSetID_1_6 == MAGIC_NCARSY);
//	assert(syslbn_data.dataSetID_7_12 == MAGIC_STEMHD);
//	assert(syslbn_data.dataSetID_13_16 == MAGIC_1000);
//	assert(syslbn_data.dataSetID_17 == MAGIC_1);
//	assert(syslbn_data.hdr2 == MAGIC_HDR2);

	while (1) {
		fprintf(stderr, "Enter an offset: ");
		getline(&responseText, &responseTextLen, stdin);
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
		        "\t1) 6-bit DPC\n"
		        "\t2) Data Buffer Flags (aka ``Record Control Word\")\n"
		        "\t3) Block Control Pointer\n"
		        "\t4) File Control Pointer\n"
		        "\t5) File History Word\n"
		        "\t6) SYSLBN\n"
		        "\n"
		        "Enter a choice [1-6]: ");
				
		getline(&responseText, &responseTextLen, stdin);
		responseValue = atoi(responseText);

		switch (responseValue) {
			case 1: /* 6-bit DPC */
getNumChars:
				fprintf(stderr, "How many characters? [1-%ld] ", decodeAmount);
				getline(&responseText, &responseTextLen, stdin);
				responseValue = atoi(responseText);
				if ((size_t) responseValue > decodeAmount) {
					fprintf(stderr, "Value too large.");
					goto getNumChars;
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
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &dbf,
				                         offset%8, 60, 0,
				                         sizeof(DataBufferFlags)/8);
				print_dataBufferFlags(&dbf, offset);
				break;
			case 3: /* Block Control Pointer */
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &bcp,
				                         offset%8, 60, 0,
				                         sizeof(BlockControlPointer)/8);
				print_blockControlPointer(&bcp, offset);
				break;
			case 4: /* File Control Pointer */
				gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &fcp,
				                         offset%8, 60, 0,
				                         sizeof(FileControlPointer)/8);
				print_fileControlPtr(&fcp, offset);
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
			default:
				fprintf(stderr, "Invalid selection.\n");
				goto getDisplay;
				break;
		}
	}

	free(responseText);

#if 0
	#define DBF_START_BITS 984720
	offset = DBF_START_BITS;
	do {
		numDBF++;
		gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &dbf,
		                         offset%8, 60, 0,
		                         sizeof(DataBufferFlags)/8);
		print_dataBufferFlags(&dbf, offset);

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
	size_t writeOffset = 0;
	memset(decodeBuf, 0, sizeof(uint8_t)*(decodeAmount + 8*numDBF /* approximate */));
	do {
		gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &dbf,
		                         offset%8, 60, 0,
		                         sizeof(DataBufferFlags)/8);
		offset += 60;
		gbytes<uint8_t,uint8_t>(inBuf+(offset/8), tmp, offset%8, 8, 0, DIV_CEIL((dbf.nextPtrOffset-1)*60,8));
		memcpy(decodeBuf+(writeOffset/8), tmp, DIV_CEIL((dbf.nextPtrOffset-1)*60,8));
		writeOffset += 60*(dbf.nextPtrOffset-1);
		/* Align writeOffset to 64-bit boundaries */
		if (!dbf.isEOF && (writeOffset % 64) == 0) {
			writeOffset += 64;
		} else {
			writeOffset = 64*DIV_CEIL(writeOffset,64);
		}
		offset += 60*(dbf.nextPtrOffset-1);
	} while (!dbf.isEOF);
#endif

	free(inBuf);

	return 0;
}
