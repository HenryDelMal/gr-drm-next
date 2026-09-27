#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2014 <+YOU OR YOUR COMPANY+>.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import drm_next as drm

class qa_cell_mapping_cc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()
        self.tp = drm.transm_params(1, 3, False, 0, 1, 0, 1, 1, 0, False, 24000, "station label", "text message")
        vlen_msc = self.tp.msc().N_MUX() * self.tp.ofdm().M_TF()
        vlen_sdc = self.tp.sdc().N()
        vlen_fac = self.tp.fac().N() * self.tp.ofdm().M_TF()
        self.cell_mapping = drm.cell_mapping_cc(
            self.tp,
            (vlen_msc * gr.sizeof_gr_complex,
             vlen_sdc * gr.sizeof_gr_complex,
             vlen_fac * gr.sizeof_gr_complex))

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        vlen_msc = self.tp.msc().N_MUX() * self.tp.ofdm().M_TF()
        vlen_sdc = self.tp.sdc().N()
        vlen_fac = self.tp.fac().N() * self.tp.ofdm().M_TF()
        nfft = self.tp.ofdm().nfft()

        def cells(length, offset):
            return [complex(((i + offset) % 7 + 1) / 10.0,
                            ((i + offset) % 5 + 1) / 10.0)
                    for i in range(length)]

        msc = cells(vlen_msc, 0)
        sdc = cells(vlen_sdc, 1)
        fac = cells(vlen_fac, 2)
        mapped_sink = blocks.vector_sink_c(nfft)
        self.tb.connect(blocks.vector_source_c(msc),
                        (self.cell_mapping, 0))
        self.tb.connect(blocks.vector_source_c(sdc),
                        (self.cell_mapping, 1))
        self.tb.connect(blocks.vector_source_c(fac),
                        (self.cell_mapping, 2))
        self.tb.connect(self.cell_mapping, mapped_sink)
        self.tb.run()

        mapped = mapped_sink.data()
        self.assertEqual(len(mapped), self.tp.ofdm().N_S() *
                         self.tp.ofdm().M_TF() * nfft)

        # Limit the upstream vector buffer to reproduce the macOS scheduler
        # condition where fewer than one superframe's 45 symbols fit at once.
        self.tb = gr.top_block()
        mapped_source = blocks.vector_source_c(mapped, False, nfft)
        mapped_source.set_max_output_buffer(7)
        demapper = drm.cell_demapping_cc(self.tp)
        msc_sink = blocks.vector_sink_c(vlen_msc)
        sdc_sink = blocks.vector_sink_c(vlen_sdc)
        fac_sink = blocks.vector_sink_c(vlen_fac)
        self.tb.connect(mapped_source, demapper)
        self.tb.connect((demapper, 0), msc_sink)
        self.tb.connect((demapper, 1), sdc_sink)
        self.tb.connect((demapper, 2), fac_sink)
        self.tb.run()

        self.assertComplexTuplesAlmostEqual(tuple(msc), msc_sink.data(), 6)
        self.assertComplexTuplesAlmostEqual(tuple(sdc), sdc_sink.data(), 6)
        self.assertComplexTuplesAlmostEqual(tuple(fac), fac_sink.data(), 6)


if __name__ == '__main__':
    gr_unittest.run(qa_cell_mapping_cc, "qa_cell_mapping_cc.xml")
