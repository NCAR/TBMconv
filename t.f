      program tbm2cos
*                                                                      *
************************************************************************
* tbm2cos                                                              *
************************************************************************
*                                                                      *
* PURPOSE  The utility tbm2cos allows a Cray job to read TBM volumes   *
*          written by the CDC 7600.  1) It unpacks the original        *
*	   records of a desired file or files from the control-word    *
*	   envelope created by the 7600.  2) It converts those         *
*	   records from 7600 format to Cray format.  3) It writes      *
*	   the resulting records to a user-specified Cray dataset.     *
*                                                                      *
*          TBMCONV will not convert all 7600-written volumes to a      *
*          form which is directly usable on the Cray.  Two cases come  *
*          immediately to mind:  1) If the records written by the      *
*          7600 were BUFFERed out and contain mixed-mode data, the     *
*          user may either have to invoke TBMCONV more than once,      *
*          with different MODE values, to get several different        *
*          output datasets which may then be selectively read for the  *
*          portions properly converted, or to invoke TBMCONV with      *
*          "MODE=1", so as just to get back the bits written by the    *
*          7600, and convert the resulting records using SCONV and/or  *
*          MCONV.  2) If the records were written on the 7600 using    *
*          unformatted FORTRAN writes and the parameter "CONV=LG" was  *
*          not used on the *VOLUME card, then either REFORM or DESEG   *
*          must be used to get rid of 7600 control words and recreate  *
*          full-length original data records (which may then need to   *
*          be operated upon by SCONV and/or MCONV, as well).           *
*                                                                      *
*     JCL  tbm2cos [-l] [-d] tbm cos				       *
*                                                                      *
*     tbm  Specifies the dataset from which files are to be converted. *
*          This dataset must contain a TBM volume written by the 7600  *
*          This parameter is required.				       *
*                                                                      *
*     cos  Name of the local dataset to receive the converted files.   *
*          Separate files in the original volume become separate files *
*	   on "odn".  This dataset is not rewound, either initially or *
*	   at termination  This parameter is required.		       *
*                                                                      *
*      -l  Directory control.  If this parameter appears, a directory  *
*          of the dataset "idn" is produced on stdout.		       *
*								       *
*      -d  Debug control.  If set, print the number and size of the    *
*          records in each file on stdout.			       *
*                                                                      *
* Example: 
*               msread blotto.tbm /MYDIR/subdir/blotto		       *
*               tbm2cos blotto.tbm blotto		               *
*                                                                      *
C----------------------------------------------------------------------*
*                                                                      *
*   Notes  Binary integers greater than 2 to the 48th power minus 1    *
*          in absolute value will not be correctly converted using     *
*          mode 6 - they will be interpreted as floating-point numbers *
*          instead.  This is crucial for persons using REFRMT, which   *
*          assumes that a dataset has been converted with MODE=6.      *
*                                                                      *
*          Mode 6 converts 7600 infinites and indefinites into Cray    *
*          reals which, if used, would cause floating-point overflow.  *
*          The values used are different depending on whether the 7600 *
*          value was a positive or a negative infinite or indefinite,  *
*          as follows:                                                 *
*                                                                      *
*               THE ORIGINAL 7600 VALUE    THE RESULTING CRAY VALUE    *
*               -----------------------    ------------------------    *
*               positive infinite          0600004000000000000003B     *
*               negative infinite          1600004000000000000002B     *
*               positive indefinite        0600004000000000000005B     *
*               negative indefinite        1600004000000000000004B     *
*                                                                      *
*          where the above Cray numbers are, of course, in octal.      *
*                                                                      *
* DEFINITIONS:                                                         *
*                                                                      *
*          LABEL BUFFER - TABLE DESCRIBING THE NAMED FILES ON A VOLUME,*
*          GENERATED BY THE CDC 7600 WHEN THE VOLUME WAS CREATED, AND  *
*          WRITTEN IN THE FIRST TBM BLOCK OF THE VOLUME.               *
*                                                                      *
*          TBM BLOCK - 15360 CRAY-1 WORDS (16384 CDC-7600 WORDS).      *
*                                                                      *
*          CDC-7600 BLOCK - 2048*BK CDC-7600 WORDS, WHERE "BK" IS AN   *
*          INTEGER IN THE RANGE 1-80 DEFINING THE BLOCK SIZE USED WHEN *
*          THE VOLUME WAS GENERATED ON THE CDC 7600.  THE DEFAULT BK   *
*          IS 8.  ALSO CALLED A TLIB BLOCK.                            *
*                                                                      *
*          CRAY-1 BLOCK - 512 CRAY-1 WORDS.                            *
*                                                                      *
*                                                                      *
* ASSUMPTIONS:                                                         *
*                                                                      *
*          THE INPUT DATASET idn IS ASSUMED TO BE AN UNBLOCKED DATASET *
*          CONTAINING A TOTAL NUMBER OF WORDS WHICH IS A MULTIPLE OF   *
*          THE TBM BLOCK SIZE (15360 CRAY-1 WORDS).                    *
*                                                                      *
*          THE LABEL BUFFER IS ASSUMED TO BE THE 1ST TBM BLOCK OF THE  *
*          DATASET idn.                                                *
*                                                                      *
*          THE RANGE OF BK IS 1 TO 40.                                 *
*                                                                      *
*          A MODE IS ASSOCIATED WITH EACH RECORD OF A FILE.            *
*                                                                      *
*             0 - DPC                                                  *
*             1 - BINARY BIT SERIAL                                    *
*             2 - BCD                                                  *
*             3 - ASCII                                                *
*             4 - EBCDIC                                               *
*             5 - BINARY INTEGER                                       *
*             6 - FLOATING-POINT                                       *
*             7 - DPC CARD IMAGE                                       *
*             8 - TRANSPARENT                                          *
*                                                                      *
*  MODE=i  Mode override.  Specifying a mode causes TBMCONV to treat   *
*          all records as if they had been written with that mode.     *
*          The default is to use the value associated with each        *
*          record as it was written on the 7600.  The possible modes   *
*          and the actions taken by TBMCONV are as follows:            *
*                                                                      *
*               MODE  ACTION OF TBMCONV                                *
*               ----  --------------------------------------------     *
*                                                                      *
*                0    Converts DPC characters to ASCII, with the       *
*                     last Cray word zero-filled on the right.         *
*                                                                      *
*                1    No conversion - just transmits the bits of       *
*                     the original record, 64 per word.                *
*                                                                      *
*                2    Intended to cause conversion from BCD to         *
*                     ASCII.  Not implemented.                         *
*                                                                      *
*                3    No conversion - same effect as 1 - record        *
*                     is assumed to contain ASCII characters.          *
*                                                                      *
*                4    Intended to cause conversion from EBCDIC         *
*                     to ASCII.  Not implemented.                      *
*                                                                      *
*                5    Converts 60-bit 7600 integers to 64-bit          *
*                     Cray integers.                                   *
*                                                                      *
*                6    Converts 60-bit 7600 reals to 64-bit Cray        *
*                     reals - if the real has a zero exponent          *
*                     field, it is assumed to be a 60-bit 7600         *
*                     integer and is converted to a 64-bit Cray        *
*                     integer.                                         *
*                                                                      *
*                7    Converts DPC cards to ASCII cards - like         *
*                     mode 0, except that COSY characters in each      *
*                     record are expanded into sequences of blanks     *
*                     and the result is forced, by truncation or       *
*                     by blank fill, to exactly eighty characters      *
*                     in length.  (A few PLIB volumes were COSYed      *
*                     for the sake of efficiency, and this mode        *
*                     was specifically intended for them, but it       *
*                     may be useful to force the output records        *
*                     to be 80-column card images.)                    *
*                                                                      *
*                8    Transparent mode (implies that the original      *
*                     volume was created on the Cray and disposed      *
*                     with "MF=76,DF=TR") - either FN or FS may        *
*                     appear, but not with a list.                     *
*                                                                      *
*                9    Transmits the bits of the original record,       *
*                     60 per word, right-justified, with the four      *
*                     leading bits zeroed.                             *
*                                                                      *
*                                                                      *
* PROGRAM FLOW:                                                        *
*                                                                      *
*          1. TRANSFER AND VERIFY CONTROL CARD PARAMETER VALUES        *
*                                                                      *
*          2. VERIFY LABEL BUFFER INFORMATION AND BUILD NAMED FILE     *
*             DIRECTORY (BDIR).                                        *
*                                                                      *
*          3. WRITE NAMED FILE DIRECTORY INFORMATION TO stdout.        *
*                                                                      *
*          4. BUILD THE FILE CONVERSION LIST FROM NAMED FILE           *
*	      DIRECTORY (BCL).			                       *
*                                                                      *
*          5. CONVERT FILES ACCORDING TO THE FILE CONVERSION LIST AND  *
*             CONTROL CARD OPTIONS (FCON).                             *
*                                                                      *
*          6. EXIT.                                                    *
*                                                                      *
* NOTE:    THE ROUTINES OF tbm2cos (AFTER THIS ONE) ARE ARRANGED       *
*          ALPHABETICALLY, SO THAT THEY MAY BE FOUND EASILY.           *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* declare some of the arrays in common.  this reduces the overall field
* length required by tbmconv.  note that there is a common declaration
* in the routine bbk (which see), as well.
*
      common rb1,rb2,dh,dl,bki,cl
