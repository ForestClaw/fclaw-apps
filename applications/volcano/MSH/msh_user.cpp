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

#include "../all/euler_user.h"

#include <fclaw2d_include_all.h>

#include <fclaw3dx_clawpatch.h>
#include <fclaw3dx_clawpatch_fort.h>

#include <fc3d_clawpack46.h>
#include <fc3d_clawpack46_options.h>

#include <fc3d_clawpack46_user_fort.h>
#include <fclaw3d_metric.h>


void msh_problem_setup(fclaw2d_global_t* glob)
{
    const user_options_t* user = msh_get_options(glob);
    fclaw_options_t* fclaw_opt = fclaw2d_get_options(glob);
    fc3d_clawpack46_options_t *clawopt = fc3d_clawpack46_get_options(glob);

    if (glob->mpirank == 0)
    {
        FILE *f = fopen("setprob.data","w");
        fprintf(f,  "%-24d   %s",user->example,"\% example\n");
        fprintf(f,  "%-24d   %s",user->mapping,"\% mapping\n");
        fprintf(f,  "%-24d   %s",fclaw_opt->manifold,"\% manifold\n");
        fprintf(f,  "%-24d   %s",clawopt->mcapa,"\% mcapa\n");
        fprintf(f,  "%-24d   %s",user->init_choice,"\% init_choice\n");

        /* Overpressure and ambient parameters */
        fprintf(f,  "%-24.16f   %s",user->gamma,"\% gamma\n");
        fprintf(f,  "%-24.16f   %s",user->x0,"\% x0\n");
        fprintf(f,  "%-24.16f   %s",user->y0,"\% y0\n");
        fprintf(f,  "%-24.16f   %s",user->z0,"\% z0\n");
        fprintf(f,  "%-24.16f   %s",user->r0,"\% r0\n");
        fprintf(f,  "%-24.16f   %s",user->rhoin,"\% rhoin\n");
        fprintf(f,  "%-24.16f   %s",user->rhoout,"\% rhoout\n");
        fprintf(f,  "%-24.16f   %s",user->pin,"\% pin\n");
        fprintf(f,  "%-24.16f   %s",user->pout,"\% pout\n");

        fprintf(f,  "%-24s   %s",user->topo_file,"\% topo_file\n");

        /* Misc. grid parameters (may not be used) */
        fprintf(f,  "%-24.6f   %s",user->maxelev,"\% maxelev\n");
        fprintf(f,  "%-24.6f   %s",user->min_z,"\% minz\n");
        fprintf(f,  "%-24.6f   %s",user->max_z,"\% maxz\n");
        fprintf(f,  "%-24.6f   %s",user->mid_z,"\% midz\n");
        fclose(f);
    }

    /* We want to make sure node 0 gets here before proceeding */
    fclaw2d_domain_barrier (glob->domain);  /* redundant?  */
    SETPROB();
}

static
void msh_patch_setup(fclaw2d_global_t *glob,
                              fclaw2d_patch_t *patch,
                              int blockno,
                              int patchno)
{
    int mx,my,mz, mbc;
    double xlower,ylower,zlower, dx,dy, dz;
    fclaw3dx_clawpatch_grid_data(glob,patch,&mx,&my,&mz, &mbc,
                                &xlower,&ylower,&zlower, &dx,&dy, &dz);

    double *xd,*yd,*zd,*volume,*faceareas;
    double *xp,*yp,*zp;
    fclaw3d_clawpatch_mesh_data(glob,patch,&xp,&yp,&zp,
                                &xd,&yd,&zd,&volume,&faceareas);

    double *xrot, *yrot, *zrot;
    fclaw3d_metric_patch_basis(glob,patch,&xrot,&yrot,&zrot);

    int maux;
    double *aux;
    fclaw3dx_clawpatch_aux_data(glob,patch,&aux,&maux);

    fc3d_clawpack46_options_t *clawopt = fc3d_clawpack46_get_options(glob);

    /* Use mcapa as an offset into geometry */
    int mcapa = clawopt->mcapa;

    EULER3D_SETAUX_MANIFOLD(&mbc,&mx,&my,&mz, &mcapa, 
                                 &xlower,&ylower,&zlower,
                                 &dx,&dy,&dz,&maux,aux,&blockno,
                                 xrot,yrot,zrot,volume,faceareas);
}



void msh_link_solvers(fclaw2d_global_t *glob)
{
    const user_options_t* user = msh_get_options(glob);
    fclaw2d_vtable_t *fclaw_vt = fclaw2d_vt(glob);

    fclaw_vt->problem_setup = &msh_problem_setup;

    fclaw2d_patch_vtable_t *patch_vt = fclaw2d_patch_vt(glob);

    if (user->claw_version == 4)
    {
        fc3d_clawpack46_vtable_t *claw46_vt = fc3d_clawpack46_vt(glob);
        fc3d_clawpack46_options_t *clawopt = fc3d_clawpack46_get_options(glob);

        claw46_vt->fort_qinit  = &CLAWPACK46_QINIT;

#if 1
        fclaw3dx_clawpatch_vtable_t *clawpatch_vt = fclaw3dx_clawpatch_vt(glob);
        clawpatch_vt->fort_user_exceeds_threshold = &FCLAW3DX_USER_EXCEEDS_TH;
#endif        

        fclaw_options_t *fclaw_opt = fclaw2d_get_options(glob);
        if (fclaw_opt->manifold)
        {
            /* This calls a manifold version of setaux */
            FCLAW_ASSERT(clawopt->mcapa > 0);
            patch_vt->setup = msh_patch_setup;

            claw46_vt->fort_bc3    = &CLAWPACK46_BC3;
            claw46_vt->fort_rpn3   = &CLAWPACK46_RPN3_MAPPED; 
            claw46_vt->fort_rpt3   = &CLAWPACK46_RPT3_MAPPED;
            claw46_vt->fort_rptt3   = &CLAWPACK46_RPTT3_MAPPED;      
        }
        else
        {
            /* No aux routine */
            claw46_vt->fort_rpn3   = &CLAWPACK46_RPN3;
            claw46_vt->fort_rpt3   = &CLAWPACK46_RPT3;
            claw46_vt->fort_rptt3   = &CLAWPACK46_RPTT3;      
        }
    }
    else if (user->claw_version == 5)
    {
        fclaw_global_essentialf("3d example not yet implemented in Clawpack 5\n");
        exit(0);
    }
}