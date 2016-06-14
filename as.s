************************************************************************
*                                                                      *
* NAME:      BDIR                                                      *
*                                                                      *
* PURPOSE:   TO VERIFY LABEL BUFFER TABLE INFORMATION AND TO PRODUCE A *
*            FILE DIRECTORY.                                           *
*                                                                      *
* CALL:      CALL BDIR(LBT,DH,DL,BKI,IER)                              *
*                                                                      *
* ON ENTRY:  LBT - CONTAINS LABEL BUFFER TABLE                         *
*                                                                      *
* ON EXIT:   DH  - A ONE DIMENSIONAL ARRAY WHICH CONTAINS DIRECTORY    *
*                  LIST HEADER INFORMATION (SEE TABLE FIELD            *
*                  DEFINITION OF DH)                                   *
*                                                                      *
*            DL  - A TWO DIMENSIONAL ARRAY, WITH ITS 1ST DIMENSION     *
*                  EQUAL TO LE@DH, WHICH CONTAINS DIRECTORY LIST(SEE   *
*                  TABLE FIELD DEFINATION OF DL)                       *
*                                                                      *
*            BKI - A ONE DIMENSIONAL ARRAY - CONTAINS BLOCK INFORMATION*
*                  (SEE TABLE FIELD DEFINATION OF BI)                  *
*                                                                      *
*            IER - ERROR CODE                                          *
*                                                                      *
************************************************************************
         IDENT     BDIR