*
* the buffers rb1 and rb2 are used by the routine bbk in reading the
* tbm volume.
*
      dimension rb1(15360),rb2(16384)
*
* the array fl holds the names (or sequence numbers) of files the user
* wants to convert, taken from the tbmconv control card.
*
      dimension fl(3,8)
*
* dh(1) receives the volume name, dh(2) the block size for the volume,
* dh(3) the number of files in the volume, and dh(4) the total number
* of blocks in the volume.  this information comes from the volume's
* label buffer.
*
      dimension dh(10)
*
      equivalence (dh(1),vsn),(dh(2),bk),(dh(3),nedl),(dh(4),nebki)
*
* each entry in dl consists of the sequence number (one word), the name
* (three words), the version number (one word), the obsolete flag (one
* word), the output file sequence number (one word), the starting block
* number (one word), the starting word number (one word), and the total
* number of blocks (one word), of a particular file.
*
      dimension dl(10,1000)
*
* bki is indexed by cdc-7600 block number and contains checksums for
* the blocks.  currently, this information is not used.
*
      dimension bki(5000)
*
* in cl is constructed the list of files to be converted from the
* volume.  each six-word entry specifies the name of the file (three
* words), the number of the cdc-7600 block in which it starts (one
* word), the number of the first word of the file in that block (one
* word), and the total number of blocks required to hold the file (one
* word).
*
      dimension cl(6,1000)
*
* define the termination message for the user's log file.
*
      dimension tcxxx(8)
      common/debug/debug
      data dbm /1/
      character arg*8
*
      data (tcxxx(i),i=1,8) /
     + 'tcx01 - ' , 0 , ' files converted                        ', 0 /
