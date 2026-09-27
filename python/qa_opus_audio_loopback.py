#!/usr/bin/env python

import math

from gnuradio import blocks, gr, gr_unittest
import drm_next as drm


class qa_opus_audio_loopback(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_tone_survives_codec_loopback(self):
        sample_rate = 24000
        tp = drm.transm_params(1, 3, False, 0, 1, 0, 1, 1, 0, False,
                               sample_rate, "gr-drm-next", "")
        samples = [0.25 * math.sin(2 * math.pi * 1000 * n / sample_rate)
                   for n in range(int(sample_rate * 0.4))]
        source = blocks.vector_source_f(samples, False)
        encoder = drm.opus_audio_encoder_fb(tp)
        to_vector = blocks.stream_to_vector(gr.sizeof_char, tp.msc().L_MUX())
        decoder = drm.opus_audio_decoder_bf(tp)
        sink = blocks.vector_sink_f()
        self.tb.connect(source, encoder, to_vector, decoder, sink)
        self.tb.run()

        decoded = sink.data()
        self.assertEqual(len(decoded), len(samples))
        self.assertGreater(max(abs(sample) for sample in decoded), 0.02)
        rms = math.sqrt(sum(sample * sample for sample in decoded) / len(decoded))
        self.assertGreater(rms, 0.01)

    def test_custom_encoder_parameters(self):
        tp = drm.transm_params(1, 3, False, 0, 1, 0, 1, 1, 0, False,
                               24000, "gr-drm-next", "")
        encoder = drm.opus_audio_encoder_fb(
            tp, 8000, True, "voip", "voice", "wideband", 5, True)
        self.assertIsNotNone(encoder)
        with self.assertRaises(ValueError):
            drm.opus_audio_encoder_fb(tp, 64000)


if __name__ == '__main__':
    gr_unittest.run(qa_opus_audio_loopback)
