/* -*- c++ -*- */
#ifndef INCLUDED_DRM_QAM_DEMAPPER_CB_IMPL_H
#define INCLUDED_DRM_QAM_DEMAPPER_CB_IMPL_H
#include <drm/qam_demapper_cb.h>

namespace gr { namespace drm {
class qam_demapper_cb_impl : public qam_demapper_cb
{
    float d_map[8][2];
    int d_rows, d_outputs;
public:
    qam_demapper_cb_impl(const float map_table[][2], int bits_per_symbol, int n_outputs);
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