*
* transfer control card parameter values from the jcb into the parameter
* value table pvt.
*
      nargs = iargc()
      if (nargs .eq. 0) then
	 call pruse ()
	 call exit (1)
      endif
      do 10 i = 1, nargs
      nc = getarg (i, arg, 8)
      if (arg .eq. "-l") then
	 dof = 1
	 ddn = 0
      else if (arg(1:2) .eq. '-d') then
	 do 9 j = 2, 8
	    if (arg(j:j) .eq. 'd') debug = or (debug, dbm)
	    dbm = shiftl (dbm, 1)
    9	 continue
      else if (arg(1:1) .eq. '?' .or. arg(1:2) .eq. '-h'
     x	      .or. arg(1:4) .eq. 'help' ) then
	 call pruse ()
	 call exit (0)
      else if (idn .ne. 5) then
	 idn = 5
	 call asnunit (idn, '-O -s u -a' // arg, ier)
	 if (ier .ne. 0) then
	    ier = 4
	    go to 999
	 endif
      else if (odn .ne. 6) then
	 odn = 6
	 call asnunit (odn, '-O -a' // arg, ier)
	 if (ier .ne. 0) then
	    ier = 4
	    go to 999
	 endif
      else
	 print '(" unknown arg ", a8)', arg
	 call pruse ()
	 call exit (2)
      endif
   10 continue
      if (odn .ne. 6) then
	 print
     x	 '("Both the input and output data sets must be specified!")'
	 call exit (3)
      endif
*
* read the first record from the tbm volume (the label buffer).
*
c###############################################################################
      fct = 3
c###############################################################################
      irp=0
      irn=1
      call readr(idn,irp,irn,rb1,rb2,ier)
      if (ier.ne.0) go to 999
*
* verify label buffer and build directory.
*
      lbl=and(rb2(1),3777777b)
      if (lbl.gt.16384) then
        ier=1
        go to 999
      end if
*
      call bdir(rb2,dh,dl,bki,ier)
      if (ier.ne.0) go to 999
*
      bks=bk*2048
*
      call dpcasc(vsn,6,vsn)
*
      do 101 j=1,nedl
        dl(1,j)=j
        call dpcasc(dl(2,j),8,dl(2,j))
        call dpcasc(dl(3,j),8,dl(3,j))
        call dpcasc(dl(4,j),1,dl(4,j))
  101 continue
*
* write directory to ddn.
*
      call wdir(ddn,dof,dh,dl,ier)
*
* build file conversion table in the array fl.
*
      call bcl(dl,nedl,fl,nefl,fct,cl,necl,ier)
      if (ier.ne.0) go to 999
*
* convert files and generate output dataset according to file conversion
* table.
*
      call fcon(idn,bks,irp,mof,mode,cl,necl,odn,ier)
      if (ier.ne.0) go to 999
*
* end of conversion.
*
      tcxxx(2)=btd(necl)
      call ntb(tcxxx(2))
      call remark2(tcxxx)
      call close(idn)
*
      call exit (0)
*
* process error conditions.
*
  999 call error(ier)
      call close(idn)
*
      if (and (debug, 4) .ne. 0) call abort
      call exit (4)
*
      end
      subroutine bbk(dn,bks,bkn,drp,bkb,ier)
*
************************************************************************
* bbk                                                                  *
************************************************************************
*                                                                      *
* purpose:  to construct a specified cdc-7600 (tlib) block.            *
*                                                                      *
*   entry:  dn  - name of input dataset (containing tlib volume).      *
*           bks - cdc-7600 block size.                                 *
*           bkn - number of desired block.                             *
*           drp - dataset record position pointer.                     *
*                                                                      *
*    exit:  drp - modified dataset record position pointer.            *
*           bkb - block constructed.                                   *
*           ier - error code.                                          *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension the tlib block buffer.
*
      dimension bkb(1)
*
* declare the buffers used to read the tbm volume.
*
      common rb1(15360),rb2(16384)
*
* compute the number of the tbm block in which the tlib block begins
* (rn) and the index of its first word (rb2p) in that tbm block.
*
      wp=bkn*bks+1
      rn=(wp-1)/16384+2
      rb2p=mod(wp,16384)
*
* initialize the tlib block buffer pointer.
*
      bkbp=1
*
* read up a tbm block.
*
  101 call readr(dn,drp,rn,rb1,rb2,ier)
      if (ier.ne.0) return
*
* move words from the tbm block buffer to the tlib block buffer.
*
      nwm=min0(16385-rb2p,bks-bkbp+1)
      call move(rb2,rb2p,bkb,bkbp,nwm)
*
* quit if the tlib block is complete.
*
      if (bkbp+nwm.gt.bks) then
        ier=0
        return
      end if
*
* reset the pointers and get the next tbm block.
*
      rn=rn+1
      rb2p=1
      bkbp=bkbp+nwm
      go to 101
*
      end
      subroutine bcl(dl,nedl,fl,nefl,fct,cl,necl,ier)
*
************************************************************************
* bcl                                                                  *
************************************************************************
*                                                                      *
* purpose:  to build file conversion list.                             *
*                                                                      *
*   entry:  dl   - directory list.                                     *
*           nedl - number of entries in directory list.                *
*           fl   - file name/file sequence list.                       *
*           nefl - number of entries in file name/file sequence.       *
*           fct  - file conversion type.                               *
*                                                                      *
*    exit:  cl   - conversion list.                                    *
*           necl - number of entries in conversion list.               *
*           ier  - error code.                                         *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension arrays.
*
      dimension dl(10,1),fl(3,1),cl(6,1)
*
* pre-define the error code.
*
      ier=0
*
* branch according to the file conversion type.
*
      go to (100,200,300,400,500) fct+1
      ier=2
      return
*
* fct=0 - no file conversion needed.
*
  100 necl=0
      return
*
* fct=1 - convert files according to fn list.
*
  200 do 201 i=1,nefl
        call sdlfn(fl(1,i),dl,nedl,sbn,bkbp,tbn,ier)
        if (ier.ne.0) return
        cl(1,i)=fl(1,i)
        cl(2,i)=fl(2,i)
        cl(3,i)=fl(3,i)
        cl(4,i)=sbn
        cl(5,i)=bkbp
        cl(6,i)=tbn
  201 continue
      necl=nefl
      return
*
* fct=2 - convert files acording to fs list.
*
  300 do 301 i=1,nefl
        fsn=dtb(fl(1,i))
        if (fsn.le.0.or.fsn.gt.nedl) then
          ier=3
          return
        end if
        cl(1,i)=dl(2,fsn)
        cl(2,i)=dl(3,fsn)
        cl(3,i)=dl(4,fsn)
        cl(4,i)=dl(8,fsn)
        cl(5,i)=dl(9,fsn)
        cl(6,i)=dl(10,fsn)
  301 continue
      necl=nefl
      return
*
* fct=3 - convert all files except obsolete files.
*
  400 necl=0
      do 401 i=1,nedl
        if (dl(6,i).ne.0) go to 401
        necl=necl+1
        cl(1,necl)=dl(2,i)
        cl(2,necl)=dl(3,i)
        cl(3,necl)=dl(4,i)
        cl(4,necl)=dl(8,i)
        cl(5,necl)=dl(9,i)
        cl(6,necl)=dl(10,i)
  401 continue
      return
*
* fct=4 - convert all files.
*
  500 do 501 i=1,nedl
        cl(1,i)=dl(2,i)
        cl(2,i)=dl(3,i)
        cl(3,i)=dl(4,i)
        cl(4,i)=dl(8,i)
        cl(5,i)=dl(9,i)
        cl(6,i)=dl(10,i)
  501 continue
      necl=nedl
      return
*
      end
      subroutine error(ier)
*
************************************************************************
* error                                                                *
************************************************************************
*                                                                      *
* purpose:  to write an error message in the user's log file.          *
*                                                                      *
*   entry:  ier - error code.                                          *
*                                                                      *
************************************************************************
*
      dimension tcexx(8,32)
*
* the error messages are addressed by number.  to find the routine
* which caused the error message to be issued, search for a statement
* of the form "ier=n", where "n" is the number of the message that you
* got.  (there will only be one such statement in the program.)  the
* last five error messages are special, in that they are produced by
* virtue of ier's having been set by the cal routine bdir.
*
      data ((tcexx(i,j),i=1,8),j=1,10) /
     + 'tce01 - tbm volume error - label buffer is too long     ' , 0 ,
     + 'tce02 - tbmconv logic error - unknown conversion type   ' , 0 ,
     + 'tce03 - specified file sequence number is out of range  ' , 0 ,
     + 'tce04 - error in fortran-callable assign                ' , 0 ,
     + 'tce05 - mode value is outside legal range               ' , 0 ,
     + 'tce06 - cray dataset name is longer than 7 characters   ' , 0 ,
     + 'tce07 - tlib blocks are too large for buffer in tbmconv ' , 0 ,
     + 'tce08 - record control word from tbm volume is in error ' , 0 ,
     + 'tce09 - data record is too large for buffer in tbmconv  ' , 0 ,
     + 'tce10 - error changing dataset mode to unblocked        ' , 0 /
*
      data ((tcexx(i,j),i=1,8),j=11,20) /
     + 'tce11 - error writing one cray block to output dataset  ' , 0 ,
     + 'tce12 - can only request one file in transparent mode   ' , 0 ,
     + 'tce13 - unknown record mode - cannot perform conversion ' , 0 ,
     + 'tce14 - bcd-to-ascii conversion is not implemented      ' , 0 ,
     + 'tce15 - ebcdic-to-ascii conversion is not implemented   ' , 0 ,
     + 'tce16 - error reading record from tbm-volume dataset    ' , 0 ,
     + 'tce17 - directory list is empty                         ' , 0 ,
     + 'tce18 - specified file name is unknown                  ' , 0 ,
     + 'tce19 - error writing record to output dataset          ' , 0 ,
     + 'tce20 - error in fortran-callable save                  ' , 0 /
*
      data ((tcexx(i,j),i=1,8),j=21,30) /
     + 'tce21 - error in fortran-callable release               ' , 0 ,
     + 'tce22 - error in fortran-callable access                ' , 0 ,
     + 'tce23 - error in fortran-callable delete                ' , 0 ,
     + 'tce24 - error writing cray blocks to output dataset     ' , 0 ,
     + 'tce25 - fn and fs parameters both specified             ' , 0 ,
     + 'tce26 - file name is longer than 17 characters          ' , 0 ,
     + 'tce27 - more than 8 file names/numbers are specified    ' , 0 ,
     + 'tce28 - label buffer table error                        ' , 0 ,
     + 'tce29 - file control pointer error                      ' , 0 ,
     + 'tce30 - block control pointer error                     ' , 0 /
*
      data ((tcexx(i,j),i=1,8),j=31,32) /
     + 'tce31 - directory list table overflow                   ' , 0 ,
     + 'tce32 - block information table overflow                ' , 0 /
*
* put the error message in the user's log.
*
      if (ier.ge.1.and.ier.le.32) call remark2(tcexx(1,ier))
*
      return
      end
      subroutine fcon(idn,bks,idrp,mof,mode,cl,necl,odn,ier)
*
************************************************************************
* fcon                                                                 *
************************************************************************
*                                                                      *
* purpose:  to convert files according to mode and conversion list.    *
*                                                                      *
*   entry:  idn  - input dataset name.                                 *
*           bks  - size of cdc-7600 (tlib) block.                      *
*           idrp - input dataset record position pointer.              *
*           mof  - mode overwrite flag.                                *
*           mode - mode value.                                         *
*           cl   - conversion list.                                    *
*           necl - number of entries in conversion list.               *
*           odn  - output dataset name.                                *
*                                                                      *
*    exit:  ier  - error code.                                         *
*                                                                      *
************************************************************************
*                                                                      *
* each record control word in a tlib block has the following format    *
* (within a 64-bit cray word).  the fields required are unpacked by    *
* the routine vrcw.  for a complete description of the tlib format,    *
* see "the ncar terabit memory system", chapter 8.                     *
*                                                                      *
*    bits          use                                                 *
*    -----         -------------------------------                     *
*    0-3           unused                                              *
*    4             start of record                                     *
*    5             end of data in volume                               *
*    6             end of file marker                                  *
*    7             unused here                                         *
*    8             label group follows                                 *
*    9-12          unused here                                         *
*    13-18         no. of bits in last word                            *
*    19-23         mode of data (mof)                                  *
*    24-42         backward record control pointer                     *
*    43-63         forward record control pointer                      *
*                                                                      *
************************************************************************
*
* all variables are integer.
*
      implicit integer (a-z)
*
* dimension the conversion list.
*
      dimension cl(6,1)
*
* define buffer sizes - to prevent overflow problems, rbcl must be at
* least 1.25 times rb7l.
*
      parameter (bkbl=81920,rb7l=40000,rbcl=5*rb7l/4)
*
* define buffers required - bkb receives reconstructed tlib blocks, rb7
* receives user records unpacked from the tlib blocks, rbc receives the
* translation of those records into cray format, and bkbc receives cray
* blocks to be written (only used for transparent mode).
*
      dimension bkb(bkbl),rb7(rb7l),rbc(rbcl),bkbc(512)
      common/debug/debug
*
* pre-set the error code.
*
      ier=0
*
* quit if the conversion list is empty.
*
      if (necl.le.0) return
*
* check for a tlib-block-buffer overflow condition.
*
      if (bks.gt.bkbl) then
        ier=7
        return
      end if
*
* convert files according to conversion list.  variables used below
* include the following:
*
* bkbcp  - the index of the next unfilled word in bkbc.
* bkbp   - the index of the next record control word in bkb.
* bkn    - the number of the next tlib block to be built by bbk (or
*          the one which was just built, depending on where you are).
* cbkbn  - the number of the tlib block currently in bkb.
* eof    - the eof bit from a record control word.
* eoff   - a flag used to get around an unadvertised problem with the
*          tbm format:  normally, the label-buffer pointer to a file
*          points to the eof immediately preceding the first data
*          record.  under certain conditions, it points to the eof
*          preceding a label record.  in both of these cases, we
*          must skip down to the beginning of the data and then
*          arrange to stop on the eof following the data.
* fptr   - the forward pointer from a record control word.
* lr     - the label-record-follows flag from a record control word.
* lwbc   - the last-word-bit-count from a record control word.
* rb7p   - the index of the next unfilled word in rb7.
* rc     - count of records written.
* rcf    - record continuation flag - set to 1 to indicate that the
*          current user record is continued in the next tlib block.
* rcw    - a record control word from the tlib block.
* rmode  - the record mode extracted from a record control word.
* trf    - transparent flag - set to 1 when processing a volume
*          which was disposed from the cray with "mf=76,df=tr".
* wc     - count of words written.
*
      cbkbn=-1
      bkbcp=1
      trf=0
*
* loop through the conversion list.
*
      do 109 i=1,necl
*
        bkn=cl(4,i)
        bkbp=cl(5,i)
        rc=0
        wc=0
        rcf=0
        rb7p=1
        eoff=1
        rcnt = 0
        lrc = 1
        ll = 0
*
* jump if we don't need a new tlib block.
*
  101   if (cbkbn.eq.bkn) go to 103
*
* build a new tlib block.
*
  102   call bbk(idn,bks,bkn,idrp,bkb,ier)
        if (ier.ne.0) return
        cbkbn=bkn
*
* jump if the record-continuation flag is set to continue building a
* data record, using words from this tlib block.
*
        if (rcf.ne.0) go to 106
*
* unpack and verify the record control word.
*
  103   rcw=bkb(bkbp)
        call vrcw(rcw,eof,lr,lwbc,rmode,fptr,jer)
	if (and (debug, 2) .ne. 0) then
	   call prrcw (rcw,eof,lr,lwbc,rmode,fptr,jer)
	endif
        if (jer.ne.0.or.fptr.eq.0) then
          ier=8
          return
        end if
*
* if the record control word represents an end of file:  1) if eoff is
* non-zero, set it to zero and skip to the next record control word.
* 2) if eoff is zero, terminate conversion of this file.
*
        if (eof.ne.0) then
          if (eoff.eq.0) go to 107
          eoff=0
          go to 104
        end if
