/* -*- c++ -*- */
#ifndef INCLUDED_DRM_DEINTERLEAVER_CC_H
#define INCLUDED_DRM_DEINTERLEAVER_CC_H
#include <drm/api.h>
#include <gnuradio/block.h>
#include <vector>
namespace gr { namespace drm {
class DRM_API deinterleaver_cc : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<deinterleaver_cc>;
    static sptr make(const std::vector<int>& sequence, bool long_interleaving, int depth);
};
} }
#endif