*************************************************
*                                               *
*  BDIR DEFINED PARAMETERS                      *
*                                               *
*************************************************
OS       =         63             OFFSET FOR 7600/CRAY FIELD CONVERSION
OSSWN    =         O'300          OFFSET FOR START WORD NUMBER
BKMIN    =         1              MINIMUM BK VALUE
BKMAX    =         80             MAXIMUM BK VALUE
LBLMIN   =         43             MINIMUM LABEL BUFFER TABEL LENGTH
LBLMAX   =         16384          MAXIMUM LABEL BUFFER TABEL LENGTH
DLMAX    =         1000           MAXIMUM NUMBER OF FILES IN DIRECTORY
BKIMAX   =         4096           MAXIMUM NUMBER OF BLOCKS IN LBT
FCPMIN   =         9              MINIMUM FCP ENTRY LENGTH
*
*  B-REGISTER ASSIGNMENTS
*
LB       DEFB
DH       DEFB
DL       DEFB
BI       DEFB
ERR      DEFB
TNF      DEFB
*
*  ERROR RETURN CODES TO CALLER
*
ELBT     =         28             LABEL BUFFER TABLE ERROR
EFCP     =         29             FILE CONTROL POINTER ERROR
EBCP     =         30             BLOCK CONTROL POINTER ERROR
EDLO     =         31             DIRECTORY LIST TABLE OVERFLOW
EBIO     =         32             BLOCK INFORMATION TABLE OVERFLOW
*
************************************************
*                                              *
*  TABLE FIELD DEFINITIONS                     *
*                                              *
************************************************
*
*  LABEL BUFFER TABLE
*
LB       TABLE     LE=15360,NE=1,SZ=LE@LB*NE@LB
LBMT     FIELD     0,OS-59,4      MACHINE TYPE
LBBK     FIELD     0,OS-39,8      BK VALUE FROM TLIB CARD
LBLBL    FIELD     0,OS-19,20     ACTUAL LENGTH OF LABEL BUFFER
*
LBVOL1   FIELD     4,OS-59,24     'VOL1' CHARACTERS
LBVSN    FIELD     4,OS-35,36     VOLUME SERIAL NAME CHARACTERS
*
LBOFCP   FIELD     29,OS-59,30    OFFSET TO CURRENT FCP
LBOBCP   FIELD     29,OS-29,30    OFFSET TO CURRENT BCP
*
*  FILE CONTROL FIELD DEFINITION
*
LB       REDEFINE  0
LBEOF    FIELD     0,OS-59,1      END OF FILE FLAG
LBOF     FIELD     0,OS-58,1      OBSOLETE FILE FLAG
LBSBN    FIELD     0,OS-23,12     START BLOCK NUMBER
LBNFCP   FIELD     0,OS-11,12     WORDS TO NEXT FCP
*
LBID1    FIELD     1,OS-59,48     1ST 8 CHARACTERS OF FILE IDENTIFIER
LBID2    FIELD     1,OS-11,12     2ND 8 CHARACTERS OF FILE IDENTIFIER
LBID3    FIELD     2,OS-59,36
LBID4    FIELD     2,OS-23,6      LAST CHARACTER OF FILE IDENTIFIER
LBVN     FIELD     4,OS-11,12     VERSION NUMBER
*
*  BLOCK CONTROL FIELD DEFINITION
*
LB       REDEFINE  0
LBRSF    FIELD     0,OS-59,1      RECORD START FLAG
LBCKS    FIELD     0,OS-56,12     CHECKSUM
LBSWN    FIELD     0,OS-23,24     START WORD NUMBER
*********************************************
*                                           *
*  DIRECTORY LIST TABLE                     *
*                                           *
*********************************************
DL       TABLE     LE=10,NE=1000
DLIFSN   FIELD     0,0,64         INPUT FILE SEQUENCE NO.
DLIFN1   FIELD     1,0,48         1ST 8 CHARACTERS OF INPUT FILE NAME
DLIFN2   FIELD     2,0,12         2ND 8 CHARACTERS OF INPUT FILE NAME
DLIFN3   FIELD     2,12,36
DLIFN4   FIELD     3,0,6          LAST CHARACTER OF INPUT FILE NAME
DLIFVN   FIELD     4,0,36         INPUT FILE VERSION NO.
DLIFOF   FIELD     5,0,64         INPUT FILE OBSOLETE FLAG
DLOFSN   FIELD     6,0,64         OUTPUT FILE SEQUENCE NO.
DLSBN    FIELD     7,0,64         START BLOCK NUMBER(BEGINES WITH 0)
DLSWN    FIELD     8,0,64         START WORD NUMBER(BEGINES WITH 0)
DLTBN    FIELD     9,0,64         TOTAL BLOCK NUMBER
*********************************************
*                                           *
*  BLOCK INFORMATION TABLE                  *
*                                           *
*********************************************
BI       TABLE     LE=1,NE=5000
BIBKN    FIELD     0,0,13         BLOCK NUMBER
BICKS    FIELD     0,13,12        CHECKSUM
*********************************************
*                                           *
*  DIRECTORY HEADER INFORMATION TABLE       *
*                                           *
*********************************************
DH       TABLE     LE=10
DHVSN    FIELD     0,0,36         VSN
DHBK     FIELD     1,0,64         BK VALUE
DHTNF    FIELD     2,0,64         TOTAL NUMBER OF FILES
DHTNB    FIELD     3,0,64         TOTAL NUMBER OF BLOCKS
*********************************************
*                                           *
*  ENTRY                                    *
*                                           *
*********************************************
BDIR     ENTER     NP=5,NB=6
         ARGADD    A1,1           ADDRESS OF LBT(LABEL BUFFER TABLE)
         ARGADD    A2,2           ADDRESS OF DIRECTORY HEADER
         ARGADD    A3,3           ADDRESS OF DIRECTORY LIST
         ARGADD    A4,4           ADDRESS OF BLOCK INFORMATION
         ARGADD    A5,5           ADDRESS OF ERROR CODE
*
*  STORE AGRUMENT ADDRESS IN B-REGISTERS
*
         B.LB      A1
         B.DH      A2
         B.DL      A3
         B.BI      A4
         B.ERR     A5
*
*  VERIFY LABEL BUFFER TABLE
*
         GET,S0    S6&S7,LBMT,A1
         S5        ELBT
         JSN       BDERR          ERROR - INVALID MACHINE TYPE
         GET,S1    S6&S7,LBVOL1,A1
         S2        VOL1,0
         S0        S1\S2
         S5        ELBT
         JSN       BDERR          ERROR - 'VOL1' DOES NOT MATCH
         GET,S1    S6&S7,LBBK,A1
         A5        BKMAX
         A6        S1
         A0        A5-A6
         S5        ELBT
         JAM       BDERR          ERROR - BK VALUE OUT OF RANGE
         A5        BKMIN
         A0        A6-A5
         S5        ELBT
         JAM       BDERR          ERROR - BK VALUE OUT OF RANGE
         GET,S2    S6&S7,LBLBL,A1
         A5        LBLMAX
         A6        S2
         A0        A5-A6
         S5        ELBT
         JAM       BDERR          ERROR - LBL OUT OF RANGE
         A5        LBLMIN
         A0        A6-A5
         S5        ELBT
         JAM       BDERR          ERROR - LBL OUT OF RANGE
