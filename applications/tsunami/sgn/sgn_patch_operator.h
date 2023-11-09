#include "sgn_options.h"

#include <ThunderEgg.h>
#include "fc2d_thunderegg_options.h"

class sgn : public ThunderEgg::PatchOperator<2>
{
    private:

    public:
#if 0        
    std::shared_ptr<const ThunderEgg::Vector<2>> q_n;
#endif
    ThunderEgg::Vector<2> q_n;

    const sgn_options_t *sgn_opt;

#if 0
    sgn(fclaw2d_global_t *glob,
               std::shared_ptr<const ThunderEgg::Vector<2>> q_n_in,
               std::shared_ptr<const ThunderEgg::Domain<2>> domain,
               std::shared_ptr<const ThunderEgg::GhostFiller<2>> ghost_filler);
#endif

    sgn(fclaw2d_global_t *glob,
        const ThunderEgg::Vector<2>& phi_n_in,
        const ThunderEgg::Domain<2>& domain,
        const ThunderEgg::GhostFiller<2>& ghost_filler);



#if 0
    sgn(const fc2d_thunderegg_options *mg_opt,const sgn_options_t* sgn_opt,
               std::shared_ptr<const ThunderEgg::Vector<2>> q_n_in,
               std::shared_ptr<const ThunderEgg::Domain<2>> domain,
               std::shared_ptr<const ThunderEgg::GhostFiller<2>> ghost_filler);
#endif               

    sgn(const fc2d_thunderegg_options *te_opt,const sgn_options* sgn_opt,
               const ThunderEgg::Vector<2>& phi_n_in,
               const ThunderEgg::Domain<2>& domain,
               const ThunderEgg::GhostFiller<2>& ghost_filler);


    sgn* clone() const override;

    void applySinglePatch(const ThunderEgg::PatchInfo<2>& pinfo,
                          const ThunderEgg::PatchView<const double, 2>& us,
                          const ThunderEgg::PatchView<double, 2>& fs) const override;

#if 0
    void applySinglePatch(std::shared_ptr<const ThunderEgg::PatchInfo<2>> pinfo,
                          const std::vector<ThunderEgg::LocalData<2>> &us,
                          std::vector<ThunderEgg::LocalData<2>> &fs,
                          bool interior_dirichlet) const override;


    void addGhostToRHS(std::shared_ptr<const ThunderEgg::PatchInfo<2>> pinfo,
                       const std::vector<ThunderEgg::LocalData<2>> &us,
                       std::vector<ThunderEgg::LocalData<2>> &fs) const override;


#endif

    void applySinglePatchWithInternalBoundaryConditions(const ThunderEgg::PatchInfo<2>& pinfo,
                                                        const ThunderEgg::PatchView<const double, 2>& u,
                                                        const ThunderEgg::PatchView<double, 2>& f) const override;

    void modifyRHSForInternalBoundaryConditions(const ThunderEgg::PatchInfo<2> &pinfo,
                                                const ThunderEgg::PatchView<const double, 2> &u,
                                                const ThunderEgg::PatchView<double, 2> &f) const override;



    int s[4]; /* Determines sign when applying BCs */

    const int * getS() const;
};