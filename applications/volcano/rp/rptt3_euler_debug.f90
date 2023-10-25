subroutine clawpack46_rptt3_mapped(ixyz,icoor,ilr,impt,maxm,meqn,mwaves,& 
    maux,mbc,mx,ql_cart,qr_cart,aux1,aux2,aux3,bsasdq_cart,& 
    cmbsasdq_cart,cpbsasdq_cart)
 
    !! ==================================================================
    !! 
    !! # Riemann solver in the transverse direction for the
    !! # Euler equations.
    !! #
    !! # Uses Roe averages and other quantities which were
    !! # computed in rpn3eu and stored in the common block comroe.
    !! #
    !! #
    !! # On input,
    !! 
    !! #    ql,qr is the data along some one-dimensional slice, as in rpn3
    !! #         This slice is
    !! #             in the x-direction if ixyz=1,
    !! #             in the y-direction if ixyz=2, or
    !! #             in the z-direction if ixyz=3.
    !! #    asdq is an array of flux differences (A^*\Dq).
    !! #         asdq(i,:) is the flux difference propagating away from
    !! #         the interface between cells i-1 and i.
    !! #    Note that asdq represents B^*\Dq if ixyz=2 or C^*\Dq if ixyz=3.
    !! #
    !! #    ixyz indicates the direction of the original Riemann solve,
    !! #         called the x-like direction in the table below:
    !! #
    !! #               x-like direction   y-like direction   z-like direction
    !! #      ixyz=1:        x                  y                  z
    !! #      ixyz=2:        y                  z                  x
    !! #      ixyz=3:        z                  x                  y
    !! #
    !! #    icoor indicates direction in which the transverse solve should
    !! #         be performed.
    !! #      icoor=2: split in the y-like direction.
    !! #      icoor=3: split in the z-like direction.
    !! # 
    !! #    For example,
    !! #        ixyz=1, icoor=3 means bsasdq=B^*A^*\Dq, and should be
    !! #                        split in z into
    !! #                           cmbsasdq = C^-B^*A^*\Dq,
    !! #                           cpbsasdq = C^+B^*A^*\Dq.
    !! #
    !! #        ixyz=2, icoor=3 means bsasdq=C^*B^*\Dq, and should be
    !! #                        split in x into
    !! #                           cmbsasdq = A^-C^*B^*\Dq,
    !! #                           cpbsasdq = A^+C^*B^*\Dq.
    !! #
    !! #    The parameters imp and impt are generally needed only if aux
    !! #    arrays are being used, in order to access the appropriate
    !! #    variable coefficients:
    !! #
    !! #    imp =  1 if bsasdq = B^*A^- \Dq, a left-going flux difference
    !! #           2 if bsasdq = B^*A^+ \Dq, a right-going flux difference
    !! #    impt = 1 if bsasdq = B^-A^* \Dq, a down-going flux difference
    !! #           2 if bsasdq = B^+A^* \Dq, an up-going flux difference
    !! #
    !! #    aux2(:,:,2) is a 1d slice of the aux array along the row
    !! #                 where the data ql, qr lie.   
    !! #    aux1(:,:,2) and aux3(:,:,2) are neighboring rows in the 
    !! #                 y-like direction
    !! #    aux2(:,:,1) and aux2(:,:,3) are neighboring rows in the 
    !! #                z-like direction

    use setprob_mod, only : gamma1, mcapa
    implicit none

    integer ixyz, icoor, ilr, impt, maxm, meqn, mwaves,maux,mbc, mx
    double precision       ql_cart(meqn,1-mbc:maxm+mbc)
    double precision       qr_cart(meqn,1-mbc:maxm+mbc)
    double precision   bsasdq_cart(meqn,1-mbc:maxm+mbc)
    double precision cmbsasdq_cart(meqn,1-mbc:maxm+mbc)
    double precision cpbsasdq_cart(meqn,1-mbc:maxm+mbc)
    double precision   aux1(maux,1-mbc:maxm+mbc,3)
    double precision   aux2(maux,1-mbc:maxm+mbc,3)
    double precision   aux3(maux,1-mbc:maxm+mbc,3)

    double precision dtcom, dxcom, dycom, dzcom, tcom
    integer icom, jcom, kcom
    common /comxyzt/ dtcom,dxcom,dycom,dzcom,tcom,icom,jcom,kcom

    double precision wave(5,3),s_rot(3), bsasdq(5), uvw(3)
    double precision uvw_cart(3), rot(9), wave_cart(5,3)

    integer i, j, mws, m, i1, info
    double precision uvw2, pres, enth, area
    integer locrot, locarea, irot
    integer mv,mu,mw

    logical debugm, debugp

    debugp = .false.
    debugm = .false.
    if (icoor .eq. 2) then
        if (ixyz .eq. 1 .and. jcom .eq. 4 .and. kcom .eq. 4) then
            if (ilr .eq. 2 .and. impt .eq. 2) then
                debugm = .true.
                debugp = .true.
            endif
        endif
    endif
    debugp = .false.
    debugm = .false.

    IF(ixyz == 1)THEN
       mu = 2
       mv = 3
       mw = 4
    ELSE IF(ixyz == 2)THEN
       mu = 3
       mv = 4
       mw = 2
    ELSE
       mu = 4
       mv = 2
       mw = 3
    ENDIF


    call get_aux_locations_tt(ixyz,icoor,mcapa,locrot,locarea,irot)

    !! # Solve Riemann problem in the second coordinate direction
    do i = 2-mbc, mx+mbc
        i1 = i + ilr - 2

        !! # compute values needed for Jacobian.  These are passed to
        !! # subroutine 'solve_riemann'
        uvw_cart(1) = ql_cart(2,i)/ql_cart(1,i)
        uvw_cart(2) = ql_cart(3,i)/ql_cart(1,i)
        uvw_cart(3) = ql_cart(4,i)/ql_cart(1,i)
        uvw2 = uvw_cart(1)**2 + uvw_cart(2)**2 + uvw_cart(3)**2
        pres = gamma1*(ql_cart(5,i)  - 0.5d0*uvw2*ql_cart(1,i))
        enth = (ql_cart(5,i) + pres) / ql_cart(1,i)

        !! # -------------------------------------------------------
        !! # Compute cmbsasdq
        !! # -------------------------------------------------------

        !! Set value to avoid compiler warnings
        area = aux2(locarea,i1,1)
        if (icoor .eq. 2) then
            if (impt .eq. 1) then
                do j = 1,9
                    rot(j) = aux2(locrot+j-1,i1,1)
                enddo
                area = aux2(locarea,i1,1)
            elseif (impt .eq. 2) then
                do j = 1,9
                    rot(j) = aux2(locrot+j-1,i1,3)
                enddo
                area = aux2(locarea,i1,3)
            endif
        elseif (icoor .eq. 3) then
            if (impt .eq. 1) then
                do j = 1,9
                    rot(j) = aux1(locrot+j-1,i1,2)
                enddo
                area = aux1(locarea,i1,2)
            elseif (impt .eq. 2) then
                do j = 1,9
                    rot(j) = aux3(locrot+j-1,i1,2)
                enddo
                area = aux3(locarea,i1,2)
            endif
        endif

        do m = 1,meqn
            bsasdq(m) = bsasdq_cart(m,i)
        enddo
        bsasdq(2) = bsasdq_cart(mv,i)
        bsasdq(3) = bsasdq_cart(mu,i)
        bsasdq(4) = bsasdq_cart(mw,i)