*
*  LABEL BUFFER TABLE O.K.
*  BUILD DIRECTORY HEADER - STORE VSN, BK
*
         GET,S2    S6&S7,LBVSN,A1
         PUT,S2    S6&S7,DHVSN,A2
         PUT,S1    S6&S7,DHBK,A2
*
*  VERIFY FCP ENTRY INFORMATION
*
         GET,S1    S6&S7,LBOFCP,A1
         A5        S1
         A1        A5+A1          A1= FCP ENTRY ADDRESS
         A7        -1             A7= LAST BLOCK NUMBER
         A0        0
         B.TNF     A0
BD005    =         *
         GET,S0    S6&S7,LBEOF,A1
         JSN       BD100          NORMAL EXIT - END OF LABEL BUFFER
         GET,S1    S6&S7,LBNFCP,A1
         S2        FCPMIN
         S3        S2-S1          S3= -(NUMBER OF FILES IN FCP)
         S0        S3
         S5        EFCP
         JSP       BDERR          ERROR - FCP LENGTH TOO SHORT
*
*  FCP O. K.
*  BUILD AN ENTRY IN DIRECTORY LIST
*
         A2        B.TNF
         A2        A2+1
         A6        NE@DL
         A0        A6-A2
         S5        EDLO
         JAM       BDERR          ERROR - DL TABLE OVERFLOW
         B.TNF     A2
         GET,S4    S6&S7,LBOF,A1
         PUT,S4    S6&S7,DLIFOF,A3
         GET,S4    S6&S7,LBSBN,A1
         A6        S4             A6= START BLOCK NUMBER
         PUT,S4    S6&S7,DLSBN,A3
         GET,S4    S6&S7,LBID1,A1
         PUT,S4    S6&S7,DLIFN1,A3
         GET,S4    S6&S7,LBID2,A1
         PUT,S4    S6&S7,DLIFN2,A3
         GET,S4    S6&S7,LBID3,A1
         PUT,S4    S6&S7,DLIFN3,A3
         GET,S4    S6&S7,LBID4,A1
         PUT,S4    S6&S7,DLIFN4,A3
         GET,S4    S6&S7,LBVN,A1
         PUT,S4    S6&S7,DLIFVN,A3
         S4        -S3
         PUT,S4    S6&S7,DLTBN,A3
*
*  BUILD ENTRIES IN BLOCK INFORMATION TABLE
*
         A2        S2
         A2        A1+A2          A2= BCP ENTRY ADDRESS
         GET,S4    S6&S7,LBSWN,A2
         S5        OSSWN
         S4        S4-S5
         PUT,S4    S6&S7,DLSWN,A3
         A5        S1
         A1        A1+A5          A1= NEXT FCP ENTRY ADDRESS
         A0        A7-A6
         JAM       BD010          FILE START FROM NEW BLOCK
         A2        A2+1
         A6        A6+1
BD010    =         *
         A0        A2-A1
         JAP       BD015          NO MORE BCP
         S1        A6
         S2        NE@BI-1
         S0        S2-S1
         S5        EBIO
         JSM       BDERR          ERROR - BLOCK INFORMATION TABLE OVERFL
         PUT,S1    S6&S7,BIBKN,A4
         GET,S1    S6&S7,LBCKS,A2
         PUT,S1    S6&S7,BICKS,A4
         A6        A6+1
         A4        A4+1           A4= NEXT BKI ENTRY ADDRESS
         A2        A2+1           A2= NEXT BCP ENTRY ADDRESS
         J         BD010
BD015    =         *
         A2        LE@DL
         A3        A3+A2          A3= NEXT DL ENTRY ADDRESS
         A7        A6-1
         J         BD005          MORE FCP ENTRIES
*
*  ERROR EXIT
*
BDERR    =         *
         J         BDIRX
*
*  NORMAL EXIT - COMPUTE TOTAL NUMBER OF FILES, COMPUTE TOTAL NUMBER
*                OF BLOCKS AND CLEAR ERROR CODE
*
BD100    =         *
         A1        B.TNF
         S1        A1
         A2        B.DH
         PUT,S1    S6&S7,DHTNF,A2
*
         A1        B.BI
         A1        A4-A1
         S1        A1
         PUT,S1    S6&S7,DHTNB,A2
*
         S5        0
         J         BDIRX
