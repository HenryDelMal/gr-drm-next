/* -*- c++ -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "opus_audio_decoder_bf_impl.h"
#include <gnuradio/io_signature.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace gr { namespace drm {

opus_audio_decoder_bf::sptr opus_audio_decoder_bf::make(transm_params* tp)
{
    return gnuradio::make_block_sptr<opus_audio_decoder_bf_impl>(tp);
}

opus_audio_decoder_bf_impl::opus_audio_decoder_bf_impl(transm_params* tp)
    : gr::block("opus_audio_decoder_bf",
                gr::io_signature::make(1, 1, tp->msc().L_MUX()),
                gr::io_signature::make(1, 1, sizeof(float))),
      d_decoder(nullptr), d_l_mux(tp->msc().L_MUX()),
      d_sample_rate(tp->cfg().audio_samp_rate()), d_frame_samples(0)
{
    if (d_sample_rate != 12000 && d_sample_rate != 24000 && d_sample_rate != 48000)
        throw std::invalid_argument("Opus decoder supports 12, 24, or 48 kHz audio");
    d_frame_samples = d_sample_rate / 50;
    int error = OPUS_OK;
    d_decoder = opus_decoder_create(d_sample_rate, 1, &error);
    if (!d_decoder || error != OPUS_OK)
        throw std::runtime_error(std::string("Opus decoder initialization failed: ") +
                                 opus_strerror(error));
}

opus_audio_decoder_bf_impl::~opus_audio_decoder_bf_impl()
{
    if (d_decoder) opus_decoder_destroy(d_decoder);
}

unsigned int opus_audio_decoder_bf_impl::read_bits(const unsigned char* bits,
                                                    unsigned int& pos,
                                                    unsigned int count)
{
    unsigned int value = 0;
    for (unsigned int i = 0; i < count; ++i)
        value = (value << 1) | (bits[pos++] & 1U);
    return value;
}

unsigned char opus_audio_decoder_bf_impl::crc8(const unsigned char* data,
                                                unsigned int size)
{
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

void opus_audio_decoder_bf_impl::decode_superframe(const unsigned char* bits)
{
    unsigned int pos = 0;
    std::vector<unsigned int> borders(frames_per_superframe);
    for (auto& border : borders) border = read_bits(bits, pos, 12);
    std::vector<unsigned char> checksums(frames_per_superframe);
    for (auto& checksum : checksums)
        checksum = static_cast<unsigned char>(read_bits(bits, pos, 8));

    const unsigned int payload_bytes = (d_l_mux - pos) / 8;
    std::vector<unsigned char> payload(payload_bytes);
    for (auto& byte : payload) byte = static_cast<unsigned char>(read_bits(bits, pos, 8));

    unsigned int start = 0;
    std::vector<float> decoded(d_frame_samples);
    for (unsigned int frame = 0; frame < frames_per_superframe; ++frame) {
        const unsigned int end = borders[frame];
        const bool valid_bounds = end > start && end <= payload.size();
        const bool valid_crc = valid_bounds &&
            crc8(payload.data() + start, end - start) == checksums[frame];
        const unsigned char* packet = valid_crc ? payload.data() + start : nullptr;
        const opus_int32 packet_size = valid_crc ? static_cast<opus_int32>(end - start) : 0;
        const int samples = opus_decode_float(d_decoder, packet, packet_size,
                                               decoded.data(), d_frame_samples, 0);
        if (samples > 0)
            d_pending.insert(d_pending.end(), decoded.begin(), decoded.begin() + samples);
        else
            d_pending.insert(d_pending.end(), d_frame_samples, 0.0f);
        if (!valid_bounds) {
            start = payload.size();
        } else {
            start = end;
        }
    }
}

void opus_audio_decoder_bf_impl::forecast(int noutput_items, gr_vector_int& required)
{
    required[0] = d_pending.empty() && noutput_items > 0 ? 1 : 0;
}

int opus_audio_decoder_bf_impl::general_work(int noutput_items,
                                              gr_vector_int& ninput_items,
                                              gr_vector_const_void_star& input_items,
                                              gr_vector_void_star& output_items)
{
    if (d_pending.empty() && ninput_items[0] > 0) {
        decode_superframe(static_cast<const unsigned char*>(input_items[0]));
        consume_each(1);
    }
    const int produced = std::min<int>(noutput_items, d_pending.size());
    if (produced > 0) {
        std::memcpy(output_items[0], d_pending.data(), produced * sizeof(float));
        d_pending.erase(d_pending.begin(), d_pending.begin() + produced);
    }
    return produced;
}

} }
