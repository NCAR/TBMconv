
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
 * Author: Nicholas DeCicco <decicco@ucar.edu>
 *                          <nsd.cicco@gmail.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "gbytes.cpp"
#include "cdc.hpp"
#include "tbm.hpp"

/**
 * States for the tbm_read state machine.
 */
enum {
	kExpectVOL1,
	kExpectHDR1,
	kExpectHDR2,
	kExpectEOF1,
	kExpectEndLabelGroup,
	kExpectData,
	kExpectDBFAfterEOF1
};

/**
 *
 * @param inBuf
 * @param bk The block size (in multiples of 2048 60-bit words) specified in
 *        the SYSLBN block.
 * @param files A pointer to an array of `numFiles' TBMFiles structures.
 * @param numFiles The number of files contained in the TBM file.
 */
void tbm_read(uint8_t *const inBuf, const uint64_t bk, TBMFile *const files,
              int numFiles)
{
	size_t offset = bk * BK_BLOCK_SIZE_CDC_WORDS * 60;
	int first = 1;
	int i = 0;
	int fileComplete = 0;
	int next = kExpectVOL1;
	DataBufferFlags dbf;
	size_t writeOffset;
	VOL1_Text vol1_text;
	VOL1_Data vol1_data;

	do {
		read_dataBufferFlags(inBuf, &dbf, offset);

		if (dbf.isEOD) {
			/* Done reading the entire TBM archive. */
			assert(dbf.nextPtrOffset == 0);
			assert(dbf.prevPtrOffset == 1);
			assert(dbf.isRecordStart == 1);
			break;
		}

		/* Read data from the */
		/* File parsing state machine. */
		switch (next) {
			case kExpectVOL1:
				read_vol1(inBuf, &vol1_text,
				          &vol1_data, offset+60);
				assert(vol1_data.vol1 == MAGIC_VOL1);
				next = kExpectHDR1;
				break;
			case kExpectHDR1:
				read_hdr1(inBuf, &(files[i].hdr1_text),
				          &(files[i].hdr1_data), offset+60);
				assert(files[i].hdr1_data.hdr1 == MAGIC_HDR1);
				assert(files[i].hdr1_data.dataSetID_1_6 == MAGIC_NCARSY);
				assert(files[i].hdr1_data.dataSetID_7_12 == MAGIC_STEMHD);
				assert(files[i].hdr1_data.sysCode_1_10 ==
				       MAGIC_SYSCODE_1_10);
				assert(files[i].hdr1_data.sysCode_11_13 ==
				       MAGIC_SYSCODE_11_13);
				next = kExpectHDR2;

				break;
			case kExpectHDR2:
				read_hdr2(inBuf, &(files[i].hdr2_text),
				          &(files[i].hdr2_data), offset+60);
				assert(files[i].hdr2_data.hdr2 == MAGIC_HDR2);
				next = kExpectEndLabelGroup;
				break;
			/* End label group after header before start of data */
			case kExpectEndLabelGroup:
				assert(dbf.isEOF == 1);
				assert(dbf.nextPtrOffset == 1);
				assert(dbf.prevPtrOffset == 9);
				assert(dbf.endLabelGroup == 1);
				assert(dbf.isRecordStart == 1);
				assert(dbf.isEOD == 0);
				next = kExpectData;
				files[i].offsetToDataStart = offset+60;
				writeOffset = 0;
				break;
			case kExpectEOF1:
				assert(dbf.labelRecordFollows == 1);
				// TODO: look at dbf.blockCount
				read_hdr1(inBuf, &(files[i].eof1_text),
				          &(files[i].eof1_data), offset+60);
				assert(files[i].eof1_data.hdr1 == MAGIC_EOF1);
				assert(files[i].eof1_data.dataSetID_1_6 == MAGIC_NCARSY);
				assert(files[i].eof1_data.dataSetID_7_12 == MAGIC_STEMHD);
				assert(files[i].eof1_data.sysCode_1_10 ==
				       MAGIC_SYSCODE_1_10);
				assert(files[i].eof1_data.sysCode_11_13 ==
				       MAGIC_SYSCODE_11_13);
				next = kExpectDBFAfterEOF1;
				break;
			case kExpectDBFAfterEOF1:
				next = kExpectHDR1;
				assert(dbf.isEOF == 1);
				assert(dbf.endLabelGroup == 1);
				break;
			case kExpectData:
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

				/* Have we reached the data buffer flags that marks the
				 * end of the file? If so, there should be a DBF/EOF/DBF
				 * sequence which follows.
				 */
				if (dbf.isEOF && !dbf.endLabelGroup) {
					next = kExpectEOF1;
					files[i].size = writeOffset;
					i++;
					if (i == numFiles) {
						return;
					}
				}

				break;
		}
		offset += 60*dbf.nextPtrOffset;
	} while (!fileComplete);
}

