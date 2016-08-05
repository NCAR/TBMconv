
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

#ifndef TBM_H
#define TBM_H

// Rounds up division.
#define DIV_CEIL(n,d) (((n)-1)/(d)+1)

#define BK_BLOCK_SIZE_CDC_WORDS 2048
#define BK_BLOCK_SIZE_BYTES ((BK_BLOCK_SIZE_CDC_WORDS*60)/8)

typedef struct {
	char *str;
} Label;

/* SYSLBN word 0, bits 59-56 */
#define MACHINE_TYPE_CDC_7600  0
#define MACHINE_TYPE_CRAY_1    1
#define MACHINE_TYPE_FRONT_END 2
#define MACHINE_TYPE_MAX 2
const Label machineTypes[] = {
	{ (char*) "CDC 7600" },
	{ (char*) "Cray-1" },
	{ (char*) "Front end" }
};

/* SYSLBN word 0, bits 55-52 */
#define DENSITY_200_BPI  0
#define DENSITY_556_BPI  1
#define DENSITY_800_BPI  2
#define DENSITY_1600_BPI 3
#define DENSITY_MAX 3
const Label densities[] = {
	{ (char*) "200 BPI" },
	{ (char*) "556 BPI" },
	{ (char*) "800 BPI" },
	{ (char*) "1600 BPI" }
};

/* SYSLBN word 0, bits 51-44 */
#define DATA_TYPE_BCD_AS_DPC        0 /** "BCD as DPC" */
#define DATA_TYPE_BINARY_BIT_SERIAL 1 /** "Binary bit-serial" */
#define DATA_TYPE_BCD_NO_CONV       2 /** "BCD, no conversion from channel stage-in */
#define DATA_TYPE_ASCII             3 /** "ASCII" */
#define DATA_TYPE_EBCDIC            4 /** "EBCDIC" */
// These are from the tbmconv source code (these are not listed in the
// NCAR technical note)
#define DATA_TYPE_BINARY_INTEGER    5 /**  */
#define DATA_TYPE_FLOATING_POINT    6 /**  */
#define DATA_TYPE_DPC_CARD_IMAGE    7 /**  */
#define DATA_TYPE_TRANSPARENT       8 /**  */
#define DATA_TYPE_MAX 8
const Label dataTypes[] = {
	{ (char*) "BCD as DPC" },
	{ (char*) "Binary bit-serial" },
	{ (char*) "BCD, no conversion from channel stage-in" },
	{ (char*) "ASCII" },
	{ (char*) "EBCDIC" },
// These are from the tbmconv source code (these are not listed in the
// NCAR technical note)
	{ (char*) "binary integer" },
	{ (char*) "floating-point" },
	{ (char*) "dpc card image" },
	{ (char*) "transparent" }
};

/* Magic numbers for SYSLBN */
#define MAGIC_HDR1        0x20449C
#define MAGIC_HDR2        0x20449D
#define MAGIC_VOL1        0x58F31C
#define MAGIC_EOF1        005170634 /* octal */
#define MAGIC_NCARSY      0x3830524D9
#define MAGIC_STEMHD      0x4D414D204
#define MAGIC_1000        0x71B6DB
#define MAGIC_1           0x1C
/* Note: The leading zero on these literals means that these are octal numbers. */
#define MAGIC_SYSCODE_1_10  016030122555555233123 // "NCAR   SYS"
#define MAGIC_SYSCODE_11_13 0240515               // "TEM"

/* File control pointer bits 57-55 */
#define SECONDARY_FILE_TYPE_OLD     1
#define SECONDARY_FILE_TYPE_NEW     2
#define SECONDARY_FILE_TYPE_SCRATCH 4
#define SECONDARY_FILE_TYPE_MAX 4
const Label secondaryFileTypes[] = {
	{ (char*) "--" },
	{ (char*) "old" },
	{ (char*) "new" },
	{ (char*) "--" },
	{ (char*) "scratch" }
};

