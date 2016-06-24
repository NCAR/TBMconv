
/**
 * Author: Nicholas DeCicco <decicco@ucar.edu>
 *                          <nsd.cicco@gmail.com>
 */

#include <stdio.h>
#include <stdint.h>
#include "tbm.hpp"

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
"labelBufLen        = %7ld\n"
" ===== VOL1 ===== \n"
"vol1               = \"%.*s\" (0x%06lX)\n"
"volSerialName1     = \"%.*s\"\n"
"acc                = \"%c\" (%ld; %s)\n"
"acntNum            = \"%.*s\" (0x%06lX%06lX)\n"
"sciNum             = \"%.*s\" (0x%03lX)\n"
"tbmVolSerial       = \"%.*s\" (0x%lX)\n"
"sysLevelCode       = \"%c\" (0x%lX)\n"
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
"sysCode            = \"%.*s\"\n"
" ===== HDR2 ===== \n"
"hdr2               = \"%.*s\" (0x%06lX)\n"
"hdr2label          = \"%.*s\"\n"
" ==== SYSLBN ==== \n"
"fileCtrlPtrOff     = %5ld (\"%.*s\")\n"
"blkCtrlPtrOff      = %5ld (\"%.*s\")\n"
"firstFCPOff        = %5ld (\"%.*s\")\n"
"ctrlCardOpenOff    = %5ld (\"%.*s\")\n"
"openMergeAreaOff   = %5ld (\"%.*s\")\n"
"curCtrlCardOpenOff = %5ld (\"%.*s\")\n"
"fcpToBlkCtrlOff    = %5ld (\"%.*s\")\n",
data->machineType, data->machineType <= MACHINE_TYPE_MAX ? machineTypes[data->machineType].str : "--",
data->density, data->density <= DENSITY_MAX ? densities[data->density].str : "--",
data->dataType, data->dataType <= DATA_TYPE_MAX ? dataTypes[data->dataType].str : "--",
data->numTracks,
data->bk,
data->numBKBlocks,
data->labelBufLen,
// VOL1
4, text->vol1, data->vol1,
6, text->volSerialName1,
   text->vol1acc, data->acc, text->vol1acc == ' ' ? "unlimited access" : "special handling",
8, text->accountingNum, data->acntNum_1_3, data->acntNum_4_8,
2, text->sciNum, data->sciNum_1_4,
6, text->tbmVolSerial, data->tbmVolSerial,
text->sysLevelCode, data->sysLevelCode,
// HDR1
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
13, text->sysCode,
// HDR2
 4, text->hdr2, data->hdr2,
76, text->hdr2label,
// SYSLBN
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
fcp->fileType, fileTypes[fcp->fileType].str,
fcp->fileDisposition, fileDispositions[fcp->fileDisposition].str,
fcp->secondaryFileType, secondaryFileTypes[fcp->secondaryFileType].str,
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

void print_blockControlPointer(BlockControlPointer const*const bcp, const size_t offset)
{
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

void print_dataBufferFlags(DataBufferFlags const*const dbf, const size_t offset)
{
printf("=== Data Buffer Flags ===\n");
print_offset(offset);
printf(
"[raw data]                 = %015lX\n"
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

void print_offset(const size_t offset)
{
	printf("offset = %d (bits) %d+%d (8-bit bytes) %d+%d (60-bit words)\n",
	       (int) offset, (int) offset/8, (int) offset%8, (int) offset/60, (int) offset%60);
}
