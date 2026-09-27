#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ofdm_synchronizer_cc_impl.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
namespace gr { namespace drm {
ofdm_synchronizer_cc::sptr ofdm_synchronizer_cc::make(unsigned int nfft, unsigned int ncp)
{ return gnuradio::make_block_sptr<ofdm_synchronizer_cc_impl>(nfft,ncp); }

ofdm_synchronizer_cc_impl::ofdm_synchronizer_cc_impl(unsigned int nfft, unsigned int ncp)
    : gr::block("ofdm_synchronizer_cc", gr::io_signature::make(1,1,sizeof(gr_complex)),
                gr::io_signature::make(1,1,sizeof(gr_complex))),
      d_nfft(nfft), d_ncp(ncp), d_symbol_len(nfft+ncp), d_locked(false),
      d_frequency(0), d_phase(0)
{
    if (!nfft || !ncp || ncp >= nfft) throw std::invalid_argument("invalid OFDM dimensions");
    set_output_multiple(d_symbol_len);
}

double ofdm_synchronizer_cc_impl::cp_metric(const gr_complex* in, unsigned int offset) const
{
    std::complex<double> correlation(0,0);
    double a=0, b=0;
    for (unsigned int i=0; i<d_ncp; ++i) {
        const auto first = static_cast<std::complex<double>>(in[offset+i]);
        const auto second = static_cast<std::complex<double>>(in[offset+d_nfft+i]);
        correlation += std::conj(first)*second;
        a += std::norm(first); b += std::norm(second);
    }
    return std::norm(correlation)/(a*b+1e-30);
}

void ofdm_synchronizer_cc_impl::forecast(int noutput_items, gr_vector_int& required)
{ required[0] = noutput_items + (d_locked ? 0 : d_symbol_len); }

int ofdm_synchronizer_cc_impl::general_work(int noutput_items, gr_vector_int& ninput_items,
        gr_vector_const_void_star& inputs, gr_vector_void_star& outputs)
{
    const auto* in = static_cast<const gr_complex*>(inputs[0]);
    auto* out = static_cast<gr_complex*>(outputs[0]);
    int start = 0;
    if (!d_locked) {
        if (ninput_items[0] < static_cast<int>(2*d_symbol_len)) return 0;
        double best=-1;
        for (unsigned int offset=0; offset<d_symbol_len; ++offset) {
            const double metric=cp_metric(in,offset);
            if (metric>best) { best=metric; start=offset; }
        }
        d_locked=true;
    }
    const int symbols=std::min((ninput_items[0]-start)/static_cast<int>(d_symbol_len),
                               noutput_items/static_cast<int>(d_symbol_len));
    for (int symbol=0; symbol<symbols; ++symbol) {
        const auto* src=in+start+symbol*d_symbol_len;
        std::complex<double> correlation(0,0);
        for (unsigned int i=0;i<d_ncp;++i)
            correlation += std::conj(static_cast<std::complex<double>>(src[i])) *
                           static_cast<std::complex<double>>(src[d_nfft+i]);
        const double estimate=std::arg(correlation)/d_nfft;
        d_frequency = symbol==0 && d_phase==0 ? estimate : 0.85*d_frequency+0.15*estimate;
        for (unsigned int i=0;i<d_symbol_len;++i) {
            const gr_complex correction(std::cos(d_phase),-std::sin(d_phase));
            out[symbol*d_symbol_len+i]=src[i]*correction;
            d_phase=std::remainder(d_phase+d_frequency,2.0*M_PI);
        }
    }
    consume_each(start+symbols*d_symbol_len);
    return symbols*d_symbol_len;
}
} }
