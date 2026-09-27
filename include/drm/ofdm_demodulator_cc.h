/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OFDM_DEMODULATOR_CC_H
#define INCLUDED_DRM_OFDM_DEMODULATOR_CC_H

#include <drm/api.h>
#include <gnuradio/block.h>

namespace gr { namespace drm {

/*! Remove the DRM cyclic prefix and FFT one synchronized OFDM stream.
 *
 * Input consists of scalar complex samples. Output consists of one
 * ``nfft``-element complex vector for each decoded OFDM symbol. The block
 * assumes symbol timing and coarse frequency correction have already been
 * performed; that synchronization layer is intentionally separate.
 */
class DRM_API ofdm_demodulator_cc : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<ofdm_demodulator_cc>;
    static sptr make(unsigned int nfft, unsigned int ncp, bool fft_shift = true);
};

} }
#endif
