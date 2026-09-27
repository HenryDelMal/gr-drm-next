#ifndef INCLUDED_DRM_PILOT_EQUALIZER_VCC_IMPL_H
#define INCLUDED_DRM_PILOT_EQUALIZER_VCC_IMPL_H
#include <drm/pilot_equalizer_vcc.h>
#include "drm_tables.h"
namespace gr { namespace drm {
class pilot_equalizer_vcc_impl : public pilot_equalizer_vcc
{
    tables* d_tables;
    int d_nfft, d_ns, d_rm, d_symbol;
    bool overlaps_reference(int symbol, int carrier) const;
public:
    explicit pilot_equalizer_vcc_impl(transm_params* tp);
    int work(int, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
