/*
Copyright (c) 2021 Carsten Burstedde, Donna Calhoun, Scott Aiton, Grady Wright
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

#include "tsunami_user.h"
#include "../sgn/sgn_options.h"
#include "../sgn/sgn.h"

#include <fclaw2d_include_all.h>

#include <fclaw2d_clawpatch_options.h>
#include <fclaw2d_clawpatch.h>

#include <fc2d_clawpack46_options.h>

#include <fc2d_clawpack46.h>

#include <fc2d_thunderegg.h>
#include <fc2d_thunderegg_options.h>

static
void create_domain(fclaw2d_global_t *glob)
{
    const fclaw_options_t* fclaw_opt = fclaw2d_get_options(glob);

    int mi = fclaw_opt->mi;
    int mj = fclaw_opt->mj;
    int a = fclaw_opt->periodic_x;
    int b = fclaw_opt->periodic_y;

    /* Construct and store domain */
    fclaw2d_domain_t *domain = 
        fclaw2d_domain_new_brick(glob->mpicomm, mi, mj, a, b,
                                 fclaw_opt->minlevel);

    /* Construct brick mapping */
    fclaw2d_map_context_t *brick = 
        fclaw2d_map_new_brick(domain, mi, mj, a, b);

    fclaw2d_map_context_t *cont = 
        fclaw2d_map_new_nomap_brick (brick);

    /* Store mapping in the glob */
    fclaw2d_global_store_map (glob, cont);            

    /* Store the domain in the glob */
    fclaw2d_global_store_domain(glob, domain);

    /* print out some info */
    fclaw2d_domain_list_levels (domain, FCLAW_VERBOSITY_ESSENTIAL);
    fclaw2d_domain_list_neighbors (domain, FCLAW_VERBOSITY_DEBUG);

#if 0
    /* Use [ax,bx]x[ay,by] */

    conn = p4est_connectivity_new_brick(mi,mj,a,b);
    brick = fclaw2d_map_new_brick(conn,mi,mj);
    cont = fclaw2d_map_new_nomap_brick(brick);

    domain = fclaw2d_domain_new_conn_map (mpicomm, fclaw_opt->minlevel, conn, cont);
    fclaw2d_domain_list_levels(domain, FCLAW_VERBOSITY_ESSENTIAL);
    fclaw2d_domain_list_neighbors(domain, FCLAW_VERBOSITY_DEBUG);  
    return domain;
#endif    
}

static
void run_program(fclaw2d_global_t* glob)
{
    /* ---------------------------------------------------------------
       Set domain data.
       --------------------------------------------------------------- */
    fclaw2d_domain_data_new(glob->domain);

    /* Set domain dimensions so that we have square grid cells */
    fclaw_options_t *fclaw_opt = fclaw2d_get_options(glob);

    double ax = fclaw_opt->ax;
    double bx = fclaw_opt->bx;
    int mi = fclaw_opt->mi;


    fclaw2d_clawpatch_options_t *clawpatch_opt = fclaw2d_clawpatch_get_options(glob);
    int mx = clawpatch_opt->mx;

    double dx = (bx-ax)/(mi*mx);

    double by = mx*dx;
    fclaw_opt->by = by;   // To get square mesh cells for pseudo 1d problem

    int mj = fclaw_opt->mj;
    int my = clawpatch_opt->my;
    double dy = (by-0)/(mj*my);
    printf("tsunami.cpp : (by,dx) = %f %f\n",fclaw_opt->by,dy);


    /* Initialize virtual table for ForestClaw */
    fclaw2d_vtables_initialize(glob);

    /* Test thunderegg solver */
    fc2d_thunderegg_solver_initialize(glob);

    /* Inialize virtual tables for clawpack */
    fc2d_clawpack46_solver_initialize(glob);

    tsunami_link_solvers(glob);

    /* ---------------------------------------------------------------
       Run
       --------------------------------------------------------------- */
    fclaw2d_initialize(glob);

    sgn_run(glob);


    fclaw2d_finalize(glob);
}


int
main (int argc, char **argv)
{
    /* Initialize application */
    fclaw_app_t *app = fclaw_app_new (&argc, &argv, NULL);


    /* Options */
#if 0    
    fclaw_options_t             *fclaw_opt;
    fclaw2d_clawpatch_options_t *clawpatch_opt;
    fc2d_clawpack46_options_t   *claw46_opt;
    fc2d_thunderegg_options_t   *te_opt;
    sgn_options_t               *sgn_opt;
    fclaw2d_global_t            *glob;
    fclaw2d_domain_t            *domain;
    sc_MPI_Comm mpicomm;
#endif    

    /* Options */
    user_options_t              *user_opt;
    fclaw_options_t             *fclaw_opt;
    fclaw2d_clawpatch_options_t *clawpatch_opt;
    fc2d_clawpack46_options_t   *claw46_opt;
    fc2d_thunderegg_options_t   *te_opt;
    sgn_options_t               *sgn_opt;

    /* Create new options packages */
    fclaw_opt =                   fclaw_options_register(app,  NULL,        "fclaw_options.ini");
    clawpatch_opt =   fclaw2d_clawpatch_options_register(app, "clawpatch",  "fclaw_options.ini");
    claw46_opt =        fc2d_clawpack46_options_register(app, "clawpack46", "fclaw_options.ini");
    te_opt =            fc2d_thunderegg_options_register(app,"thunderegg","fclaw_options.ini");
    sgn_opt =                       sgn_options_register(app,"fclaw_options.ini");  
    user_opt =                   tsunami_options_register(app,"fclaw_options.ini");  

    /* Read configuration file(s) and command line, and process options */
    int first_arg;
    fclaw_exit_type_t vexit = 
        fclaw_app_options_parse (app, &first_arg,"fclaw_options.ini.used");

    /* Run the program */
    if (!vexit)
    {
        /* Options have been checked and are valid */
        int size, rank;
        sc_MPI_Comm mpicomm = fclaw_app_get_mpi_size_rank (app, &size, &rank);
        fclaw2d_global_t * glob = fclaw2d_global_new_comm (mpicomm, size, rank);

        /* Store option packages in glob */
        fclaw2d_options_store           (glob, fclaw_opt);
        fclaw2d_clawpatch_options_store (glob, clawpatch_opt);
        fc2d_clawpack46_options_store   (glob, claw46_opt);
        fc2d_thunderegg_options_store   (glob, te_opt);
        sgn_options_store               (glob, sgn_opt);
        tsunami_options_store           (glob, user_opt);

        /* Create domain and store domain in glob */
        create_domain(glob);

        run_program(glob);
        
        fclaw2d_global_destroy(glob);
    }
    
    fclaw_app_destroy (app);

    return 0;
}

