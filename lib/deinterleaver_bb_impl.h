#ifndef INCLUDED_DRM_DEINTERLEAVER_BB_IMPL_H
#define INCLUDED_DRM_DEINTERLEAVER_BB_IMPL_H
#include <drm/deinterleaver_bb.h>
namespace gr { namespace drm {
class deinterleaver_bb_impl : public deinterleaver_bb
{
    std::vector<int> d_seq;
public:
    explicit deinterleaver_bb_impl(const std::vector<int>&);
    int work(int, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
