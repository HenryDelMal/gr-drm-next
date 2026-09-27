"""Complete fixed-configuration DRM receive chain for GNU Radio 3.10."""

from gnuradio import blocks, gr
import drm_next as drm


class drm_channel_receiver_ccb(gr.hier_block2):
    """Synchronize complex DRM baseband and recover descrambled MSC frames.

    FAC and SDC are fully channel-decoded and descrambled inside the hierarchy.
    This first receiver API uses the supplied ``transm_params`` as its service
    configuration; dynamic FAC/SDC-driven reconfiguration can be layered on it.
    """

    def __init__(self, tp):
        gr.hier_block2.__init__(
            self, "DRM Channel Receiver",
            gr.io_signature(1, 1, gr.sizeof_gr_complex),
            gr.io_signature(1, 1, gr.sizeof_char * tp.msc().L_MUX()),
        )
        self.tp = tp
        nfft = tp.ofdm().nfft()
        ncp = tp.ofdm().n_cp()

        self.synchronizer = drm.ofdm_synchronizer_cc(nfft, ncp)
        self.demodulator = drm.ofdm_demodulator_cc(nfft, ncp, True)
        self.equalizer = drm.pilot_equalizer_vcc(tp)
        self.cell_demapper = drm.cell_demapping_cc(tp)
        self.connect(self, self.synchronizer, self.demodulator,
                     self.equalizer, self.cell_demapper)

        self._build_msc_path()
        self._build_control_path("SDC", 1, tp.sdc().N(), tp.sdc().L())
        self._build_control_path("FAC", 2, tp.fac().N() * tp.ofdm().M_TF(),
                                 tp.fac().L())

    @staticmethod
    def _qam_table(tp, channel):
        params = tp.msc() if channel == "MSC" else tp.sdc() if channel == "SDC" else tp.fac()
        order = params.mod_order()
        tables = tp.cfg().ptables()
        return tables.d_QAM4 if order == 2 else tables.d_QAM16 if order == 4 else tables.d_QAM64SM

    @staticmethod
    def _bit_sequences(tp, channel, levels):
        params = tp.msc() if channel == "MSC" else tp.sdc() if channel == "SDC" else tp.fac()
        if channel == "MSC" and tp.cfg().msc_mapping() == 2:
            return [None, params.bit_interl_seq_1_2(), params.bit_interl_seq_2_2()]
        sequences = [params.bit_interl_seq_0_2()]
        if levels > 1:
            sequences.append(params.bit_interl_seq_1_2())
        return sequences

    def _channel_decoder(self, source, channel, cells, payload_bits):
        params = self.tp.msc() if channel == "MSC" else self.tp.sdc() if channel == "SDC" else self.tp.fac()
        levels = params.n_levels_mlc() if channel != "FAC" else 1
        flatten = blocks.vector_to_stream(gr.sizeof_gr_complex, cells)
        demapper = drm.qam_demapper_cb(
            self._qam_table(self.tp, channel), params.mod_order(), levels)
        decoder = drm.mlc_decoder_bb(self.tp, channel)
        self.connect(source, flatten, demapper)
        deinterleavers = []
        for level, sequence in enumerate(self._bit_sequences(self.tp, channel, levels)):
            if sequence is None:
                self.connect((demapper, level), (decoder, level))
            else:
                block = drm.deinterleaver_bb(sequence)
                deinterleavers.append(block)
                self.connect((demapper, level), block, (decoder, level))
        descrambler = drm.scrambler_bb(payload_bits)
        self.connect(decoder, descrambler)
        return descrambler, (flatten, demapper, decoder, deinterleavers)

    def _build_msc_path(self):
        tp = self.tp
        total_cells = tp.msc().N_MUX() * tp.ofdm().M_TF()
        flatten = blocks.vector_to_stream(gr.sizeof_gr_complex, total_cells)
        cell_deinterleaver = drm.deinterleaver_cc(
            tp.msc().cell_interl_seq(), tp.cfg().long_interl(),
            drm.INTL_DEPTH_DRMPLUS if tp.cfg().RM() == 4 else drm.INTL_DEPTH_DRM)
        levels = tp.msc().n_levels_mlc()
        demapper = drm.qam_demapper_cb(
            self._qam_table(tp, "MSC"), tp.msc().mod_order(), levels)
        decoder = drm.mlc_decoder_bb(tp, "MSC")
        self.connect((self.cell_demapper, 0), flatten, cell_deinterleaver, demapper)
        self.msc_bit_deinterleavers = []
        for level, sequence in enumerate(self._bit_sequences(tp, "MSC", levels)):
            if sequence is None:
                self.connect((demapper, level), (decoder, level))
            else:
                block = drm.deinterleaver_bb(sequence)
                self.msc_bit_deinterleavers.append(block)
                self.connect((demapper, level), block, (decoder, level))
        descrambler = drm.scrambler_bb(tp.msc().L_MUX())
        frame = blocks.stream_to_vector(gr.sizeof_char, tp.msc().L_MUX())
        self.connect(decoder, descrambler, frame, self)
        self.msc_blocks = (flatten, cell_deinterleaver, demapper, decoder,
                           descrambler, frame)

    def _build_control_path(self, channel, port, cells, payload_bits):
        descrambler, chain = self._channel_decoder(
            (self.cell_demapper, port), channel, cells, payload_bits)
        sink = blocks.null_sink(gr.sizeof_char)
        self.connect(descrambler, sink)
        setattr(self, channel.lower() + "_blocks", chain + (descrambler, sink))


class drm_receiver_ccf(gr.hier_block2):
    """Complete channel receiver plus an external FAAD2 DRM audio decoder."""

    def __init__(self, tp):
        gr.hier_block2.__init__(
            self, "DRM Audio Receiver",
            gr.io_signature(1, 1, gr.sizeof_gr_complex),
            gr.io_signature(1, 1, gr.sizeof_float),
        )
        self.channel_receiver = drm_channel_receiver_ccb(tp)
        self.audio_decoder = drm.audio_decoder_sb(tp)
        self.connect(self, self.channel_receiver, self.audio_decoder, self)
