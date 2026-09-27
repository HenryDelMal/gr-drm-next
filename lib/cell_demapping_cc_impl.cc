/* -*- c++ -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "cell_demapping_cc_impl.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace gr { namespace drm {

cell_demapping_cc::sptr cell_demapping_cc::make(transm_params* tp)
{
    return gnuradio::make_block_sptr<cell_demapping_cc_impl>(tp);
}

cell_demapping_cc_impl::cell_demapping_cc_impl(transm_params* tp)
    : gr::block("cell_demapping_cc",
                gr::io_signature::make(1, 1, tp->ofdm().nfft() * sizeof(gr_complex)),
                gr::io_signature::makev(
                    3, 3,
                    { static_cast<int>(tp->msc().N_MUX() * tp->ofdm().M_TF() * sizeof(gr_complex)),
                      static_cast<int>(tp->sdc().N() * sizeof(gr_complex)),
                      static_cast<int>(tp->fac().N() * tp->ofdm().M_TF() * sizeof(gr_complex)) })),
      d_tp(tp), d_tables(tp->cfg().ptables()), d_nfft(tp->ofdm().nfft()),
      d_ns(tp->ofdm().N_S()), d_mtf(tp->ofdm().M_TF()), d_nmsc(tp->msc().N_MUX()),
      d_nsdc(tp->sdc().N()), d_nfac(tp->fac().N()), d_symbols_buffered(0),
      d_rm(tp->cfg().RM()), d_kmin(tp->ofdm().K_min()),
      d_kmax(tp->ofdm().K_max()), d_n_sdc_sym(0),
      d_symbol_buffer(d_ns * d_mtf * d_nfft)
{
    switch (d_rm) {
    case 0: d_unused = { -1, 0, 1 }; d_n_sdc_sym = 2; break;
    case 1: case 2: case 3: d_unused = { 0 }; d_n_sdc_sym = d_rm < 2 ? 2 : 3; break;
    case 4: d_n_sdc_sym = 5; break;
    default: throw std::invalid_argument("invalid DRM robustness mode");
    }
}

bool cell_demapping_cc_impl::used_carrier(int k) const
{
    return std::find(d_unused.begin(), d_unused.end(), k) == d_unused.end();
}

std::vector<unsigned char> cell_demapping_cc_impl::build_reserved_mask() const
{
    const unsigned int symbols = d_ns * d_mtf;
    std::vector<unsigned char> mask(symbols * d_nfft, 0);
    const int k_off = d_nfft / 2;
    auto reserve = [&](unsigned int tf, unsigned int s, int k) {
        if (k >= d_kmin && k <= d_kmax)
            mask[(tf * d_ns + s) * d_nfft + k + k_off] = 1;
    };

    const int (*freq)[2] = nullptr;
    const int (*time)[2] = nullptr;
    const int (*fac)[2] = nullptr;
    unsigned int time_count = 0;
    switch (d_rm) {
    case 0: freq = tables::d_freq_A; time = tables::d_time_A; fac = tables::d_FAC_A; time_count = RMA_NUM_TIME_PIL; break;
    case 1: freq = tables::d_freq_B; time = tables::d_time_B; fac = tables::d_FAC_B; time_count = RMB_NUM_TIME_PIL; break;
    case 2: freq = tables::d_freq_C; time = tables::d_time_C; fac = tables::d_FAC_C; time_count = RMC_NUM_TIME_PIL; break;
    case 3: freq = tables::d_freq_D; time = tables::d_time_D; fac = tables::d_FAC_D; time_count = RMD_NUM_TIME_PIL; break;
    case 4: time = tables::d_time_E; fac = tables::d_FAC_E; time_count = RME_NUM_TIME_PIL; break;
    }

    for (unsigned int tf = 0; tf < d_mtf; ++tf) {
        if (d_rm != 4) {
            for (unsigned int s = 0; s < d_ns; ++s)
                for (unsigned int i = 0; i < NUM_FREQ_PILOTS; ++i)
                    reserve(tf, s, freq[i][0]);
        } else {
            for (unsigned int s = 4, col = 0; s < d_ns; s += 35, ++col)
                for (unsigned int i = 0; i < NUM_AFS_PILOTS; ++i)
                    reserve(tf, s, tables::d_AFS[i][0]);
        }
        for (unsigned int i = 0; i < time_count; ++i)
            reserve(tf, 0, time[i][0]);
        for (unsigned int s = 0; s < d_ns; ++s)
            for (const int k : d_tables->d_gain_pos[s])
                reserve(tf, s, k);
        for (unsigned int i = 0; i < d_nfac; ++i)
            reserve(tf, fac[i][0], fac[i][1]);
    }
    return mask;
}

void cell_demapping_cc_impl::forecast(int noutput_items, gr_vector_int& required)
{
    // A complete DRM superframe can be larger than GNU Radio's mapped input
    // buffer on platforms with a large allocation granularity (notably
    // macOS). Consume it incrementally instead of requiring all symbols to be
    // present in one scheduler call.
    required[0] = noutput_items > 0 && d_symbols_buffered < d_ns * d_mtf ? 1 : 0;
}

int cell_demapping_cc_impl::general_work(int noutput_items,
                                         gr_vector_int& ninput_items,
                                         gr_vector_const_void_star& input_items,
                                         gr_vector_void_star& output_items)
{
    const unsigned int symbols_per_superframe = d_ns * d_mtf;
    const auto* input = static_cast<const gr_complex*>(input_items[0]);
    auto* msc_out = static_cast<gr_complex*>(output_items[0]);
    auto* sdc_out = static_cast<gr_complex*>(output_items[1]);
    auto* fac_out = static_cast<gr_complex*>(output_items[2]);
    const int k_off = d_nfft / 2;
    int consumed = 0;
    int produced = 0;

    const int (*fac)[2] = d_rm == 0 ? tables::d_FAC_A :
                          d_rm == 1 ? tables::d_FAC_B :
                          d_rm == 2 ? tables::d_FAC_C :
                          d_rm == 3 ? tables::d_FAC_D : tables::d_FAC_E;

    while (produced < noutput_items) {
        const unsigned int symbols_needed = symbols_per_superframe - d_symbols_buffered;
        const unsigned int symbols_available = ninput_items[0] - consumed;
        const unsigned int symbols_to_copy = std::min(symbols_needed, symbols_available);

        if (symbols_to_copy > 0) {
            std::memcpy(d_symbol_buffer.data() + d_symbols_buffered * d_nfft,
                        input + consumed * d_nfft,
                        symbols_to_copy * d_nfft * sizeof(gr_complex));
            d_symbols_buffered += symbols_to_copy;
            consumed += symbols_to_copy;
        }
        if (d_symbols_buffered < symbols_per_superframe)
            break;

        const auto* super = d_symbol_buffer.data();
        auto reserved = build_reserved_mask();

        for (unsigned int tf = 0; tf < d_mtf; ++tf)
            for (unsigned int i = 0; i < d_nfac; ++i)
                fac_out[(produced * d_mtf + tf) * d_nfac + i] =
                    super[(tf * d_ns + fac[i][0]) * d_nfft + fac[i][1] + k_off];

        unsigned int sdc_n = 0;
        for (int s = 0; s < d_n_sdc_sym && sdc_n < d_nsdc; ++s) {
            for (int k = d_kmin; k <= d_kmax && sdc_n < d_nsdc; ++k) {
                const unsigned int index = s * d_nfft + k + k_off;
                if (!reserved[index] && used_carrier(k)) {
                    sdc_out[produced * d_nsdc + sdc_n++] = super[index];
                    reserved[index] = 1;
                }
            }
        }

        unsigned int msc_n = 0;
        for (unsigned int tf = 0; tf < d_mtf && msc_n < d_nmsc * d_mtf; ++tf) {
            for (unsigned int s = 0; s < d_ns && msc_n < d_nmsc * d_mtf; ++s) {
                for (int k = d_kmin; k <= d_kmax && msc_n < d_nmsc * d_mtf; ++k) {
                    const unsigned int index = (tf * d_ns + s) * d_nfft + k + k_off;
                    if (!reserved[index] && used_carrier(k))
                        msc_out[produced * d_nmsc * d_mtf + msc_n++] = super[index];
                }
            }
        }
        if (sdc_n != d_nsdc || msc_n != d_nmsc * d_mtf)
            throw std::runtime_error("DRM cell map did not match configured channel sizes");

        d_symbols_buffered = 0;
        ++produced;
    }
    consume_each(consumed);
    return produced;
}

} }