*
*  STORE ERROR CODE AND RETURN TO CALLER
*
BDIRX  =         *
         A5        B.ERR
         0,A5      S5
         EXIT      NB=6,NAME=BDIR
*
*  BDIR DEFINED CONSTANT
*
VOL1     CON       O'26171434     'VOL1' IN DPC
         END
*
*
************************************************************************
*                                                                      *
* NAME:      VRCW                                                      *
*                                                                      *
* PURPOSE:   TO VERIFY RECORD CONTROL WORD AND RETURN RECORD-RELATED   *
*            INFORMATION TO CALLER.                                    *
*                                                                      *
* CALL:      CALL VRCW(RCW,EOF,LR,NBS,RMODE,FPTR,IER)                  *
*                                                                      *
* ON ENTRY:  RCW - A RECORD CONTROL WORD                               *
*                                                                      *
* ON EXIT:   EOF   - END OF FILE/END OF DATA FLAG                      *
*            LR    - LABEL-RECORD FLAG                                 *
*            NBS   - NUMBER OF BITS IN LAST DATA WORD                  *
*            RMODE - MODE VALUE OF THIS RECORD                         *
*            FPTR  - WORDS TO NEXT RECORD CONTROL WORD                 *
*            IER   - ERROR CODE                                        *
*                                                                      *
************************************************************************
         IDENT     VRCW
***********************************************
*                                             *
*  VRCW DEFINED PARAMETERS                    *
*                                             *
***********************************************
OS       =         63             OFFSET FOR 7600/CRAY FIELD CONVERSION
************************************************
*                                              *
*  TABLE FIELD DEFINITION                      *
*                                              *
*  THIS DATA CAN BE FOUND IN THE NCAR TMS      *
*  BOOK - P.  8.26 (SYSTEM CHARACTERISTICS)    *
*                                              *
************************************************
*
*  DATA BUFFER FIELD DEFINITION
*
DB       TABLE     LE=1
DBSOR    FIELD     0,OS-59,1      START OF RECORD
DBEOD    FIELD     0,OS-58,1      END OF DATA IN THIS VOLUME
DBEOF    FIELD     0,OS-57,1      END OF FILE
DBLR     FIELD     0,OS-55,1      LABEL RECORD FOLLOWS
DBNOB    FIELD     0,OS-50,6      NO. OF BITS IN LAST DATA WORD
DBMODE   FIELD     0,OS-44,5      MODE OF DATA IN THIS RECORD
DBBRP    FIELD     0,OS-39,19     BACKWARD RECORD CONTROL POINTER
DBFRP    FIELD     0,OS-20,21     FORWARD RECORD CONTROL POINTER
*
*  ENTRY
*
VRCW     ENTER     NP=7
         ARGADD    A1,1           ADDRESS OF RCW
         ARGADD    A2,2           ADDRESS OF EOF FLAG
         ARGADD    A3,3           LABEL RECORD FOLLOWS - NOT DATA
         ARGADD    A4,4           ADDRESS OF NUMBER OF BITS IN LAST WORD
         ARGADD    A5,5           ADDRESS OF RECORD MODE
         ARGADD    A6,6           ADDRESS OF FORWARD RECORD POINTER
         ARGADD    A7,7           ADDRESS OF ERROR CODE
*
*  VERIFY RECORD CONTROL WORD START OF RECORD FLAG
*
         GET,S0    S6&S7,DBSOR,A1
         S5        1
         JSZ       VRERR
*
*  FETCH AND STORE OUTPUT ARGUMENTS
*
         GET,S1    S6&S7,DBEOD,A1
         GET,S2    S6&S7,DBEOF,A1
         S1        S1!S2
         0,A2      S1             STORE END OF FILE FLAG
         GET,S1    S6&S7,DBLR,A1
         0,A3      S1             STORE LABEL RECORD FLAG
         GET,S1    S6&S7,DBNOB,A1
         0,A4      S1             STORE NUMBER OF BITS IN LAST WORD
         GET,S1    S6&S7,DBMODE,A1
         0,A5      S1             STORE RECORD MODE
         GET,S1    S6&S7,DBFRP,A1
         0,A6      S1             STORE FORWARD RECORD POINTER
*
*  NORMAL EXIT
*
         S5        0
         J         VRCWX
*
*  ERROR EXIT
*
VRERR    =         *
         J         VRCWX
*
*  STORE ERROR CODE AND RETURN TO CALLER
*
VRCWX  =         *
         0,A7      S5             STORE ERROR CODE
         EXIT
         END
         IDENT     CCON
         ENTRY     ASCDPC,DPCASC
         ENTRY     MS610,MS615