void read_syslbn(uint8_t const*const inBuf, SYSLBN_Text *const text,
                 SYSLBN_Data *const data, const size_t offset)
{
	gbytes<uint8_t,uint8_t>(inBuf, (uint8_t*) text, 0, 6, 0,
	                        sizeof(SYSLBN_Text));
	cdc_decode((char*) text, sizeof(SYSLBN_Text));
	gbytes<uint8_t,uint64_t>(inBuf, (uint64_t*) data, 0, 60, 0,
	                         sizeof(SYSLBN_Data)/8);
}

void read_vol1(uint8_t const*const inBuf,
               VOL1_Text *const text,
               VOL1_Data *const data,
               const size_t offset)
{
	gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) data,
	                         offset%8, 60, 0,
	                         sizeof(VOL1_Data)/8);
	gbytes<uint8_t,uint8_t>(inBuf+(offset/8), (uint8_t*) text,
	                        offset%8, 6, 0,
	                         sizeof(VOL1_Text));
	cdc_decode((char*) text, sizeof(VOL1_Text));
}

void read_hdr1(uint8_t const*const inBuf,
               HDR1_Text *const text,
               HDR1_Data *const data,
               const size_t offset)
{
	gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) data,
	                         offset%8, 60, 0,
	                         sizeof(HDR1_Data)/8);
	gbytes<uint8_t,uint8_t>(inBuf+(offset/8), (uint8_t*) text,
	                        offset%8, 6, 0,
	                         sizeof(HDR1_Text));
	cdc_decode((char*) text, sizeof(HDR1_Text));
}

void read_hdr2(uint8_t const*const inBuf,
               HDR2_Text *const text,
               HDR2_Data *const data,
               const size_t offset)
{
	gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) data,
	                         offset%8, 60, 0,
	                         sizeof(HDR2_Data)/8);
	gbytes<uint8_t,uint8_t>(inBuf+(offset/8), (uint8_t*) text,
	                        offset%8, 6, 0,
	                         sizeof(HDR2_Text));
	cdc_decode((char*) text, sizeof(HDR2_Text));
}

void read_fileControlPointer(uint8_t const*const inBuf,
                             FileControlPointer *const fcp,
                             const size_t offset)
{
	gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) fcp, offset%8,
	                         60, 0, sizeof(FileControlPointer)/8);
}

void read_fileHistoryWord(uint8_t const*const inBuf,
                          FileHistoryWord_Text *const text,
                          FileHistoryWord_Data *const data,
                          const size_t offset)
{
	gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) data,
	                         offset%8, 60, 0,
	                         sizeof(FileHistoryWord_Data)/8);
	gbytes<uint8_t,uint8_t>(inBuf+(offset/8), (uint8_t*) text,
	                        offset%8, 6, 0,
	                         sizeof(FileHistoryWord_Text));
	cdc_decode((char*) text, sizeof(FileHistoryWord_Text));
}

void read_dataBufferFlags(uint8_t const*const inBuf,
                          DataBufferFlags *const dbf,
                          const size_t offset)
{
	gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) dbf,
	                         offset%8, 60, 0,
	                         sizeof(DataBufferFlags)/8);
}

