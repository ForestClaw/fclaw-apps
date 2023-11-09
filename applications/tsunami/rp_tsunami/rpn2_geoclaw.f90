!!======================================================================

SUBROUTINE rpn2_geoclaw(ixy,maxm,meqn,mwaves,mbc,mx, & 
           ql, qr,auxl,auxr,fwave,s,amdq,apdq,maux)

!!======================================================================
!!
!! Solves normal Riemann problems for the 2D SHALLOW WATER equations
!!     with topography:
!!     #        h_t + (hu)_x + (hv)_y = 0                           #
!!     #        (hu)_t + (hu^2 + 0.5gh^2)_x + (huv)_y = -ghb_x      #
!!     #        (hv)_t + (huv)_x + (hv^2 + 0.5gh^2)_y = -ghb_y      #
!!
!! On input, ql contains the state vector at the left edge of each cell
!!     qr contains the state vector at the right edge of each cell
!!
!! This data is along a slice in the x-direction if ixy=1
!!     or the y-direction if ixy=2.
!!
!!  Note that the i'th Riemann problem has left state qr(i-1,:)
!!     and right state ql(i,:)
!!  From the basic clawpack routines, this routine is called with
!!     ql = qr
!!
!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!                                                                           !
!      # This Riemann solver is for the shallow water equations.            !
!                                                                           !
!       It allows the user to easily select a Riemann solver in             !
!       riemannsolvers_geo.f. this routine initializes all the variables    !
!       for the shallow water equations, accounting for wet dry boundary    !
!       dry cells, wave speeds etc.                                         !
!                                                                           !
!           David George, Vancouver WA, Feb. 2009                           !
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

!!  use geoclaw_module, only: g => grav, dry_tolerance => dry_tolerance, rho
!!  use geoclaw_module, only: earth_radius, deg2rad
!!  use amr_module, only: mcapa
!!
!!  use storm_module, only: pressure_forcing, pressure_index


    IMPLICIT NONE

    !input
    INTEGER maxm,meqn,maux,mwaves,mbc,mx, ixy

    DOUBLE PRECISION  fwave(1-mbc:maxm+mbc,meqn, mwaves)
    DOUBLE PRECISION      s(1-mbc:maxm+mbc,mwaves)
    DOUBLE PRECISION     ql(1-mbc:maxm+mbc,meqn)
    DOUBLE PRECISION     qr(1-mbc:maxm+mbc,meqn)
    DOUBLE PRECISION   apdq(1-mbc:maxm+mbc,meqn)
    DOUBLE PRECISION   amdq(1-mbc:maxm+mbc,meqn)
    DOUBLE PRECISION   auxl(1-mbc:maxm+mbc,maux)
    DOUBLE PRECISION   auxr(1-mbc:maxm+mbc,maux)

    DOUBLE PRECISION :: grav, dry_tolerance, sea_level
    COMMON /common_swe/ grav, dry_tolerance, sea_level

    !local only
    INTEGER m,i,mw,maxiter,mu,mv
    DOUBLE PRECISION wall(3)
    DOUBLE PRECISION fw(3,3)
    DOUBLE PRECISION sw(3)

    DOUBLE PRECISION hR,hL,huR,huL,uR,uL,phiR,phiL,pL,pR
    DOUBLE PRECISION bR,bL,sL,sR,sRoe1,sRoe2,sE1,sE2,uhat,chat
    DOUBLE PRECISION s1m,s2m
    DOUBLE PRECISION hstar,hstartest,hstarHLL,sLtest,sRtest
    DOUBLE PRECISION tw,dxdc

    DOUBLE PRECISION  hvR, hvL, vR, vL, rho
    double precision g
    LOGICAL use_simple    

    LOGICAL rare1,rare2

    integer ii_com, jj_com
    common /common_ii/ ii_com, jj_com

    integer icom, jcom
    double precision dtcom, dxcom, dycom, tcom
    common /comxyt/ dtcom,dxcom,dycom,tcom,icom,jcom

    integer i1, i2

    use_simple = .false.

