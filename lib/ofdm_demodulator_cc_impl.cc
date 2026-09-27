/* -*- c++ -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ofdm_demodulator_cc_impl.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace gr { namespace drm {

ofdm_demodulator_cc::sptr ofdm_demodulator_cc::make(unsigned int nfft,
                                                    unsigned int ncp,
                                                    bool fft_shift)
{
    return gnuradio::make_block_sptr<ofdm_demodulator_cc_impl>(nfft, ncp, fft_shift);
}

ofdm_demodulator_cc_impl::ofdm_demodulator_cc_impl(unsigned int nfft,
                                                   unsigned int ncp,
                                                   bool fft_shift)
    : gr::block("ofdm_demodulator_cc",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::make(1, 1, nfft * sizeof(gr_complex))),
      d_nfft(nfft), d_ncp(ncp), d_symbol_len(nfft + ncp), d_fft_shift(fft_shift),
      d_fft(std::make_unique<gr::fft::fft_complex_fwd>(nfft)), d_symbol(nfft)
{
    if (nfft == 0 || ncp >= nfft)
        throw std::invalid_argument("invalid OFDM FFT or cyclic-prefix length");
    set_output_multiple(1);
}

void ofdm_demodulator_cc_impl::forecast(int noutput_items, gr_vector_int& required)
{
    required[0] = noutput_items * d_symbol_len;
}

int ofdm_demodulator_cc_impl::general_work(int noutput_items,
                                           gr_vector_int& ninput_items,
                                           gr_vector_const_void_star& input_items,
                                           gr_vector_void_star& output_items)
{
    const auto* in = static_cast<const gr_complex*>(input_items[0]);
    auto* out = static_cast<gr_complex*>(output_items[0]);
    const int symbols = std::min(noutput_items, ninput_items[0] / static_cast<int>(d_symbol_len));

    for (int symbol = 0; symbol < symbols; ++symbol) {
        std::memcpy(d_fft->get_inbuf(), in + symbol * d_symbol_len + d_ncp,
                    d_nfft * sizeof(gr_complex));
        d_fft->execute();
        const auto* spectrum = d_fft->get_outbuf();
        if (!d_fft_shift) {
            std::memcpy(out + symbol * d_nfft, spectrum, d_nfft * sizeof(gr_complex));
        } else {
            const unsigned int half = d_nfft / 2;
            for (unsigned int k = 0; k < d_nfft; ++k)
                out[symbol * d_nfft + k] = spectrum[(k + half) % d_nfft];
        }
    }
    consume_each(symbols * d_symbol_len);
    return symbols;
}

} }
