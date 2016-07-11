
/**
 * Author: Nicholas DeCicco <decicco@ucar.edu>
 *                          <nsd.cicco@gmail.com>
 */

#include <stdio.h>
#include <stdint.h>
#include "tbm.hpp"

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
                          const size_t offset)
{
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