!!  no pressure forcing
    pL = 0
    pR = 0

    g = grav

    rho = -9999999.d0 !! To make sure this doesn't get used. 

!!  !set normal direction
    if (ixy .eq. 1) then
        mu=2
        mv=3
    else
        mu=3
        mv=2
    endif

    !loop through Riemann problems at each grid cell
    DO i = 2-mbc,mx+mbc
        ii_com = i

        if (jcom .eq. 1) then
!!            write(6,100) i, ql(i,1), ql(i,2), ql(i,3)
        endif

        !! -----------------------Initializing-----------------------------------
        !! inform of a bad riemann problem from the start
        IF ((qr(i-1,1) .LT. 0.d0) .OR. (ql(i,1) .LT. 0.d0)) THEN
            WRITE(6,201) 'Negative input: hl,hr,i=',i, qr(i-1,1),ql(i,1)
            stop
        ENDIF
201  format(A,I5,2E12.4)          

        !!Initialize Riemann problem for grid interface
        DO mw = 1,mwaves
            s(i,mw)=0.d0
            fwave(i,1,mw) = 0.d0
            fwave(i,2,mw) = 0.d0
            fwave(i,3,mw) = 0
        END DO

        !zero (small) negative values if they exist
        IF (qr(i-1,1) .LT. 0.d0) THEN
            qr(i-1,1) = 0.d0
            qr(i-1,2) = 0.d0
            qr(i-1,3) = 0.d0
        ENDIF

        IF (ql(i,1) .LT. 0.d0) THEN
            ql(i,1) = 0.d0
            ql(i,2) = 0.d0
            ql(i,3) = 0.d0
        ENDIF

        !skip problem if in a completely dry area
        IF (qr(i-1,1) <= dry_tolerance .AND. ql(i,1) <= dry_tolerance) THEN
            go to 30
        ENDIF

        !! Riemann problem variables
        hL  = qr(i-1,1)
        hR  = ql(i,1)
        huL = qr(i-1,mu)
        huR = ql(i,mu)
        bL  = auxr(i-1,1)
        bR  = auxl(i,1)

