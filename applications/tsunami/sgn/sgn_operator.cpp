/*
Copyright (c) 2019-2020 Carsten Burstedde, Donna Calhoun, Scott Aiton, Grady Wright
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

#include "sgn_operator.h"
#include "sgn_patch_operator.h"

#include "sgn_options.h"

//#include "tsunami_user.h"

#include "fc2d_thunderegg.h"
#include "fc2d_thunderegg_vector.hpp"

#include <fclaw2d_elliptic_solver.h>

#include <fclaw2d_clawpatch.h>
#include <fclaw2d_clawpatch_options.h>
#include <fclaw2d_clawpatch_output_ascii.h>
#include <fclaw2d_clawpatch_output_vtk.h>

#include <fclaw2d_global.h>
#include <fclaw2d_map.h>
#include <fclaw2d_map_brick.h>
#include <fclaw2d_options.h>
#include <fclaw2d_patch.h>
#include <fclaw2d_vtable.h>

#include <p4est_bits.h>
#include <p4est_wrap.h>

#include <ThunderEgg/Iterative/BiCGStab.h>
#include <ThunderEgg/Iterative/PatchSolver.h>

#include <ThunderEgg/VarPoisson/StarPatchOperator.h>
#include <ThunderEgg/GMG/LinearRestrictor.h>
#include <ThunderEgg/GMG/DirectInterpolator.h>
#include <ThunderEgg/P4estDomainGenerator.h>
#include <ThunderEgg/GMG/CycleBuilder.h>
#include <ThunderEgg/BiLinearGhostFiller.h>
#include <ThunderEgg/Vector.h>

#if 0
#include <ThunderEgg/ValVectorGenerator.h>
//#include <ThunderEgg/Poisson/FFTWPatchSolver.h>
#endif


#include <stdlib.h>

using namespace std;
using namespace ThunderEgg;
using namespace ThunderEgg::VarPoisson;

#if 0
Vector<2> restrict_phi_n_vec(const Vector<2>& prev_beta_vec, 
                             const Domain<2>& prev_domain, 
                             const Domain<2>& curr_domain)
#endif

/*  Copy current state */
    Vector<2> restrict_q_n_vec(const Vector<2>& prev_beta_vec, 
                               const Domain<2>& prev_domain, 
                               const Domain<2>& curr_domain)
    {

    GMG::LinearRestrictor<2> restrictor(prev_domain,curr_domain, true);
    return restrictor.restrict(prev_beta_vec);

#if 0
    GMG::LinearRestrictor<2> restrictor(prev_domain,curr_domain, 
                                        prev_state_vec->getNumComponents(), true);

    auto new_state_vec = ValVector<2>::GetNewVector(curr_domain, 
                                                    prev_state_vec->getNumComponents());
    restrictor.restrict(prev_state_vec, new_state_vec);
    return new_state_vec;
#endif    
}