!!        call rotate3(rot,bsasdq(2))
!!        bsasdq(1) 

        do j = 1,3
!!            uvw(j) = uvw_cart(j)
        enddo
!!        call rotate3(rot,uvw)
        uvw(1) = uvw_cart(mv)
        uvw(2) = uvw_cart(mu)
        uvw(3) = uvw_cart(mw)


        call solve_riemann(uvw,enth, bsasdq, wave,s_rot,info)

        if (info > 0) then
            write(6,*) 'Calling from double transverse solve; C-'
            stop
        endif

        do mws = 1,mwaves
            wave_cart(1, mws) = wave(1,mws)
            wave_cart(mu,mws) = wave(2,mws)
            wave_cart(mv,mws) = wave(3,mws)
            wave_cart(mw,mws) = wave(4,mws)
            wave_cart(5, mws) = wave(5,mws)
!!            call rotate3_tr(rot,wave(2,mws))
            s_rot(mws) = area*s_rot(mws)
        enddo

        do m=1,meqn
            cmbsasdq_cart(m,i) = 0.d0
            do mws=1,mwaves
!!                cmbsasdq_cart(m,i) = cmbsasdq_cart(m,i) & 
!!                      + min(s_rot(mws), 0.d0) * wave(m,mws)
                cmbsasdq_cart(m,i) = cmbsasdq_cart(m,i) & 
                      + min(s_rot(mws), 0.d0) *wave_cart(m,3)
            enddo
        enddo
        if (debugm) then
            !!write(6,211) 3, i, (ql_cart(j,i),j=1,5)
            write(6,211) 3, i, (bsasdq_cart(j,i)/area**2,j=1,5)
            write(6,211) 3, i, (cmbsasdq_cart(j,i)/area**3,j=1,5)
        endif



        !! # -------------------------------------------------------
        !! # Compute cpbsasdq
        !! # -------------------------------------------------------
        cycle

        !! Set value to avoid compiler warnings
        area = 0
        if (icoor .eq. 2) then
            if (impt .eq. 1) then
                do j = 1,9
                    rot(j) = aux3(locrot+j-1,i1,1)
                enddo
                area = aux3(locarea,i1,1)
            elseif (impt .eq. 2) then
                do j = 1,9
                    rot(j) = aux3(locrot+j-1,i1,3)
                end do
                area = aux3(locarea,i1,3)
            endif
        elseif (icoor .eq. 3) then
            if (impt .eq. 1) then
                do j = 1,9
                    rot(j) = aux1(locrot+j-1,i1,3)
                enddo
                area = aux1(locarea,i1,3)
            elseif (impt .eq. 2) then
                do j = 1,9
                    rot(j) = aux3(locrot+j-1,i1,3)
                end do
                area = aux3(locarea,i1,3)
            endif
        endif

        do m = 1,meqn
            bsasdq(m) = bsasdq_cart(m,i)
        enddo
        call rotate3(rot,bsasdq(2))

        do j = 1,3
            uvw(j) = uvw_cart(j)
        enddo
        call rotate3(rot,uvw)

        call solve_riemann(uvw,enth,bsasdq, wave,s_rot,info)

        if (info > 0) then
            write(6,*) 'Calling from double transverse solve; C+'
            stop
        endif

        do mws = 1,mwaves
            call rotate3_tr(rot,wave(2,mws))
            s_rot(mws) = area*s_rot(mws)
        end do

        do m=1,meqn
            cpbsasdq_cart(m,i) = 0.d0
!!            do mws=1,mwaves
!!               cpbsasdq_cart(m,i) = cpbsasdq_cart(m,i) & 
!!                    + max(s_rot(mws),0.d0) * wave(m,mws)
!!            enddo
        enddo
        if (debugp) then
            write(6,211) 3, i, (cpbsasdq_cart(j,i)/area**3,j=1,5)
            write(6,*) ' '
        endif

    enddo  !! end of i loop
211    format(2I5,5E16.8) 



    return
end subroutine clawpack46_rptt3_mapped
