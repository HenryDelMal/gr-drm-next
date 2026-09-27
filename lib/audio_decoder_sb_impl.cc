/* -*- c++ -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "audio_decoder_sb_impl.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace gr { namespace drm {

audio_decoder_sb::sptr audio_decoder_sb::make(transm_params* tp)
{
    return gnuradio::make_block_sptr<audio_decoder_sb_impl>(tp);
}

audio_decoder_sb_impl::audio_decoder_sb_impl(transm_params* tp)
    : gr::block("audio_decoder_sb",
                gr::io_signature::make(1, 1, tp->msc().L_MUX()),
                gr::io_signature::make(1, 1, sizeof(float))),
      d_tp(tp), d_decoder(nullptr), d_l_mux(tp->msc().L_MUX()),
      d_n_aac_frames(0), d_n_header_bytes(0), d_sample_rate(tp->cfg().audio_samp_rate())
{
    if (d_sample_rate == 12000) { d_n_aac_frames = 5; d_n_header_bytes = 6; }
    else if (d_sample_rate == 24000) { d_n_aac_frames = 10; d_n_header_bytes = 14; }
    else throw std::invalid_argument("FAAD2 DRM decoder supports 12 or 24 kHz audio");

    d_decoder = NeAACDecOpen();
    if (!d_decoder)
        throw std::runtime_error("FAAD2 decoder could not be opened");
    auto* cfg = NeAACDecGetCurrentConfiguration(d_decoder);
    cfg->defObjectType = DRM_ER_LC;
    cfg->defSampleRate = d_sample_rate;
    cfg->outputFormat = FAAD_FMT_FLOAT;
    cfg->dontUpSampleImplicitSBR = 1;
    if (!NeAACDecSetConfiguration(d_decoder, cfg))
        throw std::runtime_error("FAAD2 decoder configuration failed");
}

audio_decoder_sb_impl::~audio_decoder_sb_impl()
{
    if (d_decoder) NeAACDecClose(d_decoder);
}

unsigned int audio_decoder_sb_impl::read_bits(const unsigned char* bits, unsigned int& pos, unsigned int count)
{
    unsigned int value = 0;
    for (unsigned int i = 0; i < count; ++i) value = (value << 1) | (bits[pos++] & 1U);
    return value;
}

void audio_decoder_sb_impl::bits_to_bytes(const unsigned char* bits, unsigned int n_bits,
                                          std::vector<unsigned char>& bytes)
{
    bytes.clear();
    for (unsigned int pos = 0; pos + 8 <= n_bits; pos += 8) {
        unsigned char value = 0;
        for (unsigned int i = 0; i < 8; ++i) value = (value << 1) | (bits[pos + i] & 1U);
        bytes.push_back(value);
    }
}

void audio_decoder_sb_impl::decode_superframe(const unsigned char* bits)
{
    unsigned int pos = 0;
    std::vector<unsigned int> frame_end(d_n_aac_frames - 1);
    for (auto& end : frame_end) end = read_bits(bits, pos, 12);
    if (d_n_aac_frames == 10) (void)read_bits(bits, pos, 4);
    for (unsigned int i = 0; i < d_n_aac_frames; ++i) (void)read_bits(bits, pos, 8);

    std::vector<unsigned char> payload;
    bits_to_bytes(bits + pos, d_l_mux - pos, payload);
    const unsigned int payload_bytes = payload.size();
    unsigned int frame_start = 0;
    for (unsigned int frame = 0; frame < d_n_aac_frames; ++frame) {
        const unsigned int frame_end_byte = frame + 1 < d_n_aac_frames
            ? std::min(frame_end[frame], payload_bytes) : payload_bytes;
        if (frame_end_byte <= frame_start) { frame_start = frame_end_byte; continue; }
        NeAACDecFrameInfo info{};
        auto* decoded = static_cast<float*>(NeAACDecDecode(
            d_decoder, &info, payload.data() + frame_start, frame_end_byte - frame_start));
        if (info.error == 0 && decoded)
            d_pending.insert(d_pending.end(), decoded, decoded + info.samples * info.channels);
        frame_start = frame_end_byte;
    }
}

void audio_decoder_sb_impl::forecast(int noutput_items, gr_vector_int& required)
{
    required[0] = d_pending.empty() && noutput_items > 0 ? 1 : 0;
}

int audio_decoder_sb_impl::general_work(int noutput_items, gr_vector_int& ninput_items,
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
