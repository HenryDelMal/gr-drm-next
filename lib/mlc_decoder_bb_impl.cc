#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "mlc_decoder_bb_impl.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace gr { namespace drm {
namespace {
constexpr int K = 7;
constexpr int STATES = 1 << (K - 1);
constexpr int DENOM = 6;
constexpr int TAIL_INPUT_BITS = 6;
constexpr std::array<unsigned int, DENOM> POLY = {91, 121, 101, 91, 121, 101};
}

mlc_decoder_bb::sptr mlc_decoder_bb::make(transm_params* tp, const std::string& channel_type)
{ return gnuradio::make_block_sptr<mlc_decoder_bb_impl>(tp, channel_type); }

mlc_decoder_bb_impl::mlc_decoder_bb_impl(transm_params* tp, const std::string& channel_type)
    : gr::block("mlc_decoder_bb", gr::io_signature::make(1,3,1),
                gr::io_signature::make(1,1,1)),
      d_tp(tp), d_channel(channel_type), d_levels(0), d_cells(0), d_output_bits(0)
{
    if (channel_type == "FAC") {
        d_levels = 1; d_cells = tp->fac().N(); d_output_bits = tp->fac().L();
        d_partition_lengths = tp->fac().M_total();
        d_patterns = {tp->fac().punct_pat_0()};
        d_tail_patterns = {tp->fac().punct_pat_0()}; // matches the current FAC encoder
    } else if (channel_type == "SDC") {
        d_levels = tp->sdc().n_levels_mlc(); d_cells = tp->sdc().N(); d_output_bits = tp->sdc().L();
        d_partition_lengths = tp->sdc().M_total();
        d_patterns = {tp->sdc().punct_pat_0(), tp->sdc().punct_pat_1()};
        d_tail_patterns = {tp->sdc().punct_pat_tail_0(), tp->sdc().punct_pat_tail_1()};
    } else if (channel_type == "MSC") {
        d_levels = tp->msc().n_levels_mlc(); d_cells = tp->msc().N_MUX(); d_output_bits = tp->msc().L_MUX();
        d_partition_lengths = tp->msc().M_total();
        d_patterns = {tp->msc().punct_pat_0_2(), tp->msc().punct_pat_1_2(), tp->msc().punct_pat_2_2()};
        d_tail_patterns = {tp->msc().punct_pat_tail_0_2(), tp->msc().punct_pat_tail_1_2(), tp->msc().punct_pat_tail_2_2()};
    } else throw std::invalid_argument("channel_type must be FAC, SDC, or MSC");
    if (d_levels < 1 || d_levels > 3 || static_cast<int>(d_partition_lengths.size()) < d_levels)
        throw std::runtime_error("invalid MLC parameter set");
    set_output_multiple(d_output_bits);
}

unsigned char mlc_decoder_bb_impl::parity(unsigned int value)
{
#if defined(__GNUC__)
    return __builtin_parity(value);
#else
    unsigned char p=0; while(value){p^=value&1; value>>=1;} return p;
#endif
}

std::vector<unsigned char> mlc_decoder_bb_impl::decode_level(
    const unsigned char* input, int output_bits,
    const std::vector<unsigned char>& pattern,
    const std::vector<unsigned char>& tail_pattern) const
{
    const int steps = output_bits + TAIL_INPUT_BITS;
    const int normal_encoded = output_bits * DENOM;
    const int total_encoded = steps * DENOM;
    std::vector<int> observations(total_encoded, -1);
    int source = 0;
    for (int j=0; j<total_encoded; ++j) {
        const bool keep = j < normal_encoded
            ? pattern[j % pattern.size()] != 0
            : tail_pattern[(j-normal_encoded) % tail_pattern.size()] != 0;
        if (keep) observations[j] = input[source++] & 1;
    }
    if (source != 2*d_cells)
        throw std::runtime_error("puncturing pattern does not match channel cell count");

    const int INF = std::numeric_limits<int>::max()/4;
    std::vector<int> metric(STATES, INF), next(STATES, INF);
    std::vector<unsigned char> prev_state(steps*STATES), prev_bit(steps*STATES);
    metric[0] = 0;
    for (int t=0; t<steps; ++t) {
        std::fill(next.begin(), next.end(), INF);
        for (int state=0; state<STATES; ++state) if (metric[state] < INF) {
            for (int bit=0; bit<2; ++bit) {
                // GNU Radio's trellis::fsm shifts the new bit into the most
                // significant memory position.  Match that convention so
                // this decoder is the exact inverse of trellis::encoder_bb.
                const unsigned int reg = state | (bit << (K - 1));
                const int ns = (state >> 1) | (bit << (K - 2));
                int branch = 0;
                for (int p=0; p<DENOM; ++p) {
                    const int observed = observations[t*DENOM+p];
                    if (observed >= 0) branch += observed != parity(reg & POLY[p]);
                }
                const int candidate = metric[state] + branch;
                if (candidate < next[ns]) {
                    next[ns] = candidate;
                    prev_state[t*STATES+ns] = state;
                    prev_bit[t*STATES+ns] = bit;
                }
            }
        }
        metric.swap(next);
    }
    int state = 0;
    if (metric[state] >= INF) state = std::min_element(metric.begin(),metric.end())-metric.begin();
    std::vector<unsigned char> decoded(steps);
    for (int t=steps-1; t>=0; --t) {
        decoded[t] = prev_bit[t*STATES+state];
        state = prev_state[t*STATES+state];
    }
    decoded.resize(output_bits);
    return decoded;
}

void mlc_decoder_bb_impl::forecast(int noutput_items, gr_vector_int& required)
{
    const int frames = std::max(1, noutput_items / d_output_bits);
    for (int level=0; level<d_levels; ++level) required[level] = frames * 2*d_cells;
}

int mlc_decoder_bb_impl::general_work(int noutput_items, gr_vector_int& ninput_items,
                                      gr_vector_const_void_star& inputs, gr_vector_void_star& outputs)
{
    int frames = noutput_items / d_output_bits;
    for (int level=0; level<d_levels; ++level)
        frames = std::min(frames, ninput_items[level]/(2*d_cells));
    auto* out = static_cast<unsigned char*>(outputs[0]);
    for (int frame=0; frame<frames; ++frame) {
        int offset = 0;
        for (int level=0; level<d_levels; ++level) {
            const auto* in = static_cast<const unsigned char*>(inputs[level]) + frame*2*d_cells;
            auto decoded = decode_level(in, d_partition_lengths[level], d_patterns[level], d_tail_patterns[level]);
            std::memcpy(out + frame*d_output_bits + offset, decoded.data(), decoded.size());
            offset += decoded.size();
        }
        if (offset != d_output_bits) throw std::runtime_error("MLC partitions do not reconstruct channel payload");
    }
    for (int level=0; level<d_levels; ++level) consume(level,frames*2*d_cells);
    return frames*d_output_bits;
}
} }