/* File control pointer bits 54-52 */
#define FILE_DISPOSITION_KEEP                  0
#define FILE_DISPOSITION_DELETE_AT_CLOSE       1
#define FILE_DISPOSITION_DELETE_AT_TERMINATION 2
#define FILE_DISPOSITION_MAX 2
const Label fileDispositions[] = {
	{ (char*) "keep" },
	{ (char*) "delete at close" },
	{ (char*) "delete at termination" }
};

/* File control pointer bits 51-49 */
#define FILE_TYPE_UNDEFINED         0
#define FILE_TYPE_SEQUENTIAL_ACCESS 1
#define FILE_TYPE_DIRECT_ACCESS     2
#define FILE_TYPE_MIXED_ACCESS      3
#define FILE_TYPE_MAX 3
const Label fileTypes[] = {
	{ (char*) "undefined" },
	{ (char*) "sequential access" },
	{ (char*) "direct access" },
	{ (char*) "mixed access" }
};

typedef struct {
/* Word  4 */ uint64_t volSerialName1     : 36; /** Volume "serial name" */
              uint64_t vol1               : 24; /** "VOL1" -> 0x58F31C */
              uint64_t /* padding */      :  0;

/* Word  5 */ uint64_t /* blank */        : 54;
              uint64_t acc                :  6; /** Volume 1 label accessibility criteria. From
                                                    Chapter 4 of the "NCAR Terabit Memory System" TN:
                                                    "A label character which indicates any
                                                    restrictions on who may have access to the
                                                    information in the volume. A space means
                                                    unlimited access; any other character means
                                                    special handling, in a manner to be defined by
                                                    the user." */
              uint64_t /* padding */      :  0;

/* Word  6 */ uint64_t /* blank */        : 64;

/* Word  7 */ uint64_t acntNum_1_3        : 18; /** Accounting number characters 1-3 */
              uint64_t /* blank */        : 42;
              uint64_t /* padding */      :  0;

/* Word  8 */ uint64_t /* blank */        : 18;
              uint64_t sciNum_1_4         : 12; /** "The scientist number from the *JOB [sic] card" */
              uint64_t acntNum_4_8        : 30; /** Accounting number characters 4-8 */
              uint64_t /* padding */      :  0;

/* Word  9 */ uint64_t /* blank */        : 64;

/* Word 10 */ uint64_t /* blank */        : 64;

/* Word 11 */ uint64_t sysLevelCode       :  6; /** "System level code" */
              uint64_t /* blank */        : 18;
              uint64_t tbmVolSerial       : 36; /* "TBM Volume Serial Name" */
              uint64_t /* padding */      :  0;
} VOL1_Data;

typedef struct {
/* Word 12 */ uint64_t dataSetID_1_6      : 36; /** Data Set Identifier characters 1 through 6 ("NCARSY" => 0x3830524D9) */
              uint64_t hdr1               : 24; /** "HDR1" (0x20449C) */
              uint64_t /* padding */      :  0;

/* Word 13 */ uint64_t dataSetID_13_16    : 24; /** Data Set Identifier characters 13 through 16 ("1000" => 0x71B6DB) */
              uint64_t dataSetID_7_12     : 36; /** Data Set Identifier characters 7 through 12 ("STEMHD" => 0x4D414D204) */
              uint64_t /* padding */      :  0;

/* Word 14 */ uint64_t fileSecNum_1_3     : 18; /** File section number characters 1-3 */
              uint64_t volSerialName2     : 36; /** Duplicate copy of volSerialName1 */
              uint64_t dataSetID_17       :  6; /** Data Set Identifier character 17 ("1" => 0x1C) */
              uint64_t /* padding */      :  0;

/* Word 15 */ uint64_t versionNum_1       :  6; /** Version number character 1 */
              uint64_t generationNum      : 24; /** Generation number characters (4) */
              uint64_t fileSeqNum         : 24; /** File sequence number characters (4) */
              uint64_t fileSecNum_4       :  6; /** Fie section number character 4 */
              uint64_t /* padding */      :  0;

/* Word 16 */ uint64_t expDate_1_3        : 18; /** Expiration date characters 1-3 */
              uint64_t creationDate       : 36; /** Creation date (alphanumeric) */
              uint64_t versionNum_2       :  6; /** Version number character 2 */
              uint64_t /* padding */      :  0;

/* Word 17 */ uint64_t blockCount         : 36; /** Block count (alphanumeric) */
              uint64_t accChar            :  6; /** The "HDR1 accessibility criteria character */
              uint64_t expDate_4_6        : 18; /** Expiration date characters 4-6 */
              uint64_t /* padding */      :  0;

/* Word 18 */ uint64_t sysCode_1_10       : 60; /** System code characters 1-10 */
              uint64_t /* padding */      :  0;

/* Word 19 */ uint64_t /* blank */        : 42;
              uint64_t sysCode_11_13      : 18; /** System code characters 11-13 */
              uint64_t /* padding */      :  0;
} HDR1_Data;

