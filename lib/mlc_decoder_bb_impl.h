#ifndef INCLUDED_DRM_MLC_DECODER_BB_IMPL_H
#define INCLUDED_DRM_MLC_DECODER_BB_IMPL_H
#include <drm/mlc_decoder_bb.h>
#include <vector>
namespace gr { namespace drm {
class mlc_decoder_bb_impl : public mlc_decoder_bb
{
    transm_params* d_tp;
    std::string d_channel;
    int d_levels, d_cells, d_output_bits;
    std::vector<int> d_partition_lengths;
    std::vector<std::vector<unsigned char>> d_patterns, d_tail_patterns;
    static unsigned char parity(unsigned int value);
    std::vector<unsigned char> decode_level(const unsigned char*, int,
                                            const std::vector<unsigned char>&,
                                            const std::vector<unsigned char>&) const;
public:
    mlc_decoder_bb_impl(transm_params*, const std::string&);
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
