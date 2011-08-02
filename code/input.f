      PROGRAM ARRAY1S
c
c    Since you will be pushing your real work out of the main program
c    into Subprograms, you need to know about passing arrays in and out
c    of Subroutines.  Here is a rewrite of array1.f
c
c    John Mahaffy   12/27/94
c
      IMPLICIT NONE
      INTEGER ND
      PARAMETER (ND=10)
c
      REAL A(ND),B(ND),C(ND)
      REAL CSUM,CMAX,CMIN,AVERAGE
c
      DATA  A/1.,2.,3.,4.,5.,6.,7.,8.,9.,10./,B/3*1.,4*2.,3*3./
c
C$ SMECY MAP(PE,1)
      CALL CPROPS(A,B,C,ND,AVERAGE,CMAX,CMIN)
      CALL WRITEC(C,ND,AVERAGE,CMAX,CMIN)
      STOP
      END
      SUBROUTINE CPROPS( A, B, C, N , CAVG, CMAX, CMIN)
C
C   A common practice in Fortran 77 programs is to use a * in the dimension
C   specification.  You are saying "trust me, I know what I'm doing. All
C   you need to know is that its an array"
C
      REAL A(*),B(*),C(*)
      REAL CSUM,CMAX,CMIN,CAVG
      INTEGER I,N
c
c     Begin Executable Statements
c
      CSUM=0.
      CMAX=-1.E38
      CMIN=1.E38
      DO 100 I=1,N
         C(I)=A(I)+B(I)
         CMAX=MAX(C(I),CMAX)
         CMIN=MIN(C(I),CMIN)
 100     CSUM=CSUM+C(I)
C
      CAVG=CSUM/N
      RETURN
      END
      SUBROUTINE WRITEC( C, N, CAVG, CMAX, CMIN)
C
C   A common practice in Fortran 77 programs is to use a * in the dimension
C   specification.  You are saying "trust me, I know what I'm doing. All
C   you need to know is that its an array"
C
      REAL C(*)
      REAL CMAX,CMIN,CAVG
      INTEGER J,N
C
c    The next write statement illustrates use of * for the unit number.
c    It represents the default unit, which is the terminal.
c
      WRITE(*,*) ' RESULTS FOR FULL C ARRAY'
c
c    The format associated with the following write spreads the output
c    of the three values (AVERAGE, CMIN, and CMAX) over 3 output lines.
c
      WRITE(6,2000)CAVG,CMIN,CMAX
 2000 FORMAT(' AVERAGE OF ALL ELEMENTS IN C = ', F8.3,/,
     &       ' MINIMUM OF ALL ELEMENTS IN C = ', F8.3,/,
     &       ' MAXIMUM OF ALL ELEMENTS IN C = ', F8.3)
c
      WRITE(6,2001) (C(J),J=1,N)
 2001 FORMAT(' C = ',/,(8E10.2))
C
      RETURN
      END