typedef struct {
/* Word 20 */ uint64_t hdr2_1_6           : 36; /** "Space for the HDR2 label" */
              uint64_t hdr2               : 24; /** "HDR2" (0x20449D) */
              uint64_t /* padding */      :  0;

/* Word 21 */ uint64_t hdr2_7_16          : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;

/* Word 22 */ uint64_t hdr2_17_26         : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;

/* Word 23 */ uint64_t hdr2_27_36         : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;

/* Word 24 */ uint64_t hdr2_37_46         : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;

/* Word 25 */ uint64_t hdr2_47_56         : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;

/* Word 26 */ uint64_t hdr2_57_66         : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;

/* Word 27 */ uint64_t hdr2_67_76         : 60; /** "Space for the HDR2 label" */
              uint64_t /* padding */      :  0;
} HDR2_Data;

/**
 * SYSLBN.
 */
typedef struct {
/* Word  0 */ uint64_t labelBufLen        : 20; /** Actual length of the SYSLBN label buffer */
              uint64_t numBKBlocks        : 12; /** Number of "BK length Data Blocks" in this volume  */
              uint64_t bk                 :  8; /** "The value of BK from a TLIB card on the 7600".
                                                    This is described in Table 8-4 on pp. 8.29 (PDF
                                                    pp. 189) of NCAR Tech Note IA-106 as "size
                                                    of blocked information on disk in units of
                                                    10240 words." */
              uint64_t numTracks          :  4; /** The original number of tape tracks */
              uint64_t dataType           :  8; /** The data type */
              uint64_t density            :  4; /** The original density */
              uint64_t machineType        :  4; /** The machine type */

              uint64_t /* padding */      :  0;

/* Word  1 */ uint64_t /* reserved */     : 64;

/* Word  2 */ uint64_t /* "open" */       : 60;
              uint64_t /* padding */      :  0;

/* Word  3 */ uint64_t /* "open" */       : 60;
              uint64_t /* padding */      :  0;

/* Words  4-11 */ VOL1_Data vol1;
/* Words 12-19 */ HDR1_Data hdr1;
/* Words 20-27 */ HDR2_Data hdr2;

/* Word 28 */ uint64_t blkCtrlPtrOff      : 30; /** Offset to current block control pointer from SYSMSI */
              uint64_t fileCtrlPtrOff     : 30; /** Offset to current file control pointer */
              uint64_t /* padding */      :  0;

/* Word 29 */ uint64_t ctrlCardOpenOff    : 30; /** Offset to control card "open" area */
              uint64_t firstFCPOff        : 30; /** Offset to first file control pointer */
              uint64_t /* padding */      :  0;

/* Word 30 */ uint64_t curCtrlCardOpenOff : 30; /** Offset to current open control card area */
              uint64_t openMergeAreaOff   : 30; /** Offset to "open" merge area */
              uint64_t /* padding */      :  0;

/* Word 31 */ uint64_t fcpToBlkCtrlOff    : 30; /** Offset from file control pointer to first block control pointer */
              uint64_t /* unused */       : 30;
              uint64_t /* padding */      :  0;
} SYSLBN_Data;

