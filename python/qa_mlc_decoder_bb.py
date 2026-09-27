#!/usr/bin/env python3

from gnuradio import blocks, gr, gr_unittest
import drm_next as drm


class qa_mlc_decoder_bb(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()
        self.tp = drm.transm_params(
            1, 3, False, 0, 1, 0, 1, 1, 0, False, 24000,
            "station label", "receiver decoder test")

    def tearDown(self):
        self.tb = None

    def test_fac_noiseless_roundtrip(self):
        length = self.tp.fac().L()
        source_bits = tuple((i * 5 + i // 7 + 1) & 1 for i in range(length))

        source = blocks.vector_source_b(source_bits, False)
        encoder = drm.make_mlc("FAC", self.tp)
        demapper = drm.qam_demapper_cb(
            self.tp.cfg().ptables().d_QAM4, self.tp.fac().mod_order(), 1)
        deinterleaver = drm.deinterleaver_bb(self.tp.fac().bit_interl_seq_0_2())
        decoder = drm.mlc_decoder_bb(self.tp, "FAC")
        sink = blocks.vector_sink_b()

        self.tb.connect(source, encoder, demapper, deinterleaver, decoder, sink)
        self.tb.run()
        self.assertEqual(source_bits, tuple(sink.data()))


if __name__ == "__main__":
    gr_unittest.run(qa_mlc_decoder_bb)
