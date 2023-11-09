
#include "sgn_options.h"
#include "sgn_patch_operator.h"

#include <ThunderEgg/Vector.h>

using namespace ThunderEgg;

sgn::sgn(fclaw2d_global_t *glob,
         const Vector<2> q_n,
         const Domain<2> domain,
         const GhostFiller<2> ghost_filler) 
    : sgn(fc2d_thunderegg_get_options(glob),sgn_get_options(glob),q_n,domain,ghost_filler) 
{
    //this just calls the other constructor
}
sgn::sgn(const  *mg_opt,const sgn_options* sgn_opt,
               const Vector<2> q_n_in,
               const Domain<2> domain,
               const GhostFiller<2> ghost_filler) 
    : PatchOperator<2>(domain,ghost_filler),
      q_n(q_n_in),
      sgn_opt(sgn_opt)

{
    /* Get scale needed to apply homogeneous boundary conditions. 
       For Dirichlet (bctype=1) :   scalar is -1 
       For Neumann (bctype=2)   :   scalar is 1 */
    for(int m = 0; m < 4; m++)
    {
        s[m] = 2*mg_opt->boundary_conditions[m] - 3;
    }
}

#if 0
void sgn::applySinglePatch(std::shared_ptr<const PatchInfo<2>> pinfo, 
                                 const std::vector<LocalData<2>>& us,
                                 std::vector<LocalData<2>>& fs,
                                 bool interior_dirichlet) const 
#endif

void applySinglePatch(const ThunderEgg::PatchInfo<2>& pinfo,
                      const ThunderEgg::PatchView<const double, 2>& us,
                      const ThunderEgg::PatchView<double, 2>& fs) const override