typedef struct {
/* Word      4 */ char vol1[4];               /** Should read "VOL1" */
/* Word      4 */ char volSerialName1[6];     /** Volume serial name */
/* Word      5 */ char vol1acc;               /** Volume 1 label accessibility criteria. From
                                                  Chapter 4 of the "NCAR Terabit Memory System" TN:
                                                  "A label character which indicates any
                                                  restrictions on who may have access to the
                                                  information in the volume. A space means
                                                  unlimited access; any other character means
                                                  special handling, in a manner to be defined by
                                                  the user." */
/* Words  5- 7 */ char padding2[26];
/* Words  7- 8 */ char accountingNum[8];      /** Accounting number */
/* Word      8 */ char sciNum[2];             /** "The scientist number from the *JOB [sic] card" */
/* Words  9-10 */ char padding3[23];
/* Word     11 */ char tbmVolSerial[6];       /** TBM volume serial name */
/* Word     11 */ char padding4[3];
/* Word     11 */ char sysLevelCode;          /** System level code */
} VOL1_Text;

typedef struct {
/* Word     12 */ char hdr1[4];               /** Should read "HDR1" */
/* Words 12-14 */ char dataSetID[17];         /** Data set identifier (should read
                                                  "NCARSYSTEMHD10001") */
/* Word     14 */ char volSerialName2[6];     /** Duplicate copy of volSerialName1 */
/* Words 14-15 */ char fileSectionNum[4];     /** File section number */
/* Word     15 */ char fileSequenceNum[4];    /** File sequence number */
/* Word     15 */ char generationNum[4];      /** Generation number */
/* Words 15-16 */ char versionNum[2];         /** Version number */
/* Word     16 */ char creationDate[6];       /** Creation date */
/* Words 16-17 */ char expirationDate[6];     /** Expiration date */
/* Word     17 */ char accessibilityChar;     /** HDR1 accessiblity criteria character */
/* Word     17 */ char blockCount[6];         /** Block count, aka "record count" (as in Ch. 4 of
                                                  NCAR TN-124+IA). According to Ch. 4, it is "set
                                                  to zeros by the system." */
/* Word     18 */ char sysCode[13];           /** System code */
/* Word     19 */ char padding5[7];
} HDR1_Text;

typedef struct {
/* Word     20 */ char hdr2[4];               /** Should read "HDR2" */
/* Words 20-27 */ char hdr2label[76];         /** Header 2 label */
} HDR2_Text;

typedef struct __attribute__((packed)) {
/* Word   0- 3 */ char padding1[40];
// ==================== VOL1 ====================
VOL1_Text vol1;
// ==================== HDR1 ====================
HDR1_Text hdr1;
// ==================== HDR2 ====================
HDR2_Text hdr2;
// =================== SYSLBN ===================
/* Word     28 */ char fileCtrlPtrOff[5];     /** Offset to current file control pointer */
/* Word     28 */ char blkCtrlPtrOff[5];      /** Offset to current block control pointer from SYSMSI */
/* Word     29 */ char firstFCPOff[5];        /** Offset to first file control pointer */
/* Word     29 */ char ctrlCardOpenOff[5];    /** Offset to control card "open" area */
/* Word     30 */ char openMergeAreaOff[5];   /** Offset to "open" merge area */
/* Word     30 */ char curCtrlCardOpenOff[5]; /** Offset to current open control card area */
/* Word     31 */ char padding6[5];
/* Word     31 */ char fcpToBlkCtrlOff[5];    /** Offset from file control pointer to first block control pointer */
} SYSLBN_Text;

/** File Control Pointer ("FCP") */
typedef struct {
/* Word  0 */ uint64_t nextFCPOff          : 12; /** "[Number of?] words to next file control pointer". */
              uint64_t dataBlkNum          : 12; /** "Data block number where the file starts". */
              uint64_t bufferPtrOffset     : 21; /* "Location of buffer pointer prceding the HDR1 label for this file" */
              uint64_t /* "open" */        :  4;
              uint64_t fileType            :  3; /** One of FILE_TYPE_UNDEFINED, FILE_TYPE_SEQUENTIAL_ACCESS, FILE_TYPE_DIRECT_ACCESS, or FILE_TYPE_MIXED_ACCESS. */
              uint64_t fileDisposition     :  3;
              uint64_t secondaryFileType   :  3; /** */
              uint64_t isObsolete          :  1; /** "ON means obsolete file */
              uint64_t isEOF               :  1; /** "ON means end-of-file control pointers, no other bits used */
              uint64_t /* padding */       :  0;
} FileControlPointer;

