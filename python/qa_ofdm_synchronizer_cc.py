#!/usr/bin/env python3
from gnuradio import blocks, gr, gr_unittest
import drm


class qa_ofdm_synchronizer_cc(gr_unittest.TestCase):
    def test_prefix_timing_acquisition(self):
        nfft, ncp = 32, 8
        symbols = []
        for frame in range(4):
            body = [complex(((7*i+3*frame) % 17)-8, ((5*i+frame) % 13)-6)
                    for i in range(nfft)]
            symbols.extend(body[-ncp:] + body)
        leading = [complex(0.13*i, -0.07*i) for i in range(5)]

        tb = gr.top_block()
        source = blocks.vector_source_c(leading + symbols, False)
        synchronizer = drm.ofdm_synchronizer_cc(nfft, ncp)
        sink = blocks.vector_sink_c()
        tb.connect(source, synchronizer, sink)
        tb.run()

        self.assertComplexTuplesAlmostEqual(tuple(symbols), tuple(sink.data()), 5)


if __name__ == "__main__":
    gr_unittest.run(qa_ofdm_synchronizer_cc)
