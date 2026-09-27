/* -*- c++ -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "opus_audio_encoder_fb_impl.h"
#include <gnuradio/io_signature.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace gr { namespace drm {

opus_audio_encoder_fb::sptr opus_audio_encoder_fb::make(transm_params* tp)
{
    return gnuradio::make_block_sptr<opus_audio_encoder_fb_impl>(tp);
}

opus_audio_encoder_fb_impl::opus_audio_encoder_fb_impl(transm_params* tp)
    : gr::block("opus_audio_encoder_fb",
                gr::io_signature::make(1, 1, sizeof(float)),
                gr::io_signature::make(1, 1, sizeof(unsigned char))),
      d_encoder(nullptr), d_l_mux(tp->msc().L_MUX()),
      d_sample_rate(tp->cfg().audio_samp_rate()), d_frame_samples(0),
      d_superframe_samples(0), d_packet_bytes(0)
{
    if (d_sample_rate != 12000 && d_sample_rate != 24000 && d_sample_rate != 48000)
        throw std::invalid_argument("Opus encoder supports 12, 24, or 48 kHz audio");

    d_frame_samples = d_sample_rate / 50; // 20 ms
    d_superframe_samples = d_frame_samples * frames_per_superframe;

    constexpr unsigned int header_bits = frames_per_superframe * 12;
    constexpr unsigned int crc_bits = frames_per_superframe * 8;
    if (d_l_mux <= header_bits + crc_bits)
        throw std::invalid_argument("MSC multiplex is too small for an Opus superframe");
    d_packet_bytes = ((d_l_mux - header_bits - crc_bits) / 8) /
                     frames_per_superframe;
    if (d_packet_bytes == 0 || d_packet_bytes * frames_per_superframe > 4095)
        throw std::invalid_argument("MSC multiplex cannot represent Opus packet borders");

    int error = OPUS_OK;
    d_encoder = opus_encoder_create(d_sample_rate, 1, OPUS_APPLICATION_AUDIO, &error);
    if (!d_encoder || error != OPUS_OK)
        throw std::runtime_error(std::string("Opus encoder initialization failed: ") +
                                 opus_strerror(error));

    const int bitrate = static_cast<int>(d_packet_bytes * 8 * 50);
    if (opus_encoder_ctl(d_encoder, OPUS_SET_BITRATE(bitrate)) != OPUS_OK ||
        opus_encoder_ctl(d_encoder, OPUS_SET_VBR(0)) != OPUS_OK ||
        opus_encoder_ctl(d_encoder, OPUS_SET_DTX(0)) != OPUS_OK)
        throw std::runtime_error("Opus encoder configuration failed");

    set_output_multiple(d_l_mux);
}

opus_audio_encoder_fb_impl::~opus_audio_encoder_fb_impl()
{
    if (d_encoder) opus_encoder_destroy(d_encoder);
}

unsigned char opus_audio_encoder_fb_impl::crc8(const unsigned char* data,
                                                unsigned int size)
{
    // DReaM's degree-8 CCRC calculation (x^8 + x^4 + x^3 + x^2 + 1).
    uint32_t state = ~uint32_t(0);
    for (unsigned int byte = 0; byte < size; ++byte) {
        for (unsigned int bit = 0; bit < 8; ++bit) {
            state <<= 1;
            if (state & (1U << 8)) state |= 1U;
            if (data[byte] & (1U << (7 - bit))) state ^= 1U;
            if (state & 1U) state ^= 0x1cU;
        }
    }
    return static_cast<unsigned char>((~state) & 0xffU);
}

void opus_audio_encoder_fb_impl::write_bits(unsigned char* bits, unsigned int& pos,
                                             unsigned int value, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
        bits[pos++] = (value >> (count - i - 1)) & 1U;
}

void opus_audio_encoder_fb_impl::forecast(int noutput_items, gr_vector_int& required)
{
    required[0] = noutput_items > 0 ? d_superframe_samples : 0;
}

int opus_audio_encoder_fb_impl::general_work(int noutput_items,
                                              gr_vector_int& ninput_items,
                                              gr_vector_const_void_star& input_items,
                                              gr_vector_void_star& output_items)
{
    if (noutput_items < static_cast<int>(d_l_mux) ||
        ninput_items[0] < static_cast<int>(d_superframe_samples))
        return 0;

    const auto* input = static_cast<const float*>(input_items[0]);
    auto* output = static_cast<unsigned char*>(output_items[0]);
    std::memset(output, 0, d_l_mux);

    std::vector<std::vector<unsigned char>> packets(frames_per_superframe,
                                                     std::vector<unsigned char>(d_packet_bytes));
    std::vector<unsigned int> sizes(frames_per_superframe);
    for (unsigned int frame = 0; frame < frames_per_superframe; ++frame) {
        const int encoded = opus_encode_float(d_encoder,
                                               input + frame * d_frame_samples,
                                               d_frame_samples,
                                               packets[frame].data(), d_packet_bytes);
        if (encoded < 0)
            throw std::runtime_error(std::string("Opus encoding failed: ") +
                                     opus_strerror(encoded));
        sizes[frame] = static_cast<unsigned int>(encoded);
        packets[frame].resize(sizes[frame]);
    }

    unsigned int pos = 0;
    unsigned int border = 0;
    for (unsigned int size : sizes) {
        border += size;
        write_bits(output, pos, border, 12);
    }
    for (unsigned int frame = 0; frame < frames_per_superframe; ++frame)
        write_bits(output, pos, crc8(packets[frame].data(), sizes[frame]), 8);
    for (unsigned int frame = 0; frame < frames_per_superframe; ++frame)
        for (unsigned char byte : packets[frame]) write_bits(output, pos, byte, 8);

    consume_each(d_superframe_samples);
    return d_l_mux;
}

} }
