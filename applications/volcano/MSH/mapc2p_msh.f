      subroutine msh_mapc2p(xc,yc,zc,xp,yp,zp)
      implicit none

      double precision xc,yc,zc,xp,yp,zp

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

      integer i,j
      double precision a,b,ej,ejp1,zbase,ztop, alpha
      double precision xc1, yc1, zc1


c     # map (xc,yc) in [-1,1] into [0,1]      
      xc1 = (xc + 1)/2.
      yc1 = (yc + 1)/2.

c     # To get cells clustered near ground level, set alpha > 0.
c     # In general, alpha should satisfy 0 <= alpha <= 1/(4*dzeta + 1).
c     # Set alpha to 0 to get linear map (no grid cell clustering)
      alpha = 0
      zc1 = alpha*zc*zc + (1 - alpha)*zc

c     # Scale xc1,yc1 into interval [xlow, xhigh].
      xp = (msh_xhigh - msh_xlow)*xc1 + msh_xlow
      yp = (msh_yhigh - msh_ylow)*yc1 + msh_ylow


c     # Choose grid height (into atmosphere)
c     # Here we chose 4 times highest point on topography.
      ztop = 4*msh_zhigh

c     # Get index i in msh_x for interval [i,i+1] that brackets value xp
c     # Get index j in msh_y for interval [j,j+1] that brackets value yp
c     # Ghost cell values should just take same elevation as nearest
c     # cell in region in which elevations are specified.
      i = int(min(max((xp - msh_xlow)/msh_dx + 1,1.d0),msh_mxp1-1.))
      j = int(min(max((yp - msh_ylow)/msh_dy + 1,1.d0),msh_myp1-1.))

c     # Do bilinear interpolation here
      a = (xp - msh_x(i))/(msh_x(i+1) - msh_x(i))
      b = (yp - msh_y(j))/(msh_y(j+1) - msh_y(j))

      ej   = msh_elev(i,j)   + a*(msh_elev(i+1,j)   - msh_elev(i,j))
      ejp1 = msh_elev(i,j+1) + a*(msh_elev(i+1,j+1) - msh_elev(i,j+1))

c     % To have a scaled mapping with no topography, use this
c      ej = msh_elev(1,1)
c      ejp1 = msh_elev(1,1)

      zbase =  ej + b*(ejp1 - ej)

c     # Assign mapping values
      zp = (ztop - zbase)*zc1 + zbase

      end
