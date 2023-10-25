      subroutine readregion(region_file)
      implicit none

      character(len=50) region_file

      integer msh_mxp1, msh_myp1
      common /msh_grid0/msh_mxp1, msh_myp1

      double precision msh_xlow, msh_xhigh
      double precision msh_ylow, msh_yhigh
      double precision msh_zlow, msh_zhigh, msh_dx, msh_dy
      double precision msh_x(1500), msh_y(1500)
      double precision msh_elev(1500,1500)

      common /msh_grid1/ msh_xlow, msh_xhigh,msh_ylow,msh_yhigh,
     &      msh_zlow, msh_zhigh, msh_dx, msh_dy, msh_x, msh_y,
     &      msh_elev

      integer i, j


      open(10,file=region_file)
      read(10,*) msh_mxp1 !! number of cells + 1
      read(10,*) msh_myp1 !! number of cells + 1
      read(10,*) msh_xlow
      read(10,*) msh_ylow
      read(10,*) msh_xhigh
      read(10,*) msh_yhigh
      read(10,*) msh_zlow  !! High point for this section of topography
      read(10,*) msh_zhigh !! low point for this section
      read(10,*) msh_dx
      read(10,*) msh_dy

      write(6,*) ' '
      write(6,*) 'MSH topo extent'
      write(6,*) '(xlow, ylow) ', msh_xlow, msh_ylow
      write(6,*) '(xhigh, yhigh) ',msh_xhigh, msh_yhigh
      write(6,*) 'width ', (msh_xhigh-msh_xlow)
      write(6,*) 'depth ', (msh_yhigh-msh_ylow)
      write(6,*) ' '


      do i = 1,msh_mxp1
         do j = 1,msh_myp1
            msh_x(i) = msh_xlow + (i-1)*msh_dx
            msh_y(j) = msh_ylow + (j-1)*msh_dy
            read(10,*) msh_elev(i,j)
         enddo
      enddo
      close(10)

      end