void sgn_solve(fclaw2d_global_t *glob) 
{
    // get needed options
    const fclaw_options_t *fclaw_opt = fclaw2d_get_options(glob);
    const fc2d_thunderegg_options_t *te_opt = fc2d_thunderegg_get_options(glob);
    const fclaw2d_clawpatch_options_t *clawpatch_opt = fclaw2d_clawpatch_get_options(glob);
  
    /* This is new - borrowed from phasefield */
    GhostFillingType fill_type = GhostFillingType::Faces;

#if 0  
    fc2d_thunderegg_vtable_t *mg_vt = fc2d_thunderegg_vt();
#endif  

    // create thunderegg vector for eqn 0
    Vector<2> f = fc2d_thunderegg_get_vector(glob,RHS);

#if 0
    shared_ptr<Vector<2>> f = make_shared<fc2d_thunderegg_vector>(glob,RHS,
                                                                  clawpatch_opt->rhs_fields);
#endif                                                                  

    // get patch size
    array<int, 2> ns = {clawpatch_opt->mx, clawpatch_opt->my};

    //bool continue_on_breakdown = false;
    //bool output = false;

    // get p4est structure
    fclaw2d_domain_t *domain = glob->domain;
    p4est_wrap_t *wrap = (p4est_wrap_t *)domain->pp;

    // create map function
#if 0
    P4estDomGen::BlockMapFunc bmf = [&](int block_no, double unit_x,      
                                        double unit_y, double &x, double &y) 
#endif                                        

    P4estDomainGenerator::BlockMapFunc bmf = [&](int block_no, double unit_x,      
                                        double unit_y, double &x, double &y) 
    {
        double x1,y1,z1;
        FCLAW2D_MAP_BRICK2C(&glob->cont,&block_no,&unit_x, &unit_y, &x1, &y1, &z1);
        x = fclaw_opt->ax + (fclaw_opt->bx - fclaw_opt->ax) * x1;
        y = fclaw_opt->ay + (fclaw_opt->by - fclaw_opt->ay) * y1;
    };

#if 0
    // create neumann function
    IsNeumannFunc<2> inf = [&](Side<2> s, const array<double, 2> &lower,
                               const array<double, 2> &upper) 
    {
        return te_opt->boundary_conditions[s.getIndex()] == 2;
    };
#endif    

    // generates levels of patches for GMG;  2 layers of ghost cells    
#if 0    
    P4estDomGen domain_gen(wrap->p4est, ns, clawpatch_opt->mbc, inf, bmf);
#endif    

    P4estDomainGenerator domain_gen(wrap->p4est, ns, clawpatch_opt->mbc, bmf);


    // get finest level
#if 0
    shared_ptr<Domain<2>> te_domain = domain_gen.getFinestDomain();
#endif
    Domain<2> te_domain = domain_gen.getFinestDomain();


    /* Store q = (h,hu,hv) at time level n */
#if 0
    shared_ptr<Vector<2>> state_vec = make_shared<fc2d_thunderegg_vector>(glob,
                                                                          STORE_STATE,
                                                                          clawpatch_opt->meqn);    
#endif       
    Vector<2> state_vec = fc2d_thunderegg_get_vector(glob,STORE_STATE);


    // ghost filler
#if 0
    auto ghost_filler = make_shared<BiLinearGhostFiller>(te_domain);
#endif

    BiLinearGhostFiller ghost_filler(te_domain, fill_type);


    // patch operator
#if 0    
    auto op = make_shared<sgn>(glob,state_vec,te_domain,ghost_filler);
#endif    

    //phasefield op(glob,beta_vec,te_domain,ghost_filler);
    sgn op(glob,state_vec,te_domain,ghost_filler);


    // set the patch solver
#if 0    
    shared_ptr<PatchSolver<2>>  solver;
    solver = make_shared<BiCGStabPatchSolver<2>>(op,
                                                 te_opt->patch_bcgs_tol,
                                                 te_opt->patch_bcgs_max_it,
                                                 false);
#endif 

    Iterative::CG<2> patch_cg;
    patch_cg.setTolerance(te_opt->patch_iter_tol);
    patch_cg.setMaxIterations(te_opt->patch_iter_max_it);

    Iterative::BiCGStab<2> patch_bicg;
    patch_bicg.setTolerance(te_opt->patch_iter_tol);
    patch_bicg.setMaxIterations(te_opt->patch_iter_max_it);

    Iterative::Solver<2>* patch_iterative_solver = nullptr;
    switch(te_opt->patch_solver){
        case CG:
            patch_iterative_solver = &patch_cg;
            break; 
        case BICG:
            patch_iterative_solver = &patch_bicg;
            break;
        default:
            fclaw_global_essentialf("phasefield : No valid " \
                                    "patch solver specified\n");
            exit(0);            
    }
    Iterative::PatchSolver<2> solver(*patch_iterative_solver,op,true);

    // create matrix
    //shared_ptr<Operator<2>> A = op;

    // create gmg preconditioner
    shared_ptr<Operator<2>> M;

    if (te_opt->mg_prec && domain_gen.hasCoarserDomain())
    {
        // options
        GMG::CycleOpts copts;
        //copts.max_levels = te_opt->max_levels;
        //copts.patches_per_proc = te_opt->patches_per_proc;
        copts.pre_sweeps = te_opt->pre_sweeps;
        copts.post_sweeps = te_opt->post_sweeps;
        copts.mid_sweeps = te_opt->mid_sweeps;
        copts.coarse_sweeps = te_opt->coarse_sweeps;
        copts.cycle_type = te_opt->cycle_type;

        //GMG cycle builder
        GMG::CycleBuilder<2> builder(copts);
        
        //add finest level

        //next domain
        auto curr_domain = te_domain;
        auto next_domain = domain_gen.getCoarserDomain();


        GMG::LinearRestrictor<2> restrictor(curr_domain, 
                                            next_domain); 

#if 0
        //operator
        auto patch_operator = op;
#endif        

        //smoother
#if 0        
        shared_ptr<GMG::Smoother<2>> smoother = solver;
#endif

#if 0        
        Iterative::PatchSolver<2> smoother(*patch_iterative_solver, 
                                           coarse_patch_operator, true);


        //restrictor
        auto restrictor = make_shared<GMG::LinearRestrictor<2>>(curr_domain, 
                                                                next_domain, 
                                                                clawpatch_opt->rhs_fields);


        //vector generator
        auto vg = make_shared<ValVectorGenerator<2>>(curr_domain, 
                                                     clawpatch_opt->rhs_fields);

        builder.addFinestLevel(patch_operator, smoother, restrictor, vg);
#endif              

        Vector<2> vg(curr_domain,clawpatch_opt->rhs_fields);

        builder.addFinestLevel(op, solver, restrictor);

        //add intermediate levels
        auto prev_q_n_vec = state_vec;
        auto prev_domain = curr_domain;
        curr_domain = next_domain;
        while(domain_gen.hasCoarserDomain())
        {
            next_domain = domain_gen.getCoarserDomain();

            //operator
#if 0
            auto ghost_filler = make_shared<BiLinearGhostFiller>(curr_domain);
            auto restricted_q_n_vec = restrict_q_n_vec(prev_q_n_vec, 
                                                           prev_domain, curr_domain);
            patch_operator = make_shared<sgn>(glob,restricted_q_n_vec,curr_domain, ghost_filler);
            prev_q_n_vec = restricted_q_n_vec;
#endif            
            BiLinearGhostFiller ghost_filler(curr_domain, fill_type);
            Vector<2> restricted_q_n_vec = restrict_q_n_vec(prev_q_n_vec, 
                                                              prev_domain, curr_domain);

            sgn patch_operator(glob,restricted_q_n_vec,curr_domain, 
                               ghost_filler);

            prev_q_n_vec = restricted_q_n_vec;

            //smoother
#if 0
            shared_ptr<GMG::Smoother<2>> smoother;
            smoother = make_shared<BiCGStabPatchSolver<2>>(patch_operator,
                                                           te_opt->patch_bcgs_tol,
                                                           te_opt->patch_bcgs_max_it,
                                                           continue_on_breakdown);
#endif
            Iterative::PatchSolver<2> smoother(*patch_iterative_solver, 
                                               patch_operator, true);

            //restrictor
#if 0            
            auto restrictor = make_shared<GMG::LinearRestrictor<2>>(curr_domain, 
                                                                    next_domain, 
                                                                    clawpatch_opt->rhs_fields);
#endif 
            GMG::LinearRestrictor<2> restrictor(curr_domain, 
                                                next_domain);


            //interpolator
#if 0            
            auto interpolator = make_shared<GMG::DirectInterpolator<2>>(curr_domain, 
                                                                        prev_domain, 
                                                                        clawpatch_opt->rhs_fields);
#endif
            GMG::DirectInterpolator<2> interpolator(curr_domain, 
                                                    prev_domain);

            //vector generator            
#if 0            
            vg = make_shared<ValVectorGenerator<2>>(curr_domain, clawpatch_opt->rhs_fields);
#endif

            Vector<2> vg(curr_domain,clawpatch_opt->rhs_fields);

#if 0
            builder.addIntermediateLevel(patch_operator, smoother, restrictor, 
                                         interpolator, vg);
#endif
            builder.addIntermediateLevel(patch_operator, smoother, restrictor, 
                                         interpolator);


            prev_domain = curr_domain;
            curr_domain = next_domain;
        }

        //add coarsest level

        //operator
#if 0        
        auto ghost_filler = make_shared<BiLinearGhostFiller>(curr_domain);
        auto restricted_q_n_vec = restrict_q_n_vec(prev_q_n_vec, prev_domain, curr_domain);
        patch_operator = make_shared<sgn>(glob,restricted_q_n_vec, curr_domain, ghost_filler);
#endif
        BiLinearGhostFiller ghost_filler(curr_domain, fill_type);
        Vector<2> restricted_q_n_vec = restrict_q_n_vec(prev_q_n_vec, 
                                                        prev_domain, 
                                                        curr_domain);

        sgn coarse_patch_operator(glob,restricted_q_n_vec, curr_domain, 
                                  ghost_filler);


        //smoother
#if 0
        smoother = make_shared<BiCGStabPatchSolver<2>>(patch_operator,
                                                       te_opt->patch_bcgs_tol,
                                                       te_opt->patch_bcgs_max_it,
                                                       continue_on_breakdown);
#endif
        Iterative::PatchSolver<2> smoother(*patch_iterative_solver, 
                                           coarse_patch_operator, true);

        //interpolator
#if 0        
        auto interpolator = make_shared<GMG::DirectInterpolator<2>>(curr_domain, prev_domain, clawpatch_opt->rhs_fields);
#endif 
        GMG::DirectInterpolator<2> interpolator(curr_domain, prev_domain);


        //vector generator
#if 0        
        vg = make_shared<ValVectorGenerator<2>>(curr_domain, clawpatch_opt->rhs_fields);
#endif

        // Vector<2> vg(curr_domain,clawpatch_opt->rhs_fields);

#if 0
        builder.addCoarsestLevel(patch_operator, smoother, interpolator, vg);
#endif        

        builder.addCoarsestLevel(coarse_patch_operator, smoother, interpolator);

        M = builder.getCycle();
    }

    // solve
#if 0    
    auto vg = make_shared<ValVectorGenerator<2>>(te_domain, clawpatch_opt->rhs_fields);
#endif
    Vector<2> vg(te_domain, clawpatch_opt->rhs_fields);

#if 0
    // Set starting conditions
    printf("sgn_operator : Need to set an initial condition.");
    exit(0);
    shared_ptr<Vector<2>> u = make_shared<fc2d_thunderegg_vector>(glob,SOLN);
#else
#if 0    
    shared_ptr<Vector<2>> D = vg->getNewVector();
#endif
    Vector<2> D = vg.getZeroClone();
#endif    

#if 0
    int its = Iterative::BiCGStab<2>::solve(vg, A, D, f, M, te_opt->max_it, te_opt->tol, 
                                 nullptr, //no timer
                                 output); //output iteration information to cout
#endif

    bool prt_output = glob->mpirank == 0;
    
    Iterative::BiCGStab<2> iter_solver;
    int its = iter_solver.solve(op, D, f, M.get(), prt_output);


    fclaw_global_productionf("Iterations: %i\n", its);

    /* Solution is copied to right hand side */
    f = D;

#if 0    
    fclaw_global_productionf("f-2norm:   %24.16f\n", f->twoNorm());
    fclaw_global_productionf("f-infnorm: %24.16f\n", f->infNorm());
    fclaw_global_productionf("u-2norm:   %24.16f\n", u->twoNorm());
    fclaw_global_productionf("u-infnorm: %24.16f\n\n", u->infNorm());

    // copy solution into rhs
    fclaw_global_productionf("Checking if copy function works:\n");
    fclaw_global_productionf("fcopy-2norm:   %24.16f\n", f->twoNorm());
    fclaw_global_productionf("fcopy-infnorm: %24.16f\n\n", f->infNorm());
#endif    
}

