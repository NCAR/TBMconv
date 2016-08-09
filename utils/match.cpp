
/* Copyright (c) 2016, University Corporation for Atmospheric Research
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

/* Author: Nicholas DeCicco <nsd.cicco@gmail.com>
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "gbytes.cpp"

int read_data(uint8_t **buf, const char fn[]);

int main(int argc, char **argv)
{
	uint8_t *bufA, *bufB;
	int i = 0, j = 0, sizeA, sizeB, discrepancy, delta = 0;

	if (argc != 3) {
		fprintf(stderr, "Error: require exactly two arguments\n");
		return 1;
	}

	if (!(sizeA = read_data(&bufA, argv[1])) || !(sizeB = read_data(&bufB, argv[2]))) {
		return 1;
	}

	// search and find where the files deviate
	// note that we assume that B is the longer file
	do {
		discrepancy = 0;
		while (bufA[i] != bufB[j]) {
			discrepancy = 1;
			j++;
		}
		if (discrepancy) {
			delta = j - i - delta;
			printf("advance by %d at %d\n", delta, i);
		}
		i++;
		j++;
	} while (i < sizeA && j < sizeB);

	return 0;
}

int read_data(uint8_t **buf, const char fn[])
{
	FILE *fp;
	int readAmount;
	uint8_t *tmp;

	if (!(fp = fopen(fn, "r"))) {
		fprintf(stderr, "Error: failed to open \"%s\" for reading\n", fn);
		return 1;
	}

	fseek(fp, 0L, SEEK_END);
	readAmount = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (!(tmp = (uint8_t*) malloc(sizeof(uint8_t) * readAmount))) {
		goto mallocfail;
	}

	if (!(*buf = (uint8_t*) malloc(sizeof(uint8_t) * 2 * readAmount))) {
		goto mallocfail;
	}

	if (fread(tmp, sizeof(uint8_t), readAmount, fp) != readAmount) {
		goto readfail;
	}

	gbytes<uint8_t,uint8_t>(tmp, *buf, 0, 4, 0, 2*readAmount);

	fclose(fp);

	return 2*readAmount;

mallocfail:
	fprintf(stderr, "Error: memory allocation failed, aborting.\n");
	return 0;

readfail:
	fprintf(stderr, "Read error occured\n");
	return 0;
}
