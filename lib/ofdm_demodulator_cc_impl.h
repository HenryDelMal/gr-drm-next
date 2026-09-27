/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OFDM_DEMODULATOR_CC_IMPL_H
#define INCLUDED_DRM_OFDM_DEMODULATOR_CC_IMPL_H

#include <drm/ofdm_demodulator_cc.h>
#include <gnuradio/fft/fft.h>
#include <memory>
#include <vector>

namespace gr { namespace drm {
class ofdm_demodulator_cc_impl : public ofdm_demodulator_cc
{
    unsigned int d_nfft, d_ncp, d_symbol_len;
    bool d_fft_shift;
    std::unique_ptr<gr::fft::fft_complex_fwd> d_fft;
    std::vector<gr_complex> d_symbol;
public:
    ofdm_demodulator_cc_impl(unsigned int nfft, unsigned int ncp, bool fft_shift);
    void forecast(int, gr_vector_int&) override;
    int general_work(int, gr_vector_int&, gr_vector_const_void_star&, gr_vector_void_star&) override;
};
} }
#endif