void print_vol1(VOL1_Text const*const text, VOL1_Data const*const data,
                const size_t offset)
{
printf(
" ===== VOL1 ===== \n"
"vol1               = \"%.*s\" (0x%06lX)\n"
"volSerialName1     = \"%.*s\"\n"
"acc                = \"%c\" (%ld; %s)\n"
"acntNum            = \"%.*s\" (0x%06lX%06lX)\n"
"sciNum             = \"%.*s\" (0x%03lX)\n"
"tbmVolSerial       = \"%.*s\" (0x%lX)\n"
"sysLevelCode       = \"%c\" (0x%lX)\n",
4, text->vol1, data->vol1,
6, text->volSerialName1,
   text->vol1acc, data->acc, text->vol1acc == ' ' ? "unlimited access" : "special handling",
8, text->accountingNum, data->acntNum_1_3, data->acntNum_4_8,
2, text->sciNum, data->sciNum_1_4,
6, text->tbmVolSerial, data->tbmVolSerial,
text->sysLevelCode, data->sysLevelCode
);
}

void print_hdr1(HDR1_Text const*const text, HDR1_Data const*const data,
                const size_t offset)
{
printf(
" ===== HDR1 ===== \n"
"hdr1               = \"%.*s\" (0x%06lX)\n"
"dataSetID          = \"%.*s\"\n"
"volSerialName2     = \"%.*s\"\n"
"fileSecNum         = \"%.*s\"\n"
"fileSeqNum         = \"%.*s\"\n"
"generationNum      = \"%.*s\"\n"
"versionNum         = \"%.*s\"\n"
"creationDate       = \"%.*s\"\n"
"expDate            = \"%.*s\"\n"
"accChar            = \"%c\" (0x%02lX)\n"
"blockCount         = \"%.*s\"\n"
"sysCode            = \"%.*s\"\n",
 4, text->hdr1, data->hdr1,
17, text->dataSetID,
 6, text->volSerialName2,
 4, text->fileSectionNum,
 4, text->fileSequenceNum,
 4, text->generationNum,
 2, text->versionNum,
 6, text->creationDate,
 6, text->expirationDate,
    text->accessibilityChar, data->accChar,
 6, text->blockCount,
13, text->sysCode
);
}


void print_hdr2(HDR2_Text const*const text, HDR2_Data const*const data,
                const size_t offset)
{
printf(
" ===== HDR2 ===== \n"
"hdr2               = \"%.*s\" (0x%06lX)\n"
"hdr2label          = \"%.*s\"\n",
 4, text->hdr2, data->hdr2,
76, text->hdr2label
);
}

#define GET_REL_OFFSET(baseOff, baseMemb, memb) \
	baseOff + (( (char*) &(baseMemb->memb) - (char*) &(baseMemb) )/8)*60

void print_syslbn(SYSLBN_Text const*const text, SYSLBN_Data const*const data,
                  const size_t offset)
{
printf(" ==== SYSLBN ==== \n");
print_offset(offset);
printf(
"machineType        = %7ld (%s)\n"
"density            = %7ld (%s)\n"
"dataType           = %7ld (%s)\n"
"numTracks          = %7ld\n"
"bk                 = %7ld\n"
"numBKBlocks        = %7ld\n"
"labelBufLen        = %7ld\n",
data->machineType, data->machineType <= MACHINE_TYPE_MAX ? machineTypes[data->machineType].str : "--",
data->density, data->density <= DENSITY_MAX ? densities[data->density].str : "--",
data->dataType, data->dataType <= DATA_TYPE_MAX ? dataTypes[data->dataType].str : "--",
data->numTracks,
data->bk,
data->numBKBlocks,
data->labelBufLen
);

print_vol1(&(text->vol1), &(data->vol1), GET_REL_OFFSET(offset, data, vol1));
print_hdr1(&(text->hdr1), &(data->hdr1), GET_REL_OFFSET(offset, data, hdr1));
print_hdr2(&(text->hdr2), &(data->hdr2), GET_REL_OFFSET(offset, data, hdr2));

printf(
" ==== SYSLBN ==== \n"
"fileCtrlPtrOff     = %5ld (\"%.*s\")\n"
"blkCtrlPtrOff      = %5ld (\"%.*s\")\n"
"firstFCPOff        = %5ld (\"%.*s\")\n"
"ctrlCardOpenOff    = %5ld (\"%.*s\")\n"
"openMergeAreaOff   = %5ld (\"%.*s\")\n"
"curCtrlCardOpenOff = %5ld (\"%.*s\")\n"
"fcpToBlkCtrlOff    = %5ld (\"%.*s\")\n",
data->fileCtrlPtrOff,     5, text->fileCtrlPtrOff,   
data->blkCtrlPtrOff,      5, text->blkCtrlPtrOff,
data->firstFCPOff,        5, text->firstFCPOff,
data->ctrlCardOpenOff,    5, text->ctrlCardOpenOff,
data->openMergeAreaOff,   5, text->openMergeAreaOff,
data->curCtrlCardOpenOff, 5, text->curCtrlCardOpenOff,
data->fcpToBlkCtrlOff,    5, text->fcpToBlkCtrlOff
);
}