*
* if a data record follows the record control word, jump to process it.
* if a label record follows, set the flag eoff non-zero again, so that
* the eof following the label records won't look like an end-of-data
* for the file.
*
        if (lr.eq.0) go to 105
        eoff=1
*
* skip to the next record control word.
*
  104   bkbp=bkbp+fptr
        if (bkbp.le.bks) go to 103
        bkbp=bkbp-bks
        bkn=bkn+1
        go to 102
*
* have data record - set eoff to zero.
*
  105   eoff=0
*
* check for a data-buffer overflow condition.
*
        if (fptr-1.gt.rb7l) then
          ier=9
          return
        end if
*
* add words from the tlib block to the data record being built.
*
  106   nwm=min0(bks-bkbp,fptr-rb7p)
        call move(bkb,bkbp+1,rb7,rb7p,nwm)
*
* update pointers.
*
        bkbp=mod(bkbp+nwm,bks)
        rb7p=rb7p+nwm
*
* if the data record is not complete, go back for another block.
*
        if (rb7p.lt.fptr) then
          rcf=1
          bkn=bkn+1
          go to 102
        end if
*
* the data record is complete - turn off the record-continuation flag
* and reset the pointers so as to pick up the next data record after
* this one is written out.
*
        rcf=0
        bkbp=bkbp+1
        if (bkbp.eq.1) bkn=bkn+1
        rb7p=1