{
    //const cast since u ghost values have to be modified
    //ThunderEgg doesn't care if ghost values are modified, just don't modify the interior values.

    int mfields = us.size();
    int mx = pinfo->ns[0]; 
    int my = pinfo->ns[1];

    /* Apply boundary conditions */
    for(int m = 0; m < mfields; m++)
    {
        // LocalData<2>& D = const_cast<LocalData<2>&>(us[m]);

        if (!pinfo->hasNbr(Side<2>::west()))
        {
            /* Physical boundary */
            auto ghosts = us.getGhostSliceOn(Side<2>::west(),{0});
            for(int j = 0; j < my; j++)
                ghosts[{j}] = s[0]*D[{0,j}];
        }
        else if (interior_dirichlet)
        {
            auto ghosts = us.getGhostSliceOn(Side<2>::west(),{0});
            // auto ghosts = D.getGhostSliceOnSide(Side<2>::west(),1);
            for(int j = 0; j < my; j++)
                ghosts[{j}] = -D[{0,j}];
        }

        if (!pinfo->hasNbr(Side<2>::east()))
        {
            /* Physical boundary */
            auto ghosts = us.getGhostSliceOn(Side<2>::east(),{0});
            // auto ghosts = D.getGhostSliceOnSide(Side<2>::east(),1);
            for(int j = 0; j < my; j++)
                ghosts[{j}] = s[1]*D[{mx-1,j}];            
        } 
        else if (interior_dirichlet)
        {
            auto ghosts = us.getGhostSliceOn(Side<2>::east(),{0});
            // auto ghosts = D.getGhostSliceOnSide(Side<2>::east(),1);
            for(int j = 0; j < my; j++)
                ghosts[{j}] = -D[{mx-1,j}];
        }

        if (!pinfo->hasNbr(Side<2>::south()))
        {
            /* Physical boundary */
            auto ghosts = us.getGhostSliceOn(Side<2>::south(),{0});
            // auto ghosts = D.getGhostSliceOnSide(Side<2>::south(),1);
            for(int i = 0; i < mx; i++)
                ghosts[{i}] = s[2]*D[{i,0}];
        }
        else if (interior_dirichlet)
        {
            auto ghosts = us.getGhostSliceOn(Side<2>::south(),{0});
            // auto ghosts = D.getGhostSliceOnSide(Side<2>::south(),1);
            for(int i = 0; i < mx; i++)
                ghosts[{i}] = -D[{i,0}];
        }

        if (!pinfo->hasNbr(Side<2>::north()))
        {
            /* Physical boundary */
            auto ghosts = D.getGhostSliceOnSide(Side<2>::north(),1);
            for(int i = 0; i < mx; i++)
                ghosts[{i}] = s[3]*D[{i,my-1}];
        }
        else if (interior_dirichlet)
        {
            auto ghosts = us.getGhostSliceOn(Side<2>::north(),{0});
            // auto ghosts = D.getGhostSliceOnSide(Side<2>::north(),1);
            for(int i = 0; i < mx; i++)
                ghosts[{i}] = -D[{i,my-1}];
        }
    }

    /* Five-point Laplacian - not anisotropic yet */
    ComponentView<const double,2> u = us.getComponentView(0);
    ComponentView<double,2> Au = fs.getComponentView(0);

    ComponentView<const double,2> phi = us.getComponentView(1);
    ComponentView<double,2> Aphi = fs.getComponentView(1);

#if 0
    LocalData<2>& Dx = const_cast<LocalData<2>&>(us[0]);
    LocalData<2>& Fx = fs[0];

    LocalData<2>& Dy = const_cast<LocalData<2>&>(us[1]);
    LocalData<2>& Fy = fs[1];
#endif    

    double dx = pinfo->spacings[0];
    double dy = pinfo->spacings[1];
    double dxsq = dx*dx;
    double dysq = dy*dy;
    double dx2 = 2*dx;
    double dy2 = 2*dy;
    double dxdy4 = 4*dx*dy;

    /* We need to discretize this (from Basilisk.fr)

    res.x[] = b.x[] -
      (-alpha_d/3.*(hr3*D.x[1] + hl3*D.x[-1] - 
            (hr3 + hl3)*D.x[])/sq(Delta) +
       hc*(alpha_d*(dxeta*dxzb + hc/2.*d2x(zb)) + 1.)*D.x[] +
       alpha_d*hc*((hc/2.*d2xy(zb) + dxeta*dy(zb))*D.y[] + 
               hc/2.*dy(zb)*dx(D.y) - sq(hc)/3.*d2xy(D.y)
               - hc*dy(D.y)*(dxh + dxzb/2.)));

    Simplfied : 
       -alpha_d/3.*(  hr3*D_{i+1,j} + hl3*D_{i-1,j} - (hr3 + hl3)*D{i,j}  )/sq(Delta)
                     + hc*D_{i,j}
       + alpha_d*hc*(  -sq(hc)/3.*d2xy(D.y) - hc*dy(D.y)*(dxh)  );
    */

    double alpha = sgn_opt->alpha;
    double a3 = alpha/3;


    /* Get local view into q_n (component 0) */
    ComponentView<const double,2> h = q_n.getComponentView(0, pinfo.local_index);
    //LocalData<2> h = q_n->getLocalData(0,pinfo->local_index);

    for(int i = 0; i < mx; i++)
    {
        for(int j = 0; j < my; j++)
        {
            double hc = h[{i,j}];
            if (hc <= 0)
            {
                fclaw_global_essentialf("sgn_patch_operator : h(i,j) = 0;  %5d %5d %12.4e\n",
                                        i,j,hc);
                exit(0);
            }

            double hc2 = hc*hc;
            double hc3 = hc2*hc;

            {
                double hl = h[{i-1,j}];
                double hr = h[{i+1,j}];
                /* Compute h^3 at edges */
                double hl3 = pow((hl + hc)/2.0,3);
                double hr3 = pow((hr + hc)/2.0,3);

                double dxh = (hr - hl)/dx2;
                double dxh3Dx = (hl3*Dx[{i-1,j}] - (hl3+hr3)*Dx[{i,j}] + hr3*Dx[{i+1,j}])/dxsq;
                double dxyDy = ((Dy[{i+1,j+1}] - Dy[{i+1,j-1}]) - 
                            (Dy[{i-1,j+1}] - Dy[{i-1,j-1}]))/dxdy4;

                double dyDy = (Dy[{i,j+1}] - Dy[{i,j-1}])/dy2;            

#if 0
                Fx[{i,j}]   = -a3*dxh3Dx + hc*Dx[{i,j}] - 
                               alpha*hc*(hc2*dxyDy/3.0 + hc*dxh*dyDy);
#endif                               
                Fx[{i,j}]   = -a3*dxh3Dx + hc*Dx[{i,j}] - a3*hc3*dxyDy + alpha*hc2*dxh*dyDy;


            }

            if (0)
            {
                /* Real 2d */
                double hu = h[{i,j+1}];
                double hd = h[{i,j-1}];

                double hu3 = pow((hu + hc)/2.0,3);
                double hd3 = pow((hd + hc)/2.0,3);

                double dyh = (hu - hd)/dy2;     

                /* Operator without bathymetry */
                double dyh3Dy = (hd3*Dy[{i,j-1}] - (hu3+hd3)*Dy[{i,j}] + hu3*Dy[{i,j+1}])/dysq;

                double dxyDx = ((Dx[{i+1,j+1}] - Dx[{i+1,j-1}]) - 
                                (Dx[{i-1,j+1}] - Dx[{i-1,j-1}]))/dxdy4;

                double dxDx = (Dx[{i+1,j}] - Dx[{i-1,j}])/dx2;

#if 0
                Fy[{i,j}]   = -a3*dyh3Dy + hc*Dy[{i,j}] - 
                               alpha*hc*(hc*dyh*dxDx + hc2*dxyDx/3.0);
#endif                               
                Fy[{i,j}]   = -a3*dyh3Dy + hc*Dy[{i,j}] - alpha*hc2*dyh*dxDx + a3*hc3*dxyDx;

            }
            else
            {
                /* pseudo-1d */
                Fy[{i,j}] = Dy[{i,j}];                
            }

        }
    }
}