************************************************************************
*                                                                      *
* ROUTINE TO CONVERT UP TO 8 ASCII CHARACTERS TO DPC                   *
*                                                                      *
************************************************************************
*                                                                      *
* ENTRY FROM FORTRAN:  CALL ASCDPC(IA,NC,IB)                           *
*                                                                      *
*                      IA CONTAINS ASCII CHARACTERS, LEFT JUSTIFIED.   *
*                      NC IS THE NUMBER OF CHARACTERS TO CONVERT.      *
*                                                                      *
* EXIT FROM FORTRAN:   IB IS THE DPC RESULT, LEFT JUSTIFIED, NULL FILL.*
*                                                                      *
* ENTRY FROM CAL:                                                      *
*                                                                      *
*        S1        ASCII CHARACTERS, LEFT JUSTIFIED                    *
*        A1        NUMBER OF CHARACTERS TO CONVERT                     *
*                                                                      *
* EXIT FROM CAL:                                                       *
*                                                                      *
*        S6        DPC RESULT, LEFT JUSTIFIED, NULL FILL               *
*                                                                      *
*        S1        ORIGINAL LEFT CIRCULARLY SHIFTED (A1)*8             *
*        A1        ORIGINAL * 8                                        *
*                                                                      *
* REGISTERS USED:  A0-A7, S1-S7                                        *
*                                                                      *
* LOOP TIME:       350 NS PER CHARACTER.                               *
*                                                                      *
* SPECIAL CASES:   LOWER-CASE ALPHABETIC CHARACTERS ARE CONVERTED TO   *
*                  UPPER CASE.  NON-DPC CHARACTERS ARE CONVERTED TO    *
*                  BLANKS.                                             *
*                                                                      *
************************************************************************
         SPACE     2
ASCDPC   ENTER     NP=3
         ARGADD    A1,1
         ARGADD    A2,2
         ARGADD    A4,3
         S1        0,A1
         A1        0,A2
MS610    =         *
         A2        0
         A3        8
         A6        6
         A1        A1*A6
         S3        <3
         S4        <6
         S6        0
         SPACE     1
MS612    =         *
         S5        S1
         S5        S5>59
         A5        S5
         S2        MS650,A5
         S1        S1,S1<A3
         S7        S1&S3
         S7        S7<3
         A7        S7
         A7        A7+A3
         A2        A2+A6
         S2        S2,S2<A7
         A0        A2-A1
         S2        S2&S4
         S2        S2,S2>A2
         S6        S6!S2
         JAN       MS612
         0,A4      S6
         EXIT      NAME=ASCDPC
         SPACE     4
************************************************************************
*                                                                      *
* ROUTINE TO CONVERT UP TO 8 DPC CHARACTERS TO ASCII                   *
*                                                                      *
************************************************************************
*                                                                      *
* ENTRY FROM FORTRAN:  CALL DPCASC(IA,NC,IB)                           *
*                      IA CONTAINS DPC CHARACTERS, LEFT JUSTIFIED.     *
*                      NC IS THE NUMBER OF CHARACTERS TO CONVERT.      *
*                                                                      *
* EXIT FROM FORTRAN:   IB IS THE ASCII RESULT, LEFT-JUSTIFIED, WITH    *
*                      NULL FILL.                                      *
*                                                                      *
* ENTRY FROM CAL:                                                      *
*                                                                      *
*        S1        DPC CHARACTERS, LEFT JUSTIFIED                      *
*        A1        NUMBER OF CHARACTERS TO CONVERT                     *
*                                                                      *
* EXIT FROM CAL:                                                       *
*                                                                      *
*        S6        ASCII RESULT, LEFT JUSTIFIED, NULL FILLED           *
*                                                                      *
*        S1        ORIGINAL LEFT CIRCULARLY SHIFTED (A1)*6             *
*        A1        ORIGINAL * 6                                        *
*                                                                      *
* REGISTERS USED:  A0-A4, S1-S7                                        *
*                                                                      *
* LOOP TIME:       350 NS PER CHARACTER                                *
*                                                                      *
************************************************************************
         SPACE     2
DPCASC   ENTER     NP=3
         ARGADD    A1,1
         ARGADD    A2,2
         ARGADD    A4,3
         S1        0,A1           S1= DPC CHARACTERS, LEFT JUSTIFIED
         A1        0,A2           A1= NUMBER OF CHARACTERS TO CCONERT