void print_fileControlPtr(FileControlPointer const*const fcp,
                          const size_t offset, const int printHorizontal,
                          const int printHeader)
{
if (printHorizontal) {
if (printHeader) {
printf(
"          | offset to | data blk | buffer ptr | file | file        | 2nd  | is        | is   \n"
"offset    | next FCP  | number   | offset     | type | disposition | type | obsolete? | EOF? \n"
"==========+===========+==========+============+======+=============+======+===========+======\n"
);
}
printf(
"%9ld | %9ld | %8ld | %10ld | %4ld | %11ld | %4ld | %9ld | %4ld\n",
offset/60,
fcp->nextFCPOff,
fcp->dataBlkNum,
fcp->bufferPtrOffset,
fcp->fileType,
fcp->fileDisposition,
fcp->secondaryFileType,
fcp->isObsolete,
fcp->isEOF
);
} else {
printf("=== File Control Pointer ===\n");
print_offset(offset);
printf(
"nextFCPOff        = %ld\n"
"dataBlkNum        = %ld\n"
"bufferPtrOffset   = %ld\n"
"fileType          = %ld (%s)\n"
"fileDisposition   = %ld (%s)\n"
"secondaryFileType = %ld (%s)\n"
"isObsolete        = %ld\n"
"isEOF             = %ld\n",
fcp->nextFCPOff,
fcp->dataBlkNum,
fcp->bufferPtrOffset,
fcp->fileType, fcp->fileType < FILE_TYPE_MAX ?
               fileTypes[fcp->fileType].str : "--",
fcp->fileDisposition, fcp->fileDisposition <= FILE_DISPOSITION_MAX ?
                      fileDispositions[fcp->fileDisposition].str : "--",
fcp->secondaryFileType, fcp->secondaryFileType <= SECONDARY_FILE_TYPE_MAX ?
                        secondaryFileTypes[fcp->secondaryFileType].str : "--",
fcp->isObsolete,
fcp->isEOF
);
}
}

void print_fileHistoryWord(FileHistoryWord_Text const*const fhw_text,
                           FileHistoryWord_Data const*const fhw_data,
                           const size_t offset)
{
printf("=== File History Word ===\n");
print_offset(offset);
printf(
"dataSetID      = \"%.*s\"\n"
"lastReadTime   = %ld\n"
"lastReadDay    = %ld\n"
"lastReadYear   = %ld\n"
"lastWriteTime  = %ld\n"
"lastWriteDay   = %ld\n"
"lastWriteYear  = %ld\n"
"useCount       = %ld (\"%.*s\")\n"
"versionNum     = \"%.*s\"\n"
"writePasswd    = \"%.*s\"\n"
"readPasswd     = \"%.*s\"\n"
"recordLen      = %ld (\"%.*s\")\n"
"maxRecordNum   = %ld (\"%.*s\")\n"
"creationYear   = \"%.*s\"\n"
"creationDay    = \"%.*s\"\n"
"expirationYear = \"%.*s\"\n"
"expirationDay  = \"%.*s\"\n",
17, fhw_text->dataSetID,
fhw_data->lastReadTime,
fhw_data->lastReadDay,
fhw_data->lastReadYear,
fhw_data->lastWriteTime,
fhw_data->lastWriteDay,
fhw_data->lastWriteYear,
fhw_data->useCount, 2, fhw_text->useCount,
2, fhw_text->versionNum,
5, fhw_text->writePasswd,
5, fhw_text->readPasswd,
fhw_data->recordLen, 5, fhw_text->recordLen,
fhw_data->maxRecordNum, 5, fhw_text->maxRecordNum,
2, fhw_text->creationYear,
3, fhw_text->creationDay,
2, fhw_text->expirationYear,
3, fhw_text->expirationDay
);
}

