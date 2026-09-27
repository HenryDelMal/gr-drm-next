#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "deinterleaver_bb_impl.h"
#include <stdexcept>
namespace gr { namespace drm {
deinterleaver_bb::sptr deinterleaver_bb::make(const std::vector<int>& sequence)
{ return gnuradio::make_block_sptr<deinterleaver_bb_impl>(sequence); }
deinterleaver_bb_impl::deinterleaver_bb_impl(const std::vector<int>& sequence)
    : gr::sync_block("deinterleaver_bb", gr::io_signature::make(1,1,1),
                     gr::io_signature::make(1,1,1)), d_seq(sequence)
{
    if (d_seq.empty()) throw std::invalid_argument("empty deinterleaver sequence");
    set_output_multiple(d_seq.size());
}
int deinterleaver_bb_impl::work(int noutput_items, gr_vector_const_void_star& inputs,
                                gr_vector_void_star& outputs)
{
    const auto* in = static_cast<const unsigned char*>(inputs[0]);
    auto* out = static_cast<unsigned char*>(outputs[0]);
    const int vectors = noutput_items / d_seq.size();
    for (int n=0; n<vectors; ++n)
        for (size_t i=0; i<d_seq.size(); ++i)
            out[n*d_seq.size()+d_seq[i]] = in[n*d_seq.size()+i];
    return vectors*d_seq.size();
}
} }