#if 0
void sgn::addGhostToRHS(std::shared_ptr<const PatchInfo<2>> pinfo, 
                              const std::vector<LocalData<2>>& us, 
                              std::vector<LocalData<2>>& Aus) const
#endif
void sgn::addGhostToRHS(const PatchInfo<2>& pinfo, 
                        const PatchView<const double,2>& us, 
                        const PatchView<double,2>& Aus) const                                
{
    int mfields = us.size();
    int mx = pinfo->ns[0]; 
    int my = pinfo->ns[1];


    ValVector<2> new_u(MPI_COMM_WORLD,pinfo->ns,1,us.size(),1);
    ValVector<2> new_Au(MPI_COMM_WORLD,pinfo->ns,1,us.size(),1);
    auto new_us = new_u.getLocalDatas(0);
    auto new_Aus = new_Au.getLocalDatas(0);
    if(pinfo->hasNbr(Side<2>::west())){
        for(int field=0; field<mfields; field++){
            for(int j=0; j < my; j++){
                new_us[field][{-1,j}]=us[field][{-1,j}]+us[field][{0,j}];
            }
        }
    }
    if(pinfo->hasNbr(Side<2>::east())){
        for(int field=0; field<mfields; field++){
            for(int j=0; j < my; j++){
                new_us[field][{my,j}]=us[field][{my,j}]+us[field][{my-1,j}];
            }
        }
    }
    if(pinfo->hasNbr(Side<2>::south())){
        for(int field=0; field<mfields; field++){
            for(int i=0; i < mx; i++){
                new_us[field][{i,-1}]=us[field][{i,-1}]+us[field][{i,0}];
            }
        }
    }
    if(pinfo->hasNbr(Side<2>::north())){
        for(int field=0; field<mfields; field++){
            for(int i=0; i < mx; i++){
                new_us[field][{i,mx}]=us[field][{i,mx}]+us[field][{i,mx-1}];
            }
        }
    }
    
    /* Call patch operator to set boundary conditions correctly */
    applySinglePatch(pinfo,new_us,new_Aus,false);


    //modify rhs
    for(int field=0; field<mfields; field++){
        for(int j=0; j < my; j++){
            for(int i=0; i < mx; i++){
                Aus[field][{i,j}]-=new_Aus[field][{i,j}];
            }
        }
    }
}

const int *sgn::getS() const{
    return s;
}