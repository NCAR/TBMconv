

/* Author: Nicholas DeCicco <decicco@ucar.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "gbytes.cpp"
#include "cdc.hpp"

#define DIV_CEIL(n,d) (((n)-1)/(d)+1)
#define DECOMP_SIZE(n) DIV_CEIL((n)*8,6)
#if 1
#define LINE_LENGTH 100
#else
#define LINE_LENGTH 12
#endif

void decode(uint8_t *in_buffer, char *out_buffer, size_t length);

int main(int argc, char **argv)
{
	FILE *fp;
	char *inFileName;
	size_t compSize, decompSize;
	uint8_t *in_buffer = NULL;
	uint64_t *out_buffer = NULL;
	size_t i;
	int offset;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, "Error: Require at least one, at most two arguments.\n");
		printf("Usage:\n"
		       "\n"
		       "    sixbit INFILE\n");
		exit(1);
	}

	inFileName = argv[1];
	if (argc == 3) {
		offset = atoi(argv[2]);
	} else {
		offset = 0;
	}

	if (!(fp = fopen(inFileName, "r"))) {
		fprintf(stderr, "Error: Failed to open \"%s\" for reading.\n", inFileName);
		exit(1);
	}

	// Compute the size of the file
	fseek(fp, 0L, SEEK_END);
	compSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (!(in_buffer = (uint8_t*) malloc(sizeof(uint8_t)*compSize))) {
		goto malloc_fail;
	}

	if (fread(in_buffer, sizeof(uint8_t), compSize, fp) != compSize) {
		fprintf(stderr, "Error: failed to read data.\n");
		free(in_buffer);
		return 0;
	}

	// Compute the decompressed size
	decompSize = 308;
#define BIT_OFFSET 3636

	if (!(out_buffer = (uint64_t*) malloc(sizeof(uint64_t)*decompSize))) {
		goto malloc_fail;
	}

	// Unpack 60-bit bk values into 64-bit words
//	gbytes<uint8_t,uint64_t>(in_buffer+(BIT_OFFSET/8), out_buffer, BIT_OFFSET%8, 6, 0, decompSize);
	gbytes<uint8_t,char>(in_buffer+(BIT_OFFSET/8), (char*)out_buffer, BIT_OFFSET%8, 6, 0, decompSize);
	cdc_decode((char*) out_buffer, decompSize);

	uint8_t tmp[10];

	// Dump the contents to stdout
	for (i = 0; i < decompSize; i++) {
//		gbytes<uint64_t,uint8_t>(out_buffer+i, tmp, 0, 6, 0, 10);
//		cdc_decode((char*) tmp, 10);
//		printf("%.*s %20ld first ptr = %ld last record = %ld\n", 10, tmp, out_buffer[i], out_buffer[i] & 0xFFFFFF, (out_buffer[i] >> 24) & 0x1FFFFF);
	}
	fwrite((char*) out_buffer, sizeof(char), decompSize*8, stdout);

	fclose(fp);
	free(in_buffer);
	free(out_buffer);

	return 0;

malloc_fail:
	fprintf(stderr, "Error: Memory allocation failed.\n");
	return 1;
}
