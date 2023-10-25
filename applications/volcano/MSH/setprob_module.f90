module setprob_mod
    implicit none

    double precision pi, pi2
    integer example
    integer mapping
    integer manifold, mcapa
    integer init_choice
    double precision gamma, gamma1
    double precision x0, y0, z0, r0, maxelev
    double precision qin(5), qout(5)

    character(len=50) topo_file

end module setprob_mod

subroutine setprob
    use setprob_mod

    implicit none

    !! Not needed in a common blocks
    double precision rhoin, rhoout, pin, pout

    double precision pi_com, pi2_com
    common /compi/ pi_com, pi2_com

    integer mapping_com
    common /com_mapping/ mapping_com


    pi = 4.d0*atan(1.d0)
    pi2 = 2*pi

    pi_com = pi
    pi2_com = pi2

    open(10,file='setprob.data')
    read(10,*) example
    read(10,*) mapping
    read(10,*) manifold
    read(10,*) mcapa
    read(10,*) init_choice

    mapping_com = mapping

    !! # These should be read in as options
    read(10,*) gamma
    gamma1 = gamma - 1.d0

    read(10,*) x0    
    read(10,*) y0    
    read(10,*) z0
    read(10,*) r0    
    read(10,*) rhoin 
    read(10,*) rhoout
    read(10,*) pin
    read(10,*) pout
!!    read(10,*) topo_file
    close(10)

    topo_file = 'msh3.region'


    !! # density outside bubble and pressure ahead of shock are fixed:

    !! From MSH
    !! rhoin = 1.5d0
    !! rhoout = 0.8d0
    !! pin    = 1.d7
    !! pout   = 1.d5

    qin(1) = rhoin
    qin(2) = 0.d0
    qin(3) = 0.d0
    qin(4) = 0.d0
    qin(5) = pin/gamma1

    qout(1) = rhoout
    qout(2) = 0.d0
    qout(3) = 0.d0
    qout(4) = 0.d0
    qout(5) = pout/gamma1

    call readregion(topo_file)

    return
end subroutine setprob

subroutine setprob_donotcall
     implicit none

     double precision gamma, gamma1, x0,y0,z0,r0
     double precision rhoin, pin, rhoout, pout
     character(len=50) topo_file

     open(unit=7,file='setprob.data')

     read(7,*) rhoin
     read(7,*) rhoout
     read(7,*) pin
     read(7,*) pout

     read(7,*) x0
     read(7,*) y0
     read(7,*) z0
     read(7,*) r0
     read(7,*) gamma
     read(7,*) topo_file
     close(7)

     gamma1 = gamma - 1.d0

     write(6,'(A)') '        Input values '
     write(6,*) '-----------------------------------'
     write(6,1001)  'rho (in)             ', rhoin
     write(6,1001)  'rho (out)            ', rhoout
     write(6,1002)  'pressure (in)        ', pin
     write(6,1002)  'pressure (out)       ', pout
     write(6,*) ' '

1001 format (A,F16.8)
1002 format (A,E16.8)

     call readregion(topo_file)

     return
end subroutine setprob_donotcall