!!         if (pressure_forcing) then
!!             pL = auxr(pressure_index, i-1)
!!             pR = auxl(pressure_index, i)
!!         end if

         hvL = qr(i-1,mv) 
         hvR = ql(i,mv)        

        !!check for wet/dry boundary
        IF (hR .GT. dry_tolerance) THEN
            uR = huR/hR
            vR=hvR/hR
            phiR = 0.5d0*grav*hR**2 + huR**2/hR
        ELSE
            hR = 0.d0
            huR = 0.d0
            hvR = 0.d0
            uR = 0.d0
            vR = 0
            phiR = 0.d0
        ENDIF

        IF (hL .gt. dry_tolerance) THEN
            uL = huL/hL
            vL=hvL/hL
            phiL = 0.5d0*grav*hL**2 + huL**2/hL
        ELSE
            hL = 0.d0
            huL = 0.d0
            hvL = 0.d0
            uL = 0.d0
            vL = 0.d0
            phiL = 0.d0
        ENDIF

        wall(1) = 1.d0
        wall(2) = 1.d0
        wall(3) = 1.d0
        IF (hR .LE. dry_tolerance) THEN
            CALL riemanntype(hL,hL,uL,-uL,hstar,s1m,s2m, &
                             rare1,rare2,1,dry_tolerance,grav)

            hstartest = MAX(hL,hstar)
            IF (hstartest + bL .LT. bR) THEN
                !!right state should become ghost values that mirror left for wall problem
                !! bR=hstartest+bL
                wall(2) = 0.d0
                wall(3) = 0.d0
                hR = hL
                huR = -huL
                bR = bL
                phiR = phiL
                uR = -uL
                vL = vR
            ELSEIF (hL+bL.LT.bR) THEN
                bR = hL + bL
            ENDIF
        ELSEIF (hL .LE. dry_tolerance) THEN ! right surface is lower than left topo
            CALL riemanntype(hR,hR,-uR,uR,hstar,s1m,s2m, &
                             rare1,rare2,1,dry_tolerance,grav)
            hstartest = MAX(hR,hstar)
            IF (hstartest + bR .LT. bL) THEN
                !!left state should become ghost values that mirror right
                !! bL=hstartest+bR
                wall(1) = 0.d0
                wall(2) = 0.d0
                hL = hR
                huL = -huR
                bL = bR
                phiL = phiR
                uL = -uR
                vR = vL
            ELSEIF (hR + bR .LT. bL) THEN
                bL = hR + bR
            ENDIF
        ENDIF

        !!determine wave speeds
        sL = uL - SQRT(grav*hL) !! 1 wave speed of left state
        sR = uR + SQRT(grav*hR) !! 2 wave speed of right state

        uhat = (SQRT(grav*hL)*uL + SQRT(grav*hR)*uR)/(SQRT(grav*hR) + SQRT(grav*hL)) 
        chat = SQRT(grav*0.5d0*(hR + hL)) 
        sRoe1 = uhat - chat 
        sRoe2 = uhat + chat 

        sE1 = MIN(sL,sRoe1) 
        sE2 = MAX(sR,sRoe2) 

        !!--------------------end initializing...finally----------

        !!solve Riemann problem.

        if (use_simple) then                 
            CALL  simple_riemann(hR,uR,vr, hL,uL,vl, uhat,chat,bL, bR, &
                                 phiR,phiL,sw,fw)
        else
            maxiter = 1
            jj_com = 1
            CALL riemann_aug_JCP(maxiter,3,3,hL,hR,huL, huR, & 
                                 hvL,hvR,bL,bR,uL,uR,vL,vR, phiL, phiR, &
                                 pL,pR,sE1,sE2,dry_tolerance,grav,rho,sw,fw)
        endif

        !! eliminate ghost fluxes for wall
        DO mw = 1,mwaves
            sw(mw)   = sw(mw)*wall(mw)
            fw(1,mw) = fw(1,mw)*wall(mw)
            fw(2,mw) = fw(2,mw)*wall(mw)
            fw(3,mw) = fw(3,mw)*wall(mw)
        ENDDO

        DO mw = 1,mwaves
            s(i,mw) = sw(mw)
            fwave(i,1,mw) = fw(1,mw)
            fwave(i,mu,mw) = fw(2,mw)
            fwave(i,mv,mw) = fw(3,mw)
        ENDDO


30  CONTINUE
    ENDDO

    !!===============================================================================


    !!============= compute fluctuations=============================================
    amdq(:,1:3) = 0.d0
    apdq(:,1:3) = 0.d0
    DO i = 2-mbc,mx+mbc
        DO  mw = 1,3
            IF (s(i,mw) < 0.d0) THEN
                amdq(i,1:3) = amdq(i,1:3) + fwave(i,1:3,mw)
            ELSE IF (s(i,mw) > 0.d0) THEN
                apdq(i,1:3)  = apdq(i,1:3) + fwave(i,1:3,mw)
            ELSE
                amdq(i,1:3) = amdq(i,1:3) + 0.5d0 * fwave(i,1:3,mw)
                apdq(i,1:3) = apdq(i,1:3) + 0.5d0 * fwave(i,1:3,mw)
            ENDIF
        ENDDO
    ENDDO

    if (jcom .eq. 1) then
        i1 = 10
        i2 = 20
        do i = i1, i2
!!        write(6,100) i, s(i,1), s(i,2), s(i,3)
!!            write(6,100) i, fwave(i,2,1), fwave(i,2,2), fwave(i,2,3)
        end do
    endif

100 format(I5,4F24.16)     

    RETURN
END SUBROUTINE rpn2_geoclaw