*
* set the length and the mode of the output dataset.
*
        rl7=fptr-1
*
        if (mof.eq.0) mode=rmode
*
* increment the output record count and word count.
*
        rc=rc+1
        wc=wc+rl7
*
* if we're dealing with a dataset in transparent mode and we haven't
* done so already, change the output dataset to an unblocked dataset.
*
        if (mode.eq.8.and.trf.eq.0) then
          trf=1
	  call asnunit (odn, '-I -s u', ier)
          if (jer.ne.0) then
            ier=10
            return
          end if
        end if
*
* convert the record as implied by the mode.
*
        call rcon(mode,rb7,rl7,lwbc,rbc,rlc,ier)
        if (ier.ne.0) return
*
* write out the record, using the appropriate routine.
*
        if (trf.eq.0) then
          call wodn(odn,rbc,rlc,ier)
        else
          call uwodn(odn,rbc,rlc,bkbc,bkbcp,ier)
        end if
	if (and (debug , 1) .ne. 0) then
	   if (ll .ne. rlc) then
	      if (ll .ne. 0) then
		  print '(i9, " records sized ", i5)', lrc, ll
	      endif
	      ll = rlc
	      lrc = 1
	   else
	      lrc = lrc + 1
	   endif
	   rcnt = rcnt + 1
	endif
        if (ier.ne.0) return
*
* go back for the next data record.
*
        go to 101
*
* end of file found - either end-file the output file or, if it is
* being written in transparent mode, zero-fill and dump the last block.
*
  107   if (trf.eq.0) then
          end file odn
        else
          if (bkbcp.ne.1) then
            do 108 j=bkbcp,512
              bkbc(j)=0
  108       continue
            bkbcp=513
            call uwodn(odn,rbc,0,bkbc,bkbcp,ier)
          end if
          if (necl.gt.1) then
            ier=12
            return
          end if
        end if
	if (and (debug , 1) .ne. 0) then
	    if (ll .ne. 0) then
	        print '(i9, " records sized ", i5)', lrc, ll
	    endif
	    ll = rlc
	    lrc = 1
	    print '("end file ",i5,i9," records")', i, rcnt
	endif
*
  109 continue
*
* normal return.
*
      return
*
      end
      subroutine i7tic(a,b,n)
