#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "deinterleaver_cc_impl.h"
#include <stdexcept>
namespace gr { namespace drm {
deinterleaver_cc::sptr deinterleaver_cc::make(const std::vector<int>& sequence,
                                              bool long_interleaving, int depth)
{ return gnuradio::make_block_sptr<deinterleaver_cc_impl>(sequence,long_interleaving,depth); }
deinterleaver_cc_impl::deinterleaver_cc_impl(const std::vector<int>& sequence, bool long_interleaving, int depth)
    : gr::block("deinterleaver_cc", gr::io_signature::make(1,1,sizeof(gr_complex)),
                gr::io_signature::make(1,1,sizeof(gr_complex))),
      d_seq(sequence), d_long(long_interleaving), d_depth(depth)
{
    if (d_seq.empty() || d_depth < 1) throw std::invalid_argument("invalid cell deinterleaver parameters");
    set_output_multiple(d_seq.size());
}
void deinterleaver_cc_impl::forecast(int, gr_vector_int& required)
{ required[0] = d_seq.size(); }
int deinterleaver_cc_impl::general_work(int noutput_items, gr_vector_int& ninput_items,
                                       gr_vector_const_void_star& inputs, gr_vector_void_star& outputs)
{
    if (ninput_items[0] < static_cast<int>(d_seq.size()) || noutput_items < static_cast<int>(d_seq.size())) return 0;
    const auto* in = static_cast<const gr_complex*>(inputs[0]);
    auto* out = static_cast<gr_complex*>(outputs[0]);
    if (!d_long) {
        for (size_t i=0; i<d_seq.size(); ++i) out[d_seq[i]] = in[i];
        consume_each(d_seq.size());
        return d_seq.size();
    }
    d_buffer.emplace_back(in, in+d_seq.size());
    consume_each(d_seq.size());
    if (d_buffer.size() < static_cast<size_t>(d_depth)) return 0;
    for (size_t i=0; i<d_seq.size(); ++i)
        out[d_seq[i]] = d_buffer[i % d_depth][i];
    d_buffer.pop_front();
    return d_seq.size();
}
} }
