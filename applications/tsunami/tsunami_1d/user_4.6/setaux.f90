SUBROUTINE clawpack46_setaux(maxmx,maxmy,mbc,mx,my, & 
           xlower,ylower,dx,dy,maux,aux)

    IMPLICIT NONE
    INTEGER maxmx, maxmy, mbc,mx,my, maux
    DOUBLE PRECISION xlower,ylower, dx, dy
    DOUBLE PRECISION  aux(1-mbc:mx+mbc,1-mbc:my+mbc,maux)

    INTEGER i, j, ibc, jbc
    DOUBLE PRECISION xc, yc, b, grad(2), d2xzb, d2yzb, d2xyzb

    yc = 0
    DO j = 1-mbc,my+mbc
        DO i = 1-mbc,mx+mbc
            xc = xlower + (i-0.5)*dx     
            call sgn_fort_bathy_complete(xc,yc,b,grad,d2xzb,d2yzb,d2xyzb)
            aux(i,j,1) = b
            aux(i,j,2) = grad(1)
            aux(i,j,3) = d2xzb
        end do
    enddo

    !! Use exact gradient and second derivatives
    return

    do j= 1-mbc,mx+2
        do i = 2-mbc,mx+1
            aux(i,j,2) = (aux(i+1,j,1) - aux(i-1,j,1))/(2.d0*dx)
        END DO
    END DO

    do j= 2-mbc,mx+1
        do i = 1-mbc,mx+2
            aux(i,j,2) = (aux(i,j+1,1) - aux(i,j-1,1))/(2.d0*dx)
            aux(i,j,3) = (aux(i,j+1,i) - 2.d0*aux(i,j,1) + aux(i,j-1,1))/(dx*dx)
        END DO
    END DO

END SUBROUTINE CLAWPACK46_SETAUX