*
************************************************************************
* i7tic                                                                *
************************************************************************
*                                                                      *
* purpose:  to convert an array of 60-bit 7600-format integers to      *
*           an array of 64-bit cray-format integers.                   *
*                                                                      *
*   entry:  a - an array of 7600-format integers, each right-justified *
*               in a 64-bit word.                                      *
*           n - the number of elements of a to be converted.           *
*                                                                      *
*    exit:  b - an array of 64-bit cray format integers.               *
*                                                                      *
************************************************************************
*
      implicit integer (a-z)
*
* dimension the arrays.
*
      dimension a(n),b(n)
*
* loop through the input array.
*
      do 101 i=1,n
        s=and(shiftr(a(i),59),1)
        b(i)=a(i)
        if (s.ne.0) b(i)=or(b(i),mask(4))+1
  101 continue
*
      return
*
      end
      subroutine move(ia,ias,ib,ibs,nws)
*
************************************************************************
* move                                                                 *
************************************************************************
*                                                                      *
* purpose:  to move information from one array to another array.       *
*                                                                      *
*   entry:  ia  - array from which information is to be moved.         *
*           ias - index, in ia, of first word to be moved.             *
*           ib  - array to which information is to be moved.           *
*           ibs - index, in ib, of destination of first word.          *
*           nws - number of words to be moved.                         *
*                                                                      *
*    exit:  ib  - contains moved information.                          *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension arrays.
*
      dimension ia(1),ib(1)
*
      if (nws.gt.0) then
        do 101 i=1,nws
          ib(ibs+i-1)=ia(ias+i-1)
  101   continue
      end if
*
      return
*
      end
      subroutine nc7tc(a,b,n)
*
************************************************************************
* nc7tc                                                                *
************************************************************************
*                                                                      *
* purpose:  to convert 60-bit 7600-format reals to 64-bit cray-format  *
*           reals.  indefinite, infinite, or unnormalized 7600 reals   *
*           yield infinite cray reals - the sign bit is set to match   *
*           that of the 7600 real and the low-order three bits are set *
*           to indicate which one or more of the conditions above      *
*           caused the infinite to be generated.  reals with a zero    *
*           exponent are assumed to be integers and converted as such. *
*                                                                      *
*   entry:  a - an array of 7600-format reals, each right-justified in *
*               a 64-bit word.                                         *
*           n - the number of elements of a to be converted.           *
*                                                                      *
*    exit:  b - an array of 64-bit cray-format reals.                  *
*                                                                      *
************************************************************************
*
      implicit integer (a-z)
*
* dimension the arrays.
*
      dimension a(n),b(n)
*
* loop through the input array.
*
      do 103 i=1,n
*
* pick off the sign, exponent, and mantissa.
*
        s=and(shiftr(a(i),59),1b)
        e=and(shiftr(xor(a(i),-s),48),3777b)
        m=and(xor(a(i),-s),7777777777777777b)
*
* if the exponent of a(i) is precisely zero, generate a cray integer.
*
        if (e.ne.0) go to 101
        b(i)=(1-2*s)*m
        go to 103
*
* a(i) is real - check for indefinite, infinite, un-normalized.
*
  101   if (e.ne.1777b.and.e.ne.3777b.and.m.ge.4000000000000000b)
     +                                                         go to 102
        b(i)=or(shiftl(s,63),600004000000000000000b)
*
* set lower bits to indicate problem which caused illegal real.
*
        if (e.eq.1777b) b(i)=or(b(i),4b)
        if (e.eq.3777b) b(i)=or(b(i),2b)
        if (m.lt.4000000000000000b) b(i)=or(b(i),1b)
        go to 103
*
* a(i) is real and nothing is wrong with it.
*
  102   if (e.lt.2000b) e=e+1
        b(i)=or(or(shiftl(s,63),shiftl(36060b+e,48)),m)
*
* end of loop.
*
  103 continue
*
* the entire array is done - return to the caller.
*
      return
*
      end
      subroutine ntb(ia)
*
************************************************************************
* ntb                                                                  *
************************************************************************
*                                                                      *
* purpose:  to convert leading nulls to blanks.                        *
*                                                                      *
*   entry:  ia - word which contains leading nulls.                    *
*                                                                      *
*    exit:  ia - word with leading nulls replaced by blanks.           *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
      bmask=mask(8)
      blank=' 'l
*
      do 101 i=1,8
        ib=and(bmask,ia)
        if (ib.ne.0) return
        ia=or(ia,blank)
        bmask=shiftr(bmask,8)
        blank=shiftr(blank,8)
  101 continue
*
      return
*
      end
      subroutine prrcw (rcw,eof,lr,lwbc,rmode,fptr,jer)
      implicit integer (a-z)
      character flags*16
      flags = '        '
      sor = and (rcw, X'0800000000000000')
      eod = and (rcw, X'0400000000000000')
      epf = and (rcw, X'0200000000000000')
      eoa = and (rcw, X'0100000000000000')
      eob = and (rcw, X'0080000000000000')
      eoc = and (rcw, X'0040000000000000')
      if (sor .ne. 0) flags( 1: 3) = 'sor'
      if (eod .ne. 0) flags( 5: 7) = 'eod'
      if (eof .ne. 0) flags( 9:11) = 'eof'
      if (eoa .ne. 0) flags(12:12) = '?'
      if ( lr .ne. 0) flags(14:15) = 'lr'
      if (eoc .ne. 0) flags(16:16) = '?'
      brp = and (shiftr (rcw, 21), X'7FFFF')
      if (lwbc .ne. and (shiftr (rcw, 45), X'3f')) then
	 print "(' lwbc wrong')"
      endif
      if (shiftr (epf, 57) .ne. eof) then
	 print "(' eof wrong')"
      endif
      mode = and (shiftr (rcw, 40), x'1f')
      if (mode .ne. rmode) then
	 print "(' mode wrong')"
      endif
      if (fptr .ne. and (rcw, X'1FFFFF')) then
	 print "(' fptr wrong')"
      endif
      print 1000, rcw, flags, lwbc, rmode, brp,fptr
 1000 format (z17,x,a16,2i3,2i8)
      end
      subroutine pruse ()
      print '("usage: tbm2cos [-l] [-d] tbm cos")'
      return
      end
      subroutine rcon(mode,rb7,rl7,lwbc,rbc,rlc,ier)
