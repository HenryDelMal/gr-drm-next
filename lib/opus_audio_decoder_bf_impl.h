/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OPUS_AUDIO_DECODER_BF_IMPL_H
#define INCLUDED_DRM_OPUS_AUDIO_DECODER_BF_IMPL_H

#include <drm/opus_audio_decoder_bf.h>
#include <opus/opus.h>
#include <vector>

namespace gr { namespace drm {

class opus_audio_decoder_bf_impl : public opus_audio_decoder_bf
{
    static constexpr unsigned int frames_per_superframe = 20;
    OpusDecoder* d_decoder;
    unsigned int d_l_mux;
    unsigned int d_sample_rate;
    unsigned int d_frame_samples;
    std::vector<float> d_pending;

    static unsigned int read_bits(const unsigned char* bits, unsigned int& pos,
                                  unsigned int count);
    static unsigned char crc8(const unsigned char* data, unsigned int size);
    void decode_superframe(const unsigned char* bits);

public:
    explicit opus_audio_decoder_bf_impl(transm_params* tp);
    ~opus_audio_decoder_bf_impl() override;
    void forecast(int noutput_items, gr_vector_int& required) override;
    int general_work(int noutput_items, gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;
};

} }
#endif
