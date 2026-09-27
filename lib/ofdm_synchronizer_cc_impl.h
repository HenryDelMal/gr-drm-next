#ifndef INCLUDED_DRM_OFDM_SYNCHRONIZER_CC_IMPL_H
#define INCLUDED_DRM_OFDM_SYNCHRONIZER_CC_IMPL_H
#include <drm/ofdm_synchronizer_cc.h>
namespace gr { namespace drm {
class ofdm_synchronizer_cc_impl : public ofdm_synchronizer_cc
{
    unsigned int d_nfft, d_ncp, d_symbol_len;
    bool d_locked;
    double d_frequency, d_phase;
    double cp_metric(const gr_complex*, unsigned int) const;
public:
    ofdm_synchronizer_cc_impl(unsigned int, unsigned int);
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