/** File History Words */
typedef struct {
/* Word  1 */ uint64_t dataSetID_1_10      : 60; /** Data set identifier from the current HDR1 label characters 1-10 */
              uint64_t /* padding */       :  0;

/* Word  2 */ uint64_t /* blank */         : 18;
              uint64_t dataSetID_13_17     : 30; /** Data set identifier from the current HDR1 label characters 13-17 */
              uint64_t dataSetID_11_12     : 12; /** Data set identifier from the current HDR1 label characters 11-12 */
              uint64_t /* padding */       :  0;


/* Word  3 */ uint64_t lastWriteYear       :  6; /** "Last year file was written" */
              uint64_t lastWriteDay        :  9; /** "Last day of year file was written" */
              uint64_t lastWriteTime       : 15; /** "Last time the file was written" */
              uint64_t lastReadYear        :  6; /** "Last year file was read, 0 = 1976" */
              uint64_t lastReadDay         :  9; /** "Last day of year file was read" */
              uint64_t lastReadTime        : 15; /** "Last time the file was read" */
              uint64_t /* padding */       :  0;

/* Word  4 */ uint64_t versionNum          : 12; /** "Version number/generation number */
              uint64_t useCount            : 12; /** "Use count; i.e., the number of times the file was referenced; needed for PLIB" */
              uint64_t /* "open" */        : 36;
              uint64_t /* padding */       :  0;

/* Word  5 */ uint64_t writePasswd         : 30; /** Password for writing */
              uint64_t readPasswd          : 30; /** Password for reading */ // FIXME: these could be backwards
              uint64_t /* padding */       :  0;

/* Word  6 */ uint64_t maxRecordNum        : 30; /** Maximum record number */
              uint64_t recordLen           : 30; /** Record length */
              uint64_t /* padding */       :  0;

/* Word  7 */ uint64_t expirationDay       : 18; /** "Expiration day in DPC" */
              uint64_t expirationYear      : 12; /** "Expiration year in DPC" */
              uint64_t creationDay         : 18; /** "Creation day in DPC" */
              uint64_t creationYear        : 12; /** "Creation year in DPC" */
              uint64_t /* padding */       :  0;

/* Word  8 */ uint64_t /* "open" */        : 60;
              uint64_t /* padding */       :  0;
} FileHistoryWord_Data;

typedef struct __attribute__((packed)) {
	char dataSetID[17];
	char padding1[19];
	char useCount[2];
	char versionNum[2];
	char writePasswd[5];
	char readPasswd[5];
	char recordLen[5];
	char maxRecordNum[5];
	char creationYear[2];
	char creationDay[3];
	char expirationYear[2];
	char expirationDay[3];
	char padding2[10];
} FileHistoryWord_Text;

typedef struct {
/* Word "9"*/ uint64_t wordsToFirstPtr     : 24; /** "Words to first pointer in data block N" */
              uint64_t lastRecord          : 21; /** "Last record starting in data block N" */
              // The next 14 bits are listed as "open" in the
              // tech note, but 12 bits are listed as a checksum (LBCKS)
              // in the assembly
              uint64_t checksum            : 12;
              uint64_t /* "open" */        :  2;
              uint64_t noRecordStartsHere  :  1; /** "ON means that no record starts in this block" */
              uint64_t /* padding */       :  0;

/* TODO: Word "+9+M-1", Word "+9+M" */
} BlockControlPointer;

/**
 * Data buffer flags. "Precede each data record."
 *
 * This is the same structure that is described in fcon; aka "Record
 * Control Word."
 */
