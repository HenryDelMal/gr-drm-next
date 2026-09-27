/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OPUS_AUDIO_ENCODER_FB_IMPL_H
#define INCLUDED_DRM_OPUS_AUDIO_ENCODER_FB_IMPL_H

#include <drm/opus_audio_encoder_fb.h>
#include <opus/opus.h>

namespace gr { namespace drm {

class opus_audio_encoder_fb_impl : public opus_audio_encoder_fb
{
    static constexpr unsigned int frames_per_superframe = 20;
    OpusEncoder* d_encoder;
    unsigned int d_l_mux;
    unsigned int d_sample_rate;
    unsigned int d_frame_samples;
    unsigned int d_superframe_samples;
    unsigned int d_packet_bytes;

    static unsigned char crc8(const unsigned char* data, unsigned int size);
    static void write_bits(unsigned char* bits, unsigned int& pos,
                           unsigned int value, unsigned int count);

public:
    opus_audio_encoder_fb_impl(transm_params* tp,
                               int bitrate,
                               bool vbr,
                               const std::string& application,
                               const std::string& signal,
                               const std::string& bandwidth,
                               int complexity,
                               bool dtx);
    ~opus_audio_encoder_fb_impl() override;
    void forecast(int noutput_items, gr_vector_int& required) override;
    int general_work(int noutput_items, gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;
};

} }
#endif
