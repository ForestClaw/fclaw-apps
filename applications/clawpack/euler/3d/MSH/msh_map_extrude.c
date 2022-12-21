/*
Copyright (c) 2012-2022 Carsten Burstedde, Donna Calhoun, Scott Aiton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "msh_user.h"

/* For 2d mappings */
#include "../all/euler_user.h"

#include <fclaw2d_map.h>


/* User defined extruded mesh mapping */
static void
msh_map_3dx(fclaw2d_map_context_t * cont, int blockno,
               double xc, double yc, double zc,
               double *xp, double *yp, double *zp)
{
    /* Brick mapping to computational coordinates [0,1]x[0,1] */
    double xc1, yc1, zc1;
    cont->mapc2m(cont,blockno,xc,yc,&xc1,&yc1,&zc1);

    /* This handles everything ... */
    MSH_MAPC2P(&xc1,&yc1,&zc,xp,yp,zp);
#if 0
    printf("%d\n",blockno);
    printf("%g, %g, %g\n",xc,yc,zc);
    printf("%g %g %g\n",xc1,yc1,zc1);
    printf("%g %g %g\n",*xp,*yp,*zp);
    printf("\n");
#endif

#if 0    

    /* This returns a Cartesian map in [-1,1]x[-1,1] */
    double xp1, yp1, zp1;
    cont->mapc2m(cont,blockno,xc,yc,&xp1,&yp1,&zp1);

    /* In extruded case, no transformations are applied to the 2d mapping */
    double maxelev    = cont->user_double_3dx[0];  
    double minz       = cont->user_double_3dx[1];  
    double maxz       = cont->user_double_3dx[2];  
    double midz       = cont->user_double_3dx[3];  

    /* Scale and shift map to proper coordinates.  This is done by 'mapc2p.f'.  */
    scale_map(cont,&xp1,&yp1,&zp1);
    shift_map(cont,&xp1,&yp1,&zp1);


    /* Stretch zc into [minz+bump, maxz] */
    double rp2 = xp1*xp1 + yp1*yp1;
    if (minz < midz && midz < maxz)
    {
        double f = (midz-minz)/(maxz-minz);
        if (zc < f)
        {                
            double zlow = minz + scale_bump*exp(-30*rp2);
            *zp = zlow  + (midz-zlow)*(zc/f);
        }
        else
        {
            double zlow = midz;
            *zp = zlow  + (maxz-zlow)*((zc-f)/(1-f));
        }
    }
    else
    {
        double zlow = minz + scale_bump*exp(-30*rp2);
        *zp = zlow  + (maxz-zlow)*zc;
    }
#endif    
}



void msh_map_extrude(fclaw2d_map_context_t* cont,
                         const double maxelev,
                         const double minz,
                         const double maxz,
                         const double midz)

{
    /* May be needed to get more general mappings */
    cont->mapc2m_3dx = msh_map_3dx;

    /* Store parameters for use in routine above */
    cont->user_double_3dx[0] = maxelev;
    cont->user_double_3dx[1] = minz;
    cont->user_double_3dx[2] = maxz;
    cont->user_double_3dx[3] = midz;

    /* This is checked in 2d mappings.  If `is_extruded=1`, then the 2d mapping
       will not be scaled, shifted or rotated
    */
    cont->is_extruded = 1;

    return;
}



