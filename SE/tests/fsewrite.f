      IMPLICIT NONE
      integer MSL, NSP, INSP, FNSP, OUTNSP
      parameter (MSL=1553, NSP=11, INSP=7, FNSP=13)
      integer nstart, nend
      integer MODELL, M, I, J
      DOUBLE PRECISION MASS(MSL), R(MSL), RHO(MSL), TEMP(MSL), PR(MSL),
     $     V(MSL), DCOEFF(MSL), YPS(MSL,NSP), MTOT, ABUND(NSP)
      DOUBLE PRECISION MASS2(MSL), R2(MSL), RHO2(MSL), TEMP2(MSL), 
     $     PR2(MSL), V2(MSL), DCOEFF2(MSL), YPS2(MSL,NSP), MTOT2, 
     $     ABUND2(NSP)
      CHARACTER*7 FILENAME
      parameter (FILENAME="test.h5")
      CHARACTER*8 FILENAME2
      parameter (FILENAME2="test2.h5")
      CHARACTER*8 VERSION2
      INTEGER FID1, FID2, INTARR(MSL,INSP), INTARR2(MSL,INSP)
      REAL FLTARR(MSL,FNSP), FLTARR2(MSL,FNSP)

      include "FSE.fi"

      M = 1500

      nstart = 31001
      nend = 31009

      call FSE_OPEN(FILENAME, FID1)

      call FSE_WRITE_SATTR(FID1, -1, "testio_version", "1.0")

      call FSE_READ_SATTR(FID1, -1, "testio_version", VERSION2)

      if (VERSION2 .NE. "1.0") then
         print *, "Failed to read or write global string attribute: ", 
     $        VERSION2, "."
      endif

      do MODELL = nstart, nend
 100     format (A,I5)
         print 100, "Calling to read model number ", MODELL
         MTOT = 0.0
         do I = 1, M
            MASS(I) = 2.0*I
            MTOT = MTOT + MASS(I)
            R(I) = 3.0*I
            RHO(I) = 4.0*I
            TEMP(I) = 5.0*I
            PR(I) = 6.0*I
            V(I) = 7.0*I
            DCOEFF(I) = 8.0*I
            do J = 1, NSP
               YPS(I,J) = 100.0*I + J
            end do
            do J = 1, FNSP
               FLTARR(I,J) = 1000.0*I + J
            end do
            do J = 1, INSP
               INTARR(I,J) = 10000.0*I + J
            end do
         end do

         do J = 1, NSP
            ABUND(J) = J
         end do

         call FSE_WRITE(FID1, MODELL, M, 10,
     $        MASS, "mass", SE_DOUBLE,
     $        R, "radius", SE_DOUBLE,
     $        RHO, "rho", SE_DOUBLE,
     $        TEMP, "temperature", SE_DOUBLE,
     $        PR, "pressure", SE_DOUBLE,
     $        V, "velocity", SE_DOUBLE,
     $        DCOEFF, "dcoeff", SE_DOUBLE,
     $        YPS, "yps", SE_DOUBLE_2D, MSL, NSP,
     $        FLTARR, "fltarr", SE_FLOAT_2D, MSL, FNSP,
     $        INTARR, "intarr", SE_INT_2D, MSL, INSP)

         call FSE_WRITE_DATASET(FID1, MODELL, "testme", len("testme"), 
     $        M, 10,
     $        MASS, "mass", SE_DOUBLE,
     $        R, "radius", SE_DOUBLE,
     $        RHO, "rho", SE_DOUBLE,
     $        TEMP, "temperature", SE_DOUBLE,
     $        PR, "pressure", SE_DOUBLE,
     $        V, "velocity", SE_DOUBLE,
     $        DCOEFF, "dcoeff", SE_DOUBLE,
     $        YPS, "yps", SE_DOUBLE_2D, MSL, NSP,
     $        FLTARR, "fltarr", SE_FLOAT_2D, MSL, FNSP,
     $        INTARR, "intarr", SE_INT_2D, MSL, INSP)

         call FSE_WRITE_DATTR(FID1, MODELL, "total_mass", MTOT)

         call FSE_WRITE_DARRAYATTR(FID1, MODELL, "abundances", ABUND, 
     $        NSP)
         call FSE_READ_DARRAYATTR(FID1, MODELL, "abundances", ABUND2, 
     $        NSP)

         call FSE_READ_D(FID1, MODELL, M, "mass", MASS2)
         call FSE_READ_D(FID1, MODELL, M, "rho", RHO2)
         call FSE_READ_D(FID1, MODELL, M, "velocity", V2)
         call FSE_READ_D(FID1, MODELL, M, "temperature", TEMP2)
         call FSE_READ_D(FID1, MODELL, M, "dcoeff", DCOEFF2)
         call FSE_READ_D(FID1, MODELL, M, "pressure", PR2)
         call FSE_READ_D(FID1, MODELL, M, "radius", R2)
C *** NSP should be the number of actually used isotopes, i.e. nvar
         call FSE_READ_D_2D(FID1, MODELL, M, "yps", YPS2, MSL, NSP)
         call FSE_READ_F_2D(FID1, MODELL, M, "fltarr", FLTARR2, MSL, 
     $        FNSP)
         call FSE_READ_I_2D(FID1, MODELL, M, "intarr", INTARR2, MSL, 
     $        INSP)

         call FSE_READ_DATTR(FID1, MODELL, "total_mass", MTOT2)

         if (MTOT .ne. MTOT2) then
            exit
         endif

      call FSE_OPEN(FILENAME2, FID2)
      call FSE_WRITE_SATTR(FID2, -1, "testio_version", VERSION2)

      call FSE_WRITE(FID2, MODELL, M, 10,
     $     MASS2, "mass", 2,
     $     R2, "radius", 2,
     $     RHO2, "rho", 2,
     $     TEMP2, "temperature", 2,
     $     PR2, "pressure", 2,
     $     V2, "velocity", 2,
     $     DCOEFF2, "dcoeff", 2,
     $     YPS2, "yps", 12, MSL, NSP,
     $     FLTARR2, "fltarr", 11, MSL, FNSP,
     $     INTARR2, "intarr", 10, MSL, INSP)

         call FSE_WRITE_DATASET(FID2, MODELL, "testme", len("testme"), 
     $        M, 10,
     $        MASS2, "mass", SE_DOUBLE,
     $        R2, "radius", SE_DOUBLE,
     $        RHO2, "rho", SE_DOUBLE,
     $        TEMP2, "temperature", SE_DOUBLE,
     $        PR2, "pressure", SE_DOUBLE,
     $        V2, "velocity", SE_DOUBLE,
     $        DCOEFF2, "dcoeff", SE_DOUBLE,
     $        YPS2, "yps", SE_DOUBLE_2D, MSL, NSP,
     $        FLTARR2, "fltarr", SE_FLOAT_2D, MSL, FNSP,
     $        INTARR2, "intarr", SE_INT_2D, MSL, INSP)

      call FSE_WRITE_DATTR(FID2, MODELL, "total_mass", MTOT2)
      call FSE_WRITE_DARRAYATTR(FID2, MODELL, "abundances", ABUND2, 
     $     NSP)
      end do
      call FSE_CLOSE(FID1)
      call FSE_CLOSE(FID2)

      end
