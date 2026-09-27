#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "pilot_equalizer_vcc_impl.h"
#include <algorithm>
#include <complex>
#include <cstring>
#include <stdexcept>
namespace gr { namespace drm {
pilot_equalizer_vcc::sptr pilot_equalizer_vcc::make(transm_params* tp)
{ return gnuradio::make_block_sptr<pilot_equalizer_vcc_impl>(tp); }

pilot_equalizer_vcc_impl::pilot_equalizer_vcc_impl(transm_params* tp)
    : gr::sync_block("pilot_equalizer_vcc",
          gr::io_signature::make(1,1,tp->ofdm().nfft()*sizeof(gr_complex)),
          gr::io_signature::make(1,1,tp->ofdm().nfft()*sizeof(gr_complex))),
      d_tables(tp->cfg().ptables()), d_nfft(tp->ofdm().nfft()),
      d_ns(tp->ofdm().N_S()), d_rm(tp->cfg().RM()), d_symbol(0)
{
    if (d_nfft <= 0 || d_ns <= 0) throw std::invalid_argument("invalid DRM OFDM parameters");
}

bool pilot_equalizer_vcc_impl::overlaps_reference(int symbol, int carrier) const
{
    if (d_rm != 4) {
        const int (*freq)[2] = d_rm==0 ? tables::d_freq_A : d_rm==1 ? tables::d_freq_B :
                               d_rm==2 ? tables::d_freq_C : tables::d_freq_D;
        for (int i=0; i<NUM_FREQ_PILOTS; ++i) if (freq[i][0] == carrier) return true;
    } else if ((symbol == 4 || symbol == 39)) {
        for (int i=0; i<NUM_AFS_PILOTS; ++i) if (tables::d_AFS[i][0] == carrier) return true;
    }
    if (symbol != 0) return false;
    const int (*time)[2] = d_rm==0 ? tables::d_time_A : d_rm==1 ? tables::d_time_B :
                               d_rm==2 ? tables::d_time_C : d_rm==3 ? tables::d_time_D : tables::d_time_E;
    const int count = d_rm==0 ? RMA_NUM_TIME_PIL : d_rm==1 ? RMB_NUM_TIME_PIL :
                      d_rm==2 ? RMC_NUM_TIME_PIL : d_rm==3 ? RMD_NUM_TIME_PIL : RME_NUM_TIME_PIL;
    for (int i=0; i<count; ++i) if (time[i][0] == carrier) return true;
    return false;
}

int pilot_equalizer_vcc_impl::work(int noutput_items, gr_vector_const_void_star& inputs,
                                    gr_vector_void_star& outputs)
{
    const auto* in = static_cast<const gr_complex*>(inputs[0]);
    auto* out = static_cast<gr_complex*>(outputs[0]);
    const int offset = d_nfft/2;
    for (int item=0; item<noutput_items; ++item) {
        const int symbol = d_symbol % d_ns;
        std::complex<double> numerator(0,0);
        double denominator = 0;
        for (size_t p=0; p<d_tables->d_gain_pos[symbol].size(); ++p) {
            const int carrier = d_tables->d_gain_pos[symbol][p];
            if (overlaps_reference(symbol, carrier)) continue;
            const std::complex<double> known = d_tables->d_gain_cells[symbol][p];
            numerator += static_cast<std::complex<double>>(in[item*d_nfft + carrier + offset]) * std::conj(known);
            denominator += std::norm(known);
        }
        const std::complex<double> channel = denominator > 0 ? numerator/denominator : std::complex<double>(1,0);
        const std::complex<float> inverse = std::norm(channel) > 1e-12
            ? static_cast<std::complex<float>>(std::conj(channel)/std::norm(channel))
            : gr_complex(1,0);
        for (int k=0; k<d_nfft; ++k) out[item*d_nfft+k] = in[item*d_nfft+k] * inverse;
        d_symbol = (d_symbol + 1) % d_ns;
    }
    return noutput_items;
}
} }
