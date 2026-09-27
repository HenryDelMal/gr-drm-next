/* -*- c++ -*- */
#ifndef INCLUDED_DRM_CELL_DEMAPPING_CC_IMPL_H
#define INCLUDED_DRM_CELL_DEMAPPING_CC_IMPL_H

#include <drm/cell_demapping_cc.h>
#include <vector>

namespace gr { namespace drm {
class cell_demapping_cc_impl : public cell_demapping_cc
{
    transm_params* d_tp;
    tables* d_tables;
    unsigned int d_nfft, d_ns, d_mtf, d_nmsc, d_nsdc, d_nfac;
    int d_rm, d_kmin, d_kmax, d_n_sdc_sym;
    std::vector<int> d_unused;
    std::vector<unsigned char> build_reserved_mask() const;
    bool used_carrier(int k) const;
public:
    explicit cell_demapping_cc_impl(transm_params* tp);
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
