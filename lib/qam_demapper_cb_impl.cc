/* -*- c++ -*- */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "qam_demapper_cb_impl.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace gr { namespace drm {

qam_demapper_cb::sptr qam_demapper_cb::make(const float map_table[][2],
                                            int bits_per_symbol,
                                            int n_outputs)
{
    return gnuradio::make_block_sptr<qam_demapper_cb_impl>(map_table, bits_per_symbol, n_outputs);
}

qam_demapper_cb_impl::qam_demapper_cb_impl(const float map_table[][2],
                                           int bits_per_symbol,
                                           int n_outputs)
    : gr::block("qam_demapper_cb",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::make(n_outputs, n_outputs, sizeof(unsigned char))),
      d_rows(1 << n_outputs), d_outputs(n_outputs)
{
    if (bits_per_symbol != 2 * n_outputs || n_outputs < 1 || n_outputs > 3)
        throw std::invalid_argument("QAM demapper requires 2, 4, or 6 bits per symbol");
    std::memset(d_map, 0, sizeof(d_map));
    for (int i = 0; i < d_rows; ++i) {
        d_map[i][0] = map_table[i][0];
        d_map[i][1] = map_table[i][1];
    }
    set_output_multiple(2);
}

void qam_demapper_cb_impl::forecast(int noutput_items, gr_vector_int& required)
{
    required[0] = (noutput_items + 1) / 2;
}

int qam_demapper_cb_impl::general_work(int noutput_items,
                                       gr_vector_int& ninput_items,
                                       gr_vector_const_void_star& input_items,
                                       gr_vector_void_star& output_items)
{
    const auto* in = static_cast<const gr_complex*>(input_items[0]);
    const int symbols = std::min(ninput_items[0], noutput_items / 2);
    for (int n = 0; n < symbols; ++n) {
        int i_index = 0, q_index = 0;
        float i_error = std::numeric_limits<float>::max();
        float q_error = std::numeric_limits<float>::max();
        for (int row = 0; row < d_rows; ++row) {
            const float ie = std::abs(in[n].real() - d_map[row][0]);
            const float qe = std::abs(in[n].imag() - d_map[row][1]);
            if (ie < i_error) { i_error = ie; i_index = row; }
            if (qe < q_error) { q_error = qe; q_index = row; }
        }
        for (int stream = 0; stream < d_outputs; ++stream) {
            auto* out = static_cast<unsigned char*>(output_items[stream]);
            const int shift = d_outputs - stream - 1;
            out[2 * n] = (i_index >> shift) & 1;
            out[2 * n + 1] = (q_index >> shift) & 1;
        }
    }
    consume_each(symbols);
    return symbols * 2;
}

} }
