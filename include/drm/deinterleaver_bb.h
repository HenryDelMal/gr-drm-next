/* -*- c++ -*- */
#ifndef INCLUDED_DRM_DEINTERLEAVER_BB_H
#define INCLUDED_DRM_DEINTERLEAVER_BB_H
#include <drm/api.h>
#include <gnuradio/sync_block.h>
#include <vector>
namespace gr { namespace drm {
class DRM_API deinterleaver_bb : virtual public gr::sync_block
{
public:
    using sptr = std::shared_ptr<deinterleaver_bb>;
    static sptr make(const std::vector<int>& sequence);
};
} }
#endif
