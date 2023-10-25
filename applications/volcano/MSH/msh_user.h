/*
  Copyright (c) 2012-2022 Carsten Burstedde, Donna Calhoun
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

#ifndef MSH_USER_H
#define MSH_USER_H

#include <fclaw2d_include_all.h>

#include <fclaw3dx_clawpatch_options.h>
#include <fclaw3dx_clawpatch.h>

#include <fc3d_clawpack46.h>
#include <fc3d_clawpack46_options.h>


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct user_options
{
    int example;
    int mapping; 
    int init_choice; 

    double gamma;
    double x0;
    double y0;
    double z0;
    double r0;
    double rhoin;
    double rhoout;
    double pin;
    double pout;

    double min_z;
    double mid_z;
    double max_z;

    double maxelev;

    const char* topo_file;

    int claw_version;

    int is_registered;
} user_options_t;


void msh_link_solvers(fclaw2d_global_t *glob);

user_options_t* msh_options_register (fclaw_app_t * app,
                                          const char *configfile);

void msh_options_store (fclaw2d_global_t* glob, user_options_t* user);

user_options_t* msh_get_options(fclaw2d_global_t* glob);

void msh_map_extrude(fclaw2d_map_context_t* cont,
                              const double maxelev,
                              const double minz,
                              const double maxz,
                              const double midz);


/* ------------------------------- Fortran routines ----------------------------------- */
#define MSH_MAPC2P FCLAW_F77_FUNC(msh_mapc2p, MSH_MAPC2P)

void MSH_MAPC2P(const double* xc, const double* yc, const double *zc, 
                 double *xp, double *yp, double *zp );

#define MSH_SETAUX_MANIFOLD FCLAW_F77_FUNC(msh_setaux_manifold, \
                                               MSH_SETAUX_MANIFOLD)

void MSH_SETAUX_MANIFOLD(const int* mbc,
                            const int* mx, const int* my, const int*mz,
                            const int* mcapa, 
                            const double* xlower, const double* ylower,
                            const double* zlower,
                            const double* dx, const double* dy,
                            const double *dz,
                            const int* maux, double aux[],
                            const int* blockno,
                            double xrot[], double yrot[], double zrot[],
                            double volume[],double faceareas[]);


#define CLAWPACK46_RPN3_MAPPED FCLAW_F77_FUNC(clawpack46_rpn3_mapped,  CLAWPACK46_RPN3_MAPPED)
void CLAWPACK46_RPN3_MAPPED(const int* ixyz,const int* maxm, 
                     const int* meqn, const int* mwaves,
                     const int* maux, const int* mbc, const int* mx, 
                     double ql[], double qr[],
                     double auxl[], double auxr[], double wave[],
                     double s[], double amdq[], double apdq[]);

#define CLAWPACK46_RPT3_MAPPED FCLAW_F77_FUNC(clawpack46_rpt3_mapped, CLAWPACK46_RPT3_MAPPED)
void CLAWPACK46_RPT3_MAPPED(const int* ixyz, const int* icoor, const int* imp,
                     const int *maxm, const int* meqn, const int* mwaves, 
                     const int *maux, 
                     const int* mbc, const int* mx, double ql[], double qr[],
                     double aux1[], double aux2[], double aux3[], 
                     double asdq[], double bmasdq[], double bpasdq[]);

#define CLAWPACK46_RPTT3_MAPPED    FCLAW_F77_FUNC(clawpack46_rptt3_mapped, CLAWPACK46_RPTT3_MAPPED)
void CLAWPACK46_RPTT3_MAPPED(const int* ixyz, const int* icoor, const int* imp,
                      const int* impt, const int* maxm, const int* meqn,
                      const int* mwaves, const int* maux,
                      const int* mbc,const int* mx,
                      double ql[], double qr[],
                      double aux1[], double aux2[],
                      double aux3[],  double bsasdq[],
                      double cmbsasdq[], double cpbsasdq[]);


#ifdef __cplusplus
}
#endif

#endif
