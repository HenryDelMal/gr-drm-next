/* -*- c++ -*- */
#ifndef INCLUDED_DRM_AUDIO_DECODER_SB_IMPL_H
#define INCLUDED_DRM_AUDIO_DECODER_SB_IMPL_H

#include <drm/audio_decoder_sb.h>
#include <neaacdec.h>
#include <vector>

namespace gr { namespace drm {
class audio_decoder_sb_impl : public audio_decoder_sb
{
    transm_params* d_tp;
    NeAACDecHandle d_decoder;
    unsigned int d_l_mux, d_n_aac_frames, d_n_header_bytes, d_sample_rate;
    std::vector<float> d_pending;
    static unsigned int read_bits(const unsigned char*, unsigned int&, unsigned int);
    static void bits_to_bytes(const unsigned char*, unsigned int, std::vector<unsigned char>&);
    void decode_superframe(const unsigned char*);
public:
    explicit audio_decoder_sb_impl(transm_params*);
    ~audio_decoder_sb_impl() override;
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