*
************************************************************************
* rcon                                                                 *
************************************************************************
*                                                                      *
* purpose:  to convert a record according to a specified mode value.   *
*                                                                      *
*   entry:  mode - mode value.                                         *
*           rb7  - record buffer for cdc-7600 record.                  *
*           rl7  - record length for cdc-7600 record.                  *
*           lwbc - last word bit count for cdc-7600 record.            *
*                                                                      *
*    exit:  rbc  - record buffer for cray-1 record.                    *
*           rlc  - record length for cray-1 record.                    *
*           ier  - error code.                                         *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension the arrays.
*
      dimension rb7(1),rbc(1)
*
* pre-define the error code.
*
      ier=0
*
* jump to the proper conversion code, depending on the mode value.
*
      go to (100,200,300,200,400,500,600,700,200,800) mode+1
      ier=13
      return
*
* the record contains dpc characters, to be converted to ascii.  the
* bits are first packed into the array rb7 and then unpacked, 48 bits
* at a whack, into rbc.  the words of rbc are then converted, one at
* a time, from dpc to ascii.  the last word has to be done specially,
* since it may not be full; it is zero-filled out to eight characters.
*
  100 rlc=((60*(rl7-1)+lwbc+5)/6+7)/8
      call sbytes(rb7,rb7,0,60,0,rl7)
      call gbytes(rb7,rbc,0,48,0,rlc)
      do 101 i=1,rlc-1
        call dpcasc(shiftl(rbc(i),16),8,rbc(i))
  101 continue
      call dpcasc(shiftl(rbc(rlc),16),8-(8*rlc-(60*(rl7-1)+lwbc+5)/6),
     +                                                         rbc(rlc))
      return
*
* record mode implies no conversion - make sure the last word of the
* output array is zero-filled.
*
  200 rlc=((rl7-1)*60+lwbc+63)/64
      rbc(rlc)=0
      call sbytes(rbc,rb7,0,60,0,rl7)
      return
*
* record contains bcd characters - conversion not implemented.
*
  300 ier=14
      return
*
* record contains ebcdic characters - conversion not implemented.
*
  400 ier=15
      return
*
* record contains 60-bit one's-complement integers.
*
  500 call i7tic(rb7,rbc,rl7)
      rlc=rl7
      return
*
* record contains 60-bit reals or integers.
*
  600 call nc7tc(rb7,rbc,rl7)
      rlc=rl7
      return
*
* the record contains a cosyed dpc card image, to be converted to ascii.
* first, the dpc characters are unpacked and the special cosy characters
* expanded.  then, the bits are packed into the array rb7 and unpacked,
* 48 bits at a whack, into rbc.  the words of rbc are then converted,
* one at a time, from dpc to ascii.  the last word is done specially,
* since it may not be full; it is blank-filled out to eight characters.
* the output record length is forced to eighty characters.
*
  700 mch=min0((60*(rl7-1)+lwbc+5)/6,80)
      call sbytes(rb7,rb7,0,60,0,min0(rl7,8))
      call gbytes(rb7,rbc(81),0,6,0,mch)
      nch=0
      ies=0
      do 703 i=81,80+mch
        if (ies.eq.0) then
          if (rbc(i).eq.00b) go to 704
          if (rbc(i).lt.64b) then
            nch=nch+1
            rbc(nch)=rbc(i)
          else if (rbc(i).ne.77b) then
            nbl=rbc(i)-62b
            if (nbl.gt.10) nbl=10*(nbl-9)
            nbl=min0(nbl,80-nch)
            do 705 j=nch+1,nch+nbl
              rbc(j)=55b
  705       continue
            nch=nch+nbl
          else
            ies=1
          end if
        else
          nch=nch+1
          rbc(nch)=rbc(i)
          ies=0
        end if
        if (nch.eq.80) go to 704
  703 continue
  704 call sbytes(rb7,rbc,0,6,0,nch)
      rlc=(nch+7)/8
      call gbytes(rb7,rbc,0,48,0,rlc)
      do 701 i=1,rlc-1
        call dpcasc(shiftl(rbc(i),16),8,rbc(i))
  701 continue
      call dpcasc(shiftl(rbc(rlc),16),8-(8*rlc-nch),rbc(rlc))
      rbc(rlc)=or(rbc(rlc),shiftr('        'l,64-8*(8*rlc-nch)))
      if (rlc.lt.10) then
        do 702 i=rlc+1,10
          rbc(i)='        '
  702   continue
        rlc=10
      end if
      return
*
* record mode implies putting one 60-bit quantity in each 64-bit word.
*
  800 do 801 i=1,rl7
      rbc(i)=rb7(i)
  801 continue
      rlc=rl7
      return
*
      end
      subroutine readr(dn,drp,rn,rb1,rb2,ier)
*
************************************************************************
* readr                                                                *
************************************************************************
*                                                                      *
* purpose:  to position the dataset containing the tbm volume and read *
*           a specified block (record) from it.                        *
*                                                                      *
*   entry:  dn  - dataset name.                                        *
*           drp - dataset record position pointer.  if drp.le.0, the   *
*                 dataset position is unknown.  if drp.gt.0, it is     *
*                 positioned prior to record drp; it is assumed, in    *
*                 that case, that record number drp-1 is already in    *
*                 the buffer.  (this prevents a lot of backspacing and *
*                 rereading of records when the bk value is other than *
*                 eight.)                                              *
*           rn  - number of record to read.                            *
*           rb1 - 15360-word buffer, to be used in reading raw tbm     *
*                 blocks.                                              *
*           rb2 - 16384-word buffer, into which tbm blocks may be      *
*                 unpacked (60 bits per word).                         *
*                                                                      *
*    exit:  drp - modified dataset record position pointer.            *
*           rb1 - desired record from dataset (64 bits per word).      *
*           rb2 - desired record from dataset (60 bits per word).      *
*           ier - error code.                                          *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension the record buffers.
*
      dimension rb1(15360),rb2(16384)
*
* pre-set the error code.
*
      ier=0
