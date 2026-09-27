#ifndef INCLUDED_DRM_DEINTERLEAVER_CC_IMPL_H
#define INCLUDED_DRM_DEINTERLEAVER_CC_IMPL_H
#include <drm/deinterleaver_cc.h>
#include <deque>
namespace gr { namespace drm {
class deinterleaver_cc_impl : public deinterleaver_cc
{
    std::vector<int> d_seq;
    bool d_long;
    int d_depth;
    std::deque<std::vector<gr_complex>> d_buffer;
public:
    deinterleaver_cc_impl(const std::vector<int>&, bool, int);
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