MS615    =         *
         A2        0
         A3        6
         A6        8
         A1        A1*A6
         S3        <3
         S4        <8
         S6        0
         SPACE     1
MS617    =         *
         S5        S1
         S5        S5>61
         A5        S5
         S2        MS655,A5       GET ASCII CONVERSION CODE
         S1        S1,S1<A3       LEFT ROUND SHIFT DPC - GET NXT CHAR
         S7        S1&S3          MASK OFF LOWER 3 BITS
         S7        S7<3
         A7        S7             CHAR TO CONVERT
         A7        A7+A6
         A2        A2+A6          POSITION OF NXT CHAR TO CONVERT
         S2        S2,S2<A7       LEFT RND SHIFT - SET TO REPLACE W ASCI
         A0        A2-A1          TEST IF ALL CHARS ARE CONVERTED
         S2        S2&S4          MASK OFF LOWER 8 - BITS ASCII TABLE
         S2        S2,S2>A2       POSITON ASCII CHAR TO REPLACE DPC
         S6        S6!S2          REPLACE DPC WITH ASCII CHAR
         JAN       MS617          LOOP UNTIL ALL CHARS CONVERTED
         A7        D'64
         A0        A7-A1          REMAINING BITS TO ZERO OUT
         JAZ       MS619          NO REMAINING BITS TO ZERO OUT
         S2        <D'64
         A7        A7-A1
         S2        S2<A7          SET UP MASK FOR VALID CHARS
         S6        S6&S2          ZERO OUT REMAINING BITS
MS619    0,A4      S6             RETURN CONVERTED WORD
         EXIT      NAME=DPCASC
         SPACE     4
MS650    VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         VWD       8/C' 'R,8/C'!'R,8/O'070,8/O'060
         VWD       8/C'$'R,8/C'%'R,8/C'&'R,8/O'064
         VWD       8/C'('R,8/C')'R,8/C'*'R,8/C'+'R
         VWD       8/C','R,8/C'-'R,8/C'.'R,8/C'/'R
         VWD       8/C'0'R,8/C'1'R,8/C'2'R,8/C'3'R
         VWD       8/C'4'R,8/C'5'R,8/C'6'R,8/C'7'R
         VWD       8/C'8'R,8/C'9'R,8/C':'R,8/C';'R
         VWD       8/C'<'R,8/C'='R,8/C'>'R,8/C'?'R
         VWD       8/C'@'R,8/C'A'R,8/C'B'R,8/C'C'R
         VWD       8/C'D'R,8/C'E'R,8/C'F'R,8/C'G'R
         VWD       8/C'H'R,8/C'I'R,8/C'J'R,8/C'K'R
         VWD       8/C'L'R,8/C'M'R,8/C'N'R,8/C'O'R
         VWD       8/C'P'R,8/C'Q'R,8/C'R'R,8/C'S'R
         VWD       8/C'T'R,8/C'U'R,8/C'V'R,8/C'W'R
         VWD       8/C'X'R,8/C'Y'R,8/C'Z'R,8/C'['R
         VWD       8/C'\'R,8/C']'R,8/O'076,8/O'065
         VWD       8/C' 'R,8/C'A'R,8/C'B'R,8/C'C'R
         VWD       8/C'D'R,8/C'E'R,8/C'F'R,8/C'G'R
         VWD       8/C'H'R,8/C'I'R,8/C'J'R,8/C'K'R
         VWD       8/C'L'R,8/C'M'R,8/C'N'R,8/C'O'R
         VWD       8/C'P'R,8/C'Q'R,8/C'R'R,8/C'S'R
         VWD       8/C'T'R,8/C'U'R,8/C'V'R,8/C'W'R
         VWD       8/C'X'R,8/C'Y'R,8/C'Z'R,8/C' 'R
         VWD       8/C' 'R,8/C' 'R,8/C' 'R,8/C' 'R
         SPACE     2
MS655    DATA      ':ABCDEFG'
         DATA      'HIJKLMNO'
         DATA      'PQRSTUVW'
         DATA      'XYZ01234'
         DATA      '56789+-*'
         DATA      '/()$= ,.'
         VWD       8/X'23,24/'[]%',16/X'275F,16/'!&'
         VWD       8/X'22,40/'?<>@\',8/X'5E,8/';'
         SPACE     2
         END
