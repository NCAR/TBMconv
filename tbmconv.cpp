
/**
 * Author: Nicholas DeCicco <nsd.cicco@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "gbytes.cpp"
#include "cdc.hpp"

typedef struct {
	char *str;
} Label;

// Rounds up division.
#define DIV_CEIL(n,d) (((n)-1)/(d)+1)

#define SYSLBN_COMPRESSED_LENGTH

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
#define MAGIC_HDR1   0x20449C
#define MAGIC_HDR2   0x20449D
#define MAGIC_VOL1   0x58F31C
#define MAGIC_NCARSY 0x3830524D9
#define MAGIC_STEMHD 0x4D414D204
#define MAGIC_1000   0x71B6DB
#define MAGIC_1      0x1C

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

/**
 * SYSLBN.
 */
typedef struct {
/* Word  0 */ 
              uint64_t labelBufLen        : 20; /** Actual length of the SYSLBN label buffer */
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

typedef struct __attribute__((packed)) {
/* Word   0- 3 */ char padding1[40];
// ==================== VOL1 ====================
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
// ==================== HDR1 ====================
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
// ==================== HDR2 ====================
/* Word     20 */ char hdr2[4];               /** Should read "HDR2" */
/* Words 20-27 */ char hdr2label[76];         /** Header 2 label */
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
/* Word  0 */
              uint64_t nextFCPOff          : 12; /** "[Number of?] words to next file control pointer". */
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
              uint64_t recordLength        : 30; /** Record length */
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
 * This is the same structure that is described in fcon
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

typedef struct {
/* Word DB20:   "Stack Entry for Current Call" */
                uint64_t busyTablePtr             :  6; /** "Busy Table Pointer" (BTP) */
                uint64_t callingSequencePtr       : 21; /** "Calling Sequence for CM" (text) / "Ptr to Calling Sequence" (figure) */
                uint64_t entryLoc                 :  6; /** "Pointer Position" */
                uint64_t nextPtr                  :  6; /** "Next Pointer" */
                uint64_t prevPtr                  :  6; /** "Previous Pointer" */
                uint64_t controlPoint             : 12; /** "Control Point" */
                uint64_t /* "open" */             :  1;
                uint64_t wait                     :  1; /** "Wait Flag" */
                uint64_t isDispatched             :  1; /** "Dispatched Flag" */
                uint64_t /* padding */            :  0;

/* Word DB21+0: "Calling Sequence for Current Call" */
                uint64_t count                    : 18; /** "Count" ("CNT") */
                uint64_t /* open */               : 12;
                uint64_t opCode                   : 11; /** "Op Code" */
                uint64_t /* padding */            :  0;

/* Word DB21+1: "Calling Sequence for Current Call" (continued) */
                uint64_t statusCellPtr            : 18; /** Pointer to Status Cell" */
                uint64_t /* "open" */             : 12;
                uint64_t logicalUnitNumCellPtr    : 18; /** "Pointer To Cell Containing LUN" */
                uint64_t logicalUnitNum           : 12; /** "Logcal Unit Number" */
                uint64_t /* padding */            :  0;

/* Word DB21+2: "Calling Sequence for Current Call" (continued) */
                uint64_t lwaPlusOne               : 21; /** "LWA+1" */
                uint64_t /* open */               :  9;
                uint64_t fwa                      : 21; /** "FWA" */
                uint64_t /* open */               :  9;
                uint64_t /* padding */            :  0;

/* Word DB21+3: "Calling Sequence for Current Call" (continued) */
                uint64_t unitCellContents         : 60; /** "Contents of Unit Cell from IOC call */
                uint64_t /* padding */            :  0;

/* Word DB22:   "I/O Activity Flag" */
                uint64_t ioActivityFlag           : 60; /** "If nonzero call has been sent to DDC for this logical unit;
                                                             if disk is busy for this unit, bit 0 set */
                uint64_t /* padding */            :  0;

/* Word DB23:   "SYSMSI Length" */
                uint64_t sysmsiLen                : 21; /** Length of SYSMSI */
                uint64_t /* open */               :  9;
                uint64_t syslbiLen                : 22; /** Length of SYSLBI */
                uint64_t /* open */               :  8;
                uint64_t /* padding */            :  0;

/* Word DB24+0: (no description) */
                uint64_t tlibCatCWEntryLoc        : 19; /** "Location of TLIB Catalog CW Entry" */
                uint64_t /* open */               :  5;
                uint64_t volumeSerialNum          : 36; /** "VSN if tape" */
                uint64_t /* padding */            :  0;

/* Word DB24+1: (no description) */
//                uint64_t 
} SYSMSI;

void swizzle8(uint64_t *const _in, const size_t len);
void print_syslbn(SYSLBN_Text const*const text, SYSLBN_Data const*const data);

int main(int argc, char **argv)
{
	FILE *fp;
	char *inFileName;
//	char *outFileName;
	SYSLBN_Data syslbn_data;
	SYSLBN_Text syslbn_text;
	uint8_t *inBuf;
	size_t readAmount;
	FileHistoryWord_Data fhw_data;
	FileHistoryWord_Text fhw_text;
	FileControlPointer fcp;
	size_t offset;

	DataBufferFlags dbf;
	*((uint64_t*) &dbf) = 0x0000DC13AA00048;

//	if (argc != 3) {
	if (argc != 2) {
		fprintf(stderr, "Error: Require exactly two arguments.\n");
		printf("Usage:\n"
		       "\n"
		       "    tbmconv INFILE OUTFILE\n");
		exit(1);
	}

	inFileName = argv[1];
//	outFileName = argv[2];

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

//	swizzle8((uint64_t*) inBuf, sizeof(SYSLBN_Data)/8);

	gbytes<uint8_t,uint64_t>(inBuf, (uint64_t*) &syslbn_data, 0, 60, 0, sizeof(SYSLBN_Data)/8);

	print_syslbn(&syslbn_text, &syslbn_data);

	uint8_t test[5];
	uint64_t in = syslbn_data.vol1;
	gbytes<uint64_t,uint8_t>(&in, test, 64-4*6, 6, 0, 4);
	cdc_decode((char*) test, 4);
	test[4] = '\0';
	printf("test: %s\n", test);

	assert(syslbn_data.vol1 == MAGIC_VOL1);
	assert(syslbn_data.hdr1 == MAGIC_HDR1);
	assert(syslbn_data.dataSetID_1_6 == MAGIC_NCARSY);
	assert(syslbn_data.dataSetID_7_12 == MAGIC_STEMHD);
	assert(syslbn_data.dataSetID_13_16 == MAGIC_1000);
	assert(syslbn_data.dataSetID_17 == MAGIC_1);
	assert(syslbn_data.hdr2 == MAGIC_HDR2);

	/* The location of the first file control pointer is specified in the
	 * SYSLBN.
	 */

	offset = syslbn_data.firstFCPOff * 60;

	do {
		gbytes<uint8_t,uint64_t>(inBuf+(offset/8), (uint64_t*) &fcp, offset%8,
		                         60, 0, sizeof(FileControlPointer)/8);

		// file history word always follows the FCP?
		gbytes<uint8_t,uint64_t>(inBuf+((offset+60)/8), (uint64_t*) &fhw_data,
		                         (offset+60)%8, 60, 0,
		                         sizeof(FileHistoryWord_Data)/8);
		gbytes<uint8_t,uint8_t>(inBuf+((offset+60)/8), (uint8_t*) &fhw_text,
		                        (offset+60)%8, 6, 0,
		                         sizeof(FileHistoryWord_Text));
		cdc_decode((char*) &fhw_text, sizeof(FileHistoryWord_Text));
		offset += fcp.nextFCPOff*60;
	} while (!fcp.isEOF);

	return 1;
}

void swizzle8(uint64_t *const _in, const size_t len)
{
	uint8_t *in = (uint8_t*) _in;
	uint8_t tmp;
	size_t i;
	for (i = 0; i < len; i++) {
		tmp = in[i*8+0]; in[i*8+0] = in[i*8+7]; in[i*8+7] = tmp;
		tmp = in[i*8+1]; in[i*8+1] = in[i*8+6]; in[i*8+6] = tmp;
		tmp = in[i*8+2]; in[i*8+2] = in[i*8+5]; in[i*8+5] = tmp;
		tmp = in[i*8+3]; in[i*8+3] = in[i*8+4]; in[i*8+4] = tmp;
	}
}

void print_syslbn(SYSLBN_Text const*const text, SYSLBN_Data const*const data)
{
printf(
" ==== SYSLBN ==== \n"
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