void print_blockControlPointer(BlockControlPointer const*const bcp,
                               const size_t offset, const int printHorizontal,
                               const int printHeader)
{
if (printHorizontal) {
if (printHeader) {
printf(
"          | no record   |          | last record in | words to first pointer\n"
"offset    | starts here | checksum | data block N   | in data block N       \n"
"==========+=============+==========+================+=======================\n"
);
}
printf(
"%9ld | %11ld | %8ld | %14ld | %22ld\n",
offset/60,
bcp->noRecordStartsHere,
bcp->checksum,
bcp->lastRecord,
bcp->wordsToFirstPtr
);
} else {
printf("=== Block Control Pointer ===\n");
print_offset(offset);
printf(
"wordsToFirstPtr    = %ld\n"
"lastRecord         = %ld\n"
"checksum           = %ld\n" 
"noRecordStartsHere = %ld\n",
bcp->wordsToFirstPtr,
bcp->lastRecord,
bcp->checksum,
bcp->noRecordStartsHere
);
}
}

void print_dataBufferFlags(DataBufferFlags const*const dbf,
                           const size_t offset, const int printHorizontal,
                           const int printHeader)
{
if (printHorizontal) {
if (printHeader) {
printf(
"          |next   |prev   |record|    |record |record |has   |end  |label  |is   |   |   |is    \n"
"          |pointer|pointer|data  |num |is     |not    |parity|label|record |load |is |is |record\n"
"    offset|offset |offset |mode  |bits|shorter|written|error |group|follows|point|EOF|EOD|start \n"
"==========+=======+=======+======+====+=======+=======+======+=====+=======+=====+===+===+======\n"
);
}
printf(
"%10ld|%7ld|%7ld|%6ld|%4ld|%7ld|%7ld|%6ld|%5ld|%7ld|%5ld|%3ld|%3ld|%6ld\n",
offset/60,
dbf->nextPtrOffset,
dbf->prevPtrOffset,
dbf->recordDataMode,
dbf->numBits,
dbf->recordIsShorter,
dbf->recordNotWritten,
dbf->sourceRecordHasParityError,
dbf->endLabelGroup,
dbf->labelRecordFollows,
dbf->isLoadPoint,
dbf->isEOF,
dbf->isEOD,
dbf->isRecordStart
);
} else {
printf("=== Data Buffer Flags ===\n");
print_offset(offset);
printf(
"[raw data]                 = 0x%015lX\n"
"nextPtrOffset              = %ld\n"
"prevPtrOffset              = %ld\n"
"recordDataMode             = %ld (%s)\n"
"numBits                    = %ld\n"
"recordIsShorter            = %ld\n"
"recordNotWritten           = %ld\n"
"sourceRecordHasParityError = %ld\n"
"endLabelGroup              = %ld\n"
"labelRecordFollows         = %ld\n"
"isLoadPoint                = %ld\n"
"isEOF                      = %ld\n"
"isEOD                      = %ld\n"
"isRecordStart              = %ld\n",
*((uint64_t*) dbf),
dbf->nextPtrOffset,
dbf->prevPtrOffset,
dbf->recordDataMode, dbf->recordDataMode <= DATA_TYPE_MAX ? dataTypes[dbf->recordDataMode].str : "--",
dbf->numBits,
dbf->recordIsShorter,
dbf->recordNotWritten,
dbf->sourceRecordHasParityError,
dbf->endLabelGroup,
dbf->labelRecordFollows,
dbf->isLoadPoint,
dbf->isEOF,
dbf->isEOD,
dbf->isRecordStart
);
}
}

void print_offset(const size_t offset)
{
	printf("offset = %d (bits) %d+%d (8-bit bytes) %d+%d (60-bit words)\n",
	       (int) offset, (int) offset/8, (int) offset%8, (int) offset/60, (int) offset%60);
}
