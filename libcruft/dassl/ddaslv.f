      SUBROUTINE DDASLV (NEQ, DELTA, WM, IWM)
C***BEGIN PROLOGUE  DDASLV
C***SUBSIDIARY
C***PURPOSE  Linear system solver for DDASSL.
C***LIBRARY   SLATEC (DASSL)
C***TYPE      DOUBLE PRECISION (SDASLV-S, DDASLV-D)
C***AUTHOR  PETZOLD, LINDA R., (LLNL)
C***DESCRIPTION
C-----------------------------------------------------------------------
C     THIS ROUTINE MANAGES THE SOLUTION OF THE LINEAR
C     SYSTEM ARISING IN THE NEWTON ITERATION.
C     MATRICES AND REAL TEMPORARY STORAGE AND
C     REAL INFORMATION ARE STORED IN THE ARRAY WM.
C     INTEGER MATRIX INFORMATION IS STORED IN
C     THE ARRAY IWM.
C     FOR A DENSE MATRIX, THE LAPACK ROUTINE
C     DGETRS IS CALLED.
C     FOR A BANDED MATRIX,THE LAPACK ROUTINE
C     DGBTRS IS CALLED.
C-----------------------------------------------------------------------
C***ROUTINES CALLED  DGBTRS, DGETRF
C***REVISION HISTORY  (YYMMDD)
C   830315  DATE WRITTEN
C   901009  Finished conversion to SLATEC 4.0 format (F.N.Fritsch)
C   901019  Merged changes made by C. Ulrich with SLATEC 4.0 format.
C   901026  Added explicit declarations for all variables and minor
C           cosmetic changes to prologue.  (FNF)
C   020204  Convert to use LAPACK
C***END PROLOGUE  DDASLV
C
      INTEGER  NEQ, IWM(*)
      DOUBLE PRECISION  DELTA(*), WM(*)
C
      EXTERNAL  DGBTRS, DGETRS
C
      INTEGER  LIPVT, LML, LMU, LMTYPE, MEBAND, MTYPE, NPD, INLAPACK
      PARAMETER (NPD=1)
      PARAMETER (LML=1)
      PARAMETER (LMU=2)
      PARAMETER (LMTYPE=4)
      PARAMETER (LIPVT=21)
C
C***FIRST EXECUTABLE STATEMENT  DDASLV
      MTYPE=IWM(LMTYPE)
      GO TO(100,100,300,400,400),MTYPE
C
C     DENSE MATRIX
100   CALL DGETRS('N', NEQ, 1, WM(NPD), NEQ, IWM(LIPVT), DELTA, NEQ, 
     *     INLPCK)
      RETURN
C
C     DUMMY SECTION FOR MTYPE=3
300   CONTINUE
      RETURN
C
C     BANDED MATRIX
400   MEBAND=2*IWM(LML)+IWM(LMU)+1
      CALL DGBTRS ('N', NEQ, IWM(LML), IWM(LMU), 1, WM(NPD), MEBAND, 
     *     IWM(LIPVT), DELTA, NEQ, INLPCK)
      RETURN
C------END OF SUBROUTINE DDASLV------
      END
