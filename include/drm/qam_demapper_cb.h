/* -*- c++ -*- */
#ifndef INCLUDED_DRM_QAM_DEMAPPER_CB_H
#define INCLUDED_DRM_QAM_DEMAPPER_CB_H

#include <drm/api.h>
#include <gnuradio/block.h>

namespace gr { namespace drm {
class DRM_API qam_demapper_cb : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<qam_demapper_cb>;
    static sptr make(const float map_table[][2], int bits_per_symbol, int n_outputs);
};
} }
#endif