typedef struct {
              uint64_t nextPtrOffset              : 21; /** "Words to next buffer pointer" */
              uint64_t prevPtrOffset              : 18; /** "Words to previous buffer pointer" */
              uint64_t recordDataMode             :  6; /** "Mode of data record in this record (see Data Type bits 51-44 of first word of the label buffer)" */
              uint64_t numBits                    :  6; /** "Number of bits in last data word controlled by this pointer" */
              uint64_t recordIsShorter            :  1; /** "Record shorter than words to next pointer-1, length is in bits 0-21 of first data word" */
              uint64_t recordNotWritten           :  1; /** "Record not written (direct access volume) */
              uint64_t sourceRecordHasParityError :  1; /** "Source record had a parity error" */
              uint64_t endLabelGroup              :  1; /** "End of label group file mark" */
              uint64_t labelRecordFollows         :  1; /** "Label record follows" */
              uint64_t isLoadPoint                :  1; /** "Load Point for tape simulation" */
              uint64_t isEOF                      :  1; /** "End of file" */
              uint64_t isEOD                      :  1; /** "End of data in this volume" */
              uint64_t isRecordStart              :  1; /** "Start of record" */
              uint64_t /* padding */              :  0;
} DataBufferFlags;

/**
 * Representation of a file contained within a TBM archive. (There may be more
 * than one file contained in one TBM archive.) This does *not* directly map
 * to the organization of data contained in an actual TBM archive.
 */
typedef struct {
	size_t size;
	HDR1_Text hdr1_text;
	HDR1_Data hdr1_data;
	HDR2_Text hdr2_text;
	HDR2_Data hdr2_data;
	HDR1_Text eof1_text;
	HDR1_Data eof1_data;
	size_t offsetToDataStart; /** Offset to where data (NOT the header) begins */
	size_t offsetToDataEnd; /** Length of the file in 60-bit words */
} TBMFile;

/**
 * Representation of a TBM archive, containing one or more TBM files. Note that
 * this structure does *not* directly map to the organization of data contained
 * in an actual TBM archive.
 */
typedef struct {
	SYSLBN_Text syslbn_text;
	SYSLBN_Data syslbn_data;
	TBMFile *files;
} TBMArchive;

int tbm_read(uint8_t *const inBuf, const uint64_t bk, TBMFile **const files,
             int *const numFiles);

void read_syslbn(uint8_t const*const inBuf, SYSLBN_Text *const text,
                 SYSLBN_Data *const data, const size_t offset);
void read_fileHistoryWord(uint8_t const*const inBuf,
                          FileHistoryWord_Text *const text,
                          FileHistoryWord_Data *const data,
                          const size_t offset);
void read_dataBufferFlags(uint8_t const*const inBuf,
                          DataBufferFlags *const dbf,
                          const size_t offset);
void read_fileControlPointer(uint8_t const*const inBuf,
                             FileControlPointer *const fcp,
                             const size_t offset);
void read_vol1(uint8_t const*const inBuf,
               VOL1_Text *const text,
               VOL1_Data *const data,
               const size_t offset);
void read_hdr1(uint8_t const*const inBuf,
               HDR1_Text *const text,
               HDR1_Data *const data,
               const size_t offset);
void read_hdr2(uint8_t const*const inBuf,
               HDR2_Text *const text,
               HDR2_Data *const data,
               const size_t offset);

void print_vol1(VOL1_Text const*const text, VOL1_Data const*const data,
                const size_t offset);
void print_hdr1(HDR1_Text const*const text, HDR1_Data const*const data,
                const size_t offset);
void print_hdr2(HDR2_Text const*const text, HDR2_Data const*const data,
                const size_t offset);

void print_syslbn(SYSLBN_Text const*const text, SYSLBN_Data const*const data,
                  const size_t offset);
void print_fileControlPtr(FileControlPointer const*const fcp,
                          const size_t offset, const int printHorizontal,
                          const int printHeader);
void print_fileHistoryWord(FileHistoryWord_Text const*const fhw_text,
                           FileHistoryWord_Data const*const fhw_data,
                           const size_t offset);
void print_blockControlPointer(BlockControlPointer const*const bcp,
                               const size_t offset, const int printHorizontal,
                               const int printHeader);
void print_dataBufferFlags(DataBufferFlags const*const dbf,
                           const size_t offset, const int printHorizontal,
                           const int printHeader);
void print_offset(const size_t offset);

#endif