*
* if the tbm volume has not been read at all, or if the record desired
* precedes the one in the buffer, make sure the volume is unblocked and
* rewind it.  otherwise, if the next record up is the desired one plus
* one, there's nothing to do.
*
      if (drp.le.0.or.rn.lt.drp-1) then
        if (drp.le.0) then
	  call asnunit (odn, '-I -s u', ier)
          if (ier.ne.0) go to 901
        end if
        rewind dn
        drp=1
      else
        if (rn.eq.drp-1) return
      end if
*
* read records until we get the desired one.
*
      do 101 i=1,rn-drp+1
        buffer in (dn,0) (rb1(1),rb1(15360))
        if (unit(dn).ge.0.0) go to 902
        drp=drp+1
  101 continue
*
* unpack the record.
*
      call gbytes(rb1,rb2,0,60,0,16384)
*
* done.
*
      return
*
* error returns.
*
  901 ier=4
      return
*
  902 ier=16
      return
*
      end
      subroutine sdlfn(fn,dl,nedl,sbkn,swn,tbkn,ier)
*
************************************************************************
* sdlfn                                                                *
************************************************************************
*                                                                      *
* purpose:  to search the directory list for a file with a given name. *
*                                                                      *
*   entry:  fn   - file name (in three words).                         *
*           dl   - directory list.                                     *
*           nedl - number of entries in directory list.                *
*                                                                      *
*    exit:  sbkn - start block number.                                 *
*           swn  - start word number.                                  *
*           tbkn - total block number.                                 *
*           ier  - error code.                                         *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension input arrays.
*
      dimension fn(1),dl(10,1)
*
      do 101 i=1,nedl
        if (dl(6,i).eq.1) go to 101
        if (fn(1).ne.dl(2,i).or.fn(2).ne.dl(3,i).or.fn(3).ne.dl(4,i))
     +                                                         go to 101
        idx=i
        go to 102
  101 continue
*
* error - filename not found.
*
      ier=18
      return
*
* match found - set starting block number, starting word number, and
* total number of blocks.
*
  102 ier=0
      sbkn=dl(8,idx)
      swn=dl(9,idx)
      tbkn=dl(10,idx)
      return
*
      end
      subroutine uwodn(dn,rb,rl,bkb,bkbp,ier)
*
************************************************************************
* uwodn                                                                *
************************************************************************
*                                                                      *
* purpose:  to write a record to the (unblocked) dataset dn.           *
*                                                                      *
*   entry:  dn   - dataset name.                                       *
*           rb   - record buffer.                                      *
*           rl   - record length.                                      *
*           bkb  - block buffer for cray-1 blocks.                     *
*           bkbp - block buffer pointer.                               *
*                                                                      *
*    exit:  ier  - error code.                                         *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension the record buffer and the cray block buffer.
*
      dimension rb(1),bkb(1)
*
* initialize the record pointer.
*
      rbp=1
*
* if the cray block buffer is empty, go check to see if we can write
* out a bunch of blocks at once.
*
      if (bkbp.eq.1) go to 102
*
* move words from the record buffer to the block buffer and update all
* the pointers.
*
  101 nwm=min0(rl-rbp+1,513-bkbp)
      call move(rb,rbp,bkb,bkbp,nwm)
      rbp=rbp+nwm
      bkbp=bkbp+nwm
*
* if the block buffer is full, write it out.
*
      if (bkbp.eq.513) then
        buffer out (dn,0) (bkb(1),bkb(512))
        if (unit(dn).ge.0.0) then
          ier=11
          return
        end if
        bkbp=1
      end if
*
* if there are enough words left to fill at least one complete cray
* block, write out as many complete blocks as possible directly from
* the record, by-passing the block buffer, and update the pointers.
*
  102 if (rl-rbp+1.ge.512) then
        rbq=rbp+512*((rl-rbp+1)/512)-1
        buffer out (dn,0) (rb(rbp),rb(rbq))
        if (unit(dn).ge.0.0) then
          ier=24
          return
        end if
        rbp=rbq+1
      end if
*
* if there are remaining words to be moved to the block buffer, go do
* it.
*
      if (rbp.le.rl) go to 101
*
* normal return.
*
      ier=0
      return
*
      end
      subroutine wdir(dn,dof,dh,dl,ier)
*
************************************************************************
* wdir                                                                 *
************************************************************************
*                                                                      *
* purpose:  to write directory information on dataset dn.              *
*                                                                      *
*   entry:  dn  - dataset name.                                        *
*           dof - directory on/off flag.                               *
*           dh  - directory header information.                        *
*           dl  - directory list.                                      *
*                                                                      *
*    exit:  ier - error code.                                          *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension directory information arrays.
*
      dimension dh(1),dl(10,1)
*
      ier=0
      if (dof.ne.0) then
        nedl=dh(3)
        if (nedl.gt.0) then
          write(dn,1001)
          write(dn,1002) (dh(i),i=1,4)
          write(dn,1003)
          write(dn,1004) (dl(1,j),dl(2,j),dl(3,j),dl(4,j),dl(6,j),
     +                                dl(8,j),dl(9,j),dl(10,j),j=1,nedl)
        else
          ier=17
        end if
      end if
      return
*
* formats.
*
 1001 format ('1volume   block size   files   blocks'/
     +        ' ------   ----------   -----   ------')
 1002 format (1x,   a6,         i13,     i8,      i9/)
 1003 format ('  file      file name     obs.  starts    at   no. of'/
     +        ' number                   flag in block  word  blocks'/
     +        ' ------ ----------------- ---- -------- ------ ------')
 1004 format (1x,   i6,        1x,2a8,a1,  i5,      i9,    i7,    i7)
*
      end
      subroutine wodn(dn,rb,rl,ier)
*
************************************************************************
* wodn                                                                 *
************************************************************************
*                                                                      *
* purpose:  to write a record to the (blocked) dataset dn.             *
*                                                                      *
*   entry:  dn  - dataset name.                                        *
*           rb  - record buffer.                                       *
*           rl  - record length.                                       *
*                                                                      *
*    exit:  ier - error code.                                          *
*                                                                      *
************************************************************************
*
* all variables are integers.
*
      implicit integer (a-z)
*
* dimension the record buffer.
*
      dimension rb(1)
*
* write a record.
*
      buffer out (dn,0) (rb(1),rb(rl))
*
* check for errors.
*
      if (unit(dn).lt.0.0) then
        ier=0
      else
        ier=19
      end if
*
* done.
*
      return
*
      end
