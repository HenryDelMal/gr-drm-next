#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: DRM Full Audio Transceiver Loopback
# Author: HenryDelMal
# Description: Full experimental DRM transmitter and receiver loopback: Opus, MLC, OFDM, synchronization, channel decoding, and sound-card playback.
# GNU Radio version: 3.10.12.0

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import audio
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import fft
from gnuradio.fft import window
from gnuradio import gr
from gnuradio.filter import firdes
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
import drm_next as drm
import sip
import threading



class drm_audio_loopback(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "DRM Full Audio Transceiver Loopback", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("DRM Full Audio Transceiver Loopback")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("gnuradio/flowgraphs", "drm_audio_loopback")

        try:
            geometry = self.settings.value("geometry")
            if geometry:
                self.restoreGeometry(geometry)
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)
        self.flowgraph_started = threading.Event()

        ##################################################
        # Variables
        ##################################################
        self.tp = tp = drm.transm_params(1, 3, False, 0, 1, 0, 1, 1, 0, False, 24000, "gr-drm loopback", "Full DRM transmitter and receiver")
        self.tx_available_bitrate = tx_available_bitrate = (int(((tp.msc().L_MUX() - 400) // 160) * 400))
        self.opus_bitrate = opus_bitrate = 12000
        self.tx_used_bitrate = tx_used_bitrate = tx_available_bitrate if opus_bitrate == 0 else opus_bitrate
        self.tx_payload_utilization = tx_payload_utilization = (100.0 * tx_used_bitrate / tx_available_bitrate)
        self.tx_mode_status = tx_mode_status = 'f"RM {(\\"A\\", \\"B\\", \\"C\\", \\"D\\", \\"E\\")[tp.cfg().RM()]}, SO {tp.cfg().SO()}, MSC {16 if tp.cfg().msc_mapping() == 1 else 64}-QAM, SDC {4 if tp.cfg().sdc_mapping() == 1 else 16}-QAM, {\\"long\\" if tp.cfg().long_interl() else \\"short\\"} interleaver"'
        self.rx_pcm_status = rx_pcm_status = f"{tp.cfg().audio_samp_rate() / 1000:g} kHz mono float32"
        self.rx_detection_status = rx_detection_status = "MSC/SDC mapping + interleaver from FAC"
        self.rx_codec_status = rx_codec_status = "Opus (experimental)"
        self.rx_bitrate_status = rx_bitrate_status = tx_used_bitrate

        ##################################################
        # Blocks
        ##################################################

        self.tx_rx_tabs = Qt.QTabWidget()
        self.tx_rx_tabs_widget_0 = Qt.QWidget()
        self.tx_rx_tabs_layout_0 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.tx_rx_tabs_widget_0)
        self.tx_rx_tabs_grid_layout_0 = Qt.QGridLayout()
        self.tx_rx_tabs_layout_0.addLayout(self.tx_rx_tabs_grid_layout_0)
        self.tx_rx_tabs.addTab(self.tx_rx_tabs_widget_0, "Transmitter")
        self.tx_rx_tabs_widget_1 = Qt.QWidget()
        self.tx_rx_tabs_layout_1 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.tx_rx_tabs_widget_1)
        self.tx_rx_tabs_grid_layout_1 = Qt.QGridLayout()
        self.tx_rx_tabs_layout_1.addLayout(self.tx_rx_tabs_grid_layout_1)
        self.tx_rx_tabs.addTab(self.tx_rx_tabs_widget_1, "Receiver")
        self.top_grid_layout.addWidget(self.tx_rx_tabs, 0, 0, 2, 2)
        for r in range(0, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._tx_used_bitrate_tool_bar = Qt.QToolBar(self)

        if lambda x: f"{x / 1000:.1f} kbit/s":
            self._tx_used_bitrate_formatter = lambda x: f"{x / 1000:.1f} kbit/s"
        else:
            self._tx_used_bitrate_formatter = lambda x: str(x)

        self._tx_used_bitrate_tool_bar.addWidget(Qt.QLabel("Used Opus bitrate"))
        self._tx_used_bitrate_label = Qt.QLabel(str(self._tx_used_bitrate_formatter(self.tx_used_bitrate)))
        self._tx_used_bitrate_tool_bar.addWidget(self._tx_used_bitrate_label)
        self.tx_rx_tabs_grid_layout_0.addWidget(self._tx_used_bitrate_tool_bar, 2, 2, 1, 1)
        for r in range(2, 3):
            self.tx_rx_tabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(2, 3):
            self.tx_rx_tabs_grid_layout_0.setColumnStretch(c, 1)
        self._tx_payload_utilization_tool_bar = Qt.QToolBar(self)

        if lambda x: f"{x:.1f} %":
            self._tx_payload_utilization_formatter = lambda x: f"{x:.1f} %"
        else:
            self._tx_payload_utilization_formatter = lambda x: eng_notation.num_to_str(x)

        self._tx_payload_utilization_tool_bar.addWidget(Qt.QLabel("Payload utilization"))
        self._tx_payload_utilization_label = Qt.QLabel(str(self._tx_payload_utilization_formatter(self.tx_payload_utilization)))
        self._tx_payload_utilization_tool_bar.addWidget(self._tx_payload_utilization_label)
        self.tx_rx_tabs_grid_layout_0.addWidget(self._tx_payload_utilization_tool_bar, 2, 3, 1, 1)
        for r in range(2, 3):
            self.tx_rx_tabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(3, 4):
            self.tx_rx_tabs_grid_layout_0.setColumnStretch(c, 1)
        self._tx_mode_status_tool_bar = Qt.QToolBar(self)

        if None:
            self._tx_mode_status_formatter = None
        else:
            self._tx_mode_status_formatter = lambda x: str(x)

        self._tx_mode_status_tool_bar.addWidget(Qt.QLabel("DRM waveform"))
        self._tx_mode_status_label = Qt.QLabel(str(self._tx_mode_status_formatter(self.tx_mode_status)))
        self._tx_mode_status_tool_bar.addWidget(self._tx_mode_status_label)
        self.tx_rx_tabs_grid_layout_0.addWidget(self._tx_mode_status_tool_bar, 2, 0, 1, 1)
        for r in range(2, 3):
            self.tx_rx_tabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.tx_rx_tabs_grid_layout_0.setColumnStretch(c, 1)
        self._tx_available_bitrate_tool_bar = Qt.QToolBar(self)

        if lambda x: f"{x / 1000:.1f} kbit/s":
            self._tx_available_bitrate_formatter = lambda x: f"{x / 1000:.1f} kbit/s"
        else:
            self._tx_available_bitrate_formatter = lambda x: str(x)

        self._tx_available_bitrate_tool_bar.addWidget(Qt.QLabel("Available Opus payload"))
        self._tx_available_bitrate_label = Qt.QLabel(str(self._tx_available_bitrate_formatter(self.tx_available_bitrate)))
        self._tx_available_bitrate_tool_bar.addWidget(self._tx_available_bitrate_label)
        self.tx_rx_tabs_grid_layout_0.addWidget(self._tx_available_bitrate_tool_bar, 2, 1, 1, 1)
        for r in range(2, 3):
            self.tx_rx_tabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(1, 2):
            self.tx_rx_tabs_grid_layout_0.setColumnStretch(c, 1)
        self.transmitter_spectrum = qtgui.freq_sink_c(
            2048, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            0, #fc
            tp.ofdm().fs_soundcard(), #bw
            "DRM Transmit Baseband FFT", #name
            1,
            None # parent
        )
        self.transmitter_spectrum.set_update_time(0.10)
        self.transmitter_spectrum.set_y_axis((-120), 10)
        self.transmitter_spectrum.set_y_label('Relative power', 'dB')
        self.transmitter_spectrum.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.transmitter_spectrum.enable_autoscale(False)
        self.transmitter_spectrum.enable_grid(True)
        self.transmitter_spectrum.set_fft_average(0.2)
        self.transmitter_spectrum.enable_axis_labels(True)
        self.transmitter_spectrum.enable_control_panel(False)
        self.transmitter_spectrum.set_fft_window_normalized(False)



        labels = ['DRM baseband', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.transmitter_spectrum.set_line_label(i, "Data {0}".format(i))
            else:
                self.transmitter_spectrum.set_line_label(i, labels[i])
            self.transmitter_spectrum.set_line_width(i, widths[i])
            self.transmitter_spectrum.set_line_color(i, colors[i])
            self.transmitter_spectrum.set_line_alpha(i, alphas[i])

        self._transmitter_spectrum_win = sip.wrapinstance(self.transmitter_spectrum.qwidget(), Qt.QWidget)
        self.tx_rx_tabs_grid_layout_0.addWidget(self._transmitter_spectrum_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.tx_rx_tabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.tx_rx_tabs_grid_layout_0.setColumnStretch(c, 1)
        self.soundcard_sink = audio.sink(tp.cfg().audio_samp_rate(), "", True)
        self.sdc_scrambler = drm.scrambler_bb(tp.sdc().L())
        self.sdc_mlc = drm.make_mlc("SDC", tp)
        self.sdc_generator = drm.generate_sdc_b(tp)
        self._rx_pcm_status_tool_bar = Qt.QToolBar(self)

        if None:
            self._rx_pcm_status_formatter = None
        else:
            self._rx_pcm_status_formatter = lambda x: str(x)

        self._rx_pcm_status_tool_bar.addWidget(Qt.QLabel("Decoded PCM"))
        self._rx_pcm_status_label = Qt.QLabel(str(self._rx_pcm_status_formatter(self.rx_pcm_status)))
        self._rx_pcm_status_tool_bar.addWidget(self._rx_pcm_status_label)
        self.tx_rx_tabs_grid_layout_1.addWidget(self._rx_pcm_status_tool_bar, 1, 2, 1, 1)
        for r in range(1, 2):
            self.tx_rx_tabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(2, 3):
            self.tx_rx_tabs_grid_layout_1.setColumnStretch(c, 1)
        self._rx_detection_status_tool_bar = Qt.QToolBar(self)

        if None:
            self._rx_detection_status_formatter = None
        else:
            self._rx_detection_status_formatter = lambda x: str(x)

        self._rx_detection_status_tool_bar.addWidget(Qt.QLabel("Detection"))
        self._rx_detection_status_label = Qt.QLabel(str(self._rx_detection_status_formatter(self.rx_detection_status)))
        self._rx_detection_status_tool_bar.addWidget(self._rx_detection_status_label)
        self.tx_rx_tabs_grid_layout_1.addWidget(self._rx_detection_status_tool_bar, 1, 3, 1, 1)
        for r in range(1, 2):
            self.tx_rx_tabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(3, 4):
            self.tx_rx_tabs_grid_layout_1.setColumnStretch(c, 1)
        self._rx_codec_status_tool_bar = Qt.QToolBar(self)

        if None:
            self._rx_codec_status_formatter = None
        else:
            self._rx_codec_status_formatter = lambda x: str(x)

        self._rx_codec_status_tool_bar.addWidget(Qt.QLabel("Codec"))
        self._rx_codec_status_label = Qt.QLabel(str(self._rx_codec_status_formatter(self.rx_codec_status)))
        self._rx_codec_status_tool_bar.addWidget(self._rx_codec_status_label)
        self.tx_rx_tabs_grid_layout_1.addWidget(self._rx_codec_status_tool_bar, 1, 0, 1, 1)
        for r in range(1, 2):
            self.tx_rx_tabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 1):
            self.tx_rx_tabs_grid_layout_1.setColumnStretch(c, 1)
        self._rx_bitrate_status_tool_bar = Qt.QToolBar(self)

        if lambda x: f"{x / 1000:.1f} kbit/s target":
            self._rx_bitrate_status_formatter = lambda x: f"{x / 1000:.1f} kbit/s target"
        else:
            self._rx_bitrate_status_formatter = lambda x: str(x)

        self._rx_bitrate_status_tool_bar.addWidget(Qt.QLabel("Opus bitrate"))
        self._rx_bitrate_status_label = Qt.QLabel(str(self._rx_bitrate_status_formatter(self.rx_bitrate_status)))
        self._rx_bitrate_status_tool_bar.addWidget(self._rx_bitrate_status_label)
        self.tx_rx_tabs_grid_layout_1.addWidget(self._rx_bitrate_status_tool_bar, 1, 1, 1, 1)
        for r in range(1, 2):
            self.tx_rx_tabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(1, 2):
            self.tx_rx_tabs_grid_layout_1.setColumnStretch(c, 1)
        self.rx_auto_receiver = drm.drm_opus_auto_receiver_ccf(tp.cfg().RM(), tp.cfg().SO(), tp.cfg().msc_prot_level_2(), tp.cfg().audio_samp_rate())
        self.msc_scrambler = drm.scrambler_bb(tp.msc().L_MUX())
        self.msc_mlc = drm.make_mlc("MSC", tp)
        self.msc_interleaver = drm.interleaver_cc(tp.msc().cell_interl_seq(), tp.cfg().long_interl(), drm.INTL_DEPTH_DRM)
        self.ifft = fft.fft_vcc(tp.ofdm().nfft(), False, [], True, 1)
        self.fac_scrambler = drm.scrambler_bb(tp.fac().L())
        self.fac_mlc = drm.make_mlc("FAC", tp)
        self.fac_generator = drm.generate_fac_b(tp)
        self.decoded_audio_preview = qtgui.time_sink_f(
            2048, #size
            tp.cfg().audio_samp_rate(), #samp_rate
            "DRM Receiver Decoded Audio", #name
            1, #number of inputs
            None # parent
        )
        self.decoded_audio_preview.set_update_time(0.10)
        self.decoded_audio_preview.set_y_axis(-1, 1)

        self.decoded_audio_preview.set_y_label('Amplitude', "")

        self.decoded_audio_preview.enable_tags(False)
        self.decoded_audio_preview.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.decoded_audio_preview.enable_autoscale(False)
        self.decoded_audio_preview.enable_grid(True)
        self.decoded_audio_preview.enable_axis_labels(True)
        self.decoded_audio_preview.enable_control_panel(False)
        self.decoded_audio_preview.enable_stem_plot(False)


        labels = ['Decoded audio', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(1):
            if len(labels[i]) == 0:
                self.decoded_audio_preview.set_line_label(i, "Data {0}".format(i))
            else:
                self.decoded_audio_preview.set_line_label(i, labels[i])
            self.decoded_audio_preview.set_line_width(i, widths[i])
            self.decoded_audio_preview.set_line_color(i, colors[i])
            self.decoded_audio_preview.set_line_style(i, styles[i])
            self.decoded_audio_preview.set_line_marker(i, markers[i])
            self.decoded_audio_preview.set_line_alpha(i, alphas[i])

        self._decoded_audio_preview_win = sip.wrapinstance(self.decoded_audio_preview.qwidget(), Qt.QWidget)
        self.tx_rx_tabs_grid_layout_1.addWidget(self._decoded_audio_preview_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.tx_rx_tabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 1):
            self.tx_rx_tabs_grid_layout_1.setColumnStretch(c, 1)
        self.cyclic_prefix = digital.ofdm_cyclic_prefixer(
            tp.ofdm().nfft(),
            tp.ofdm().nfft() + tp.ofdm().n_cp(),
            0,
            "")
        self.constellation_preview = qtgui.const_sink_c(
            2048, #size
            "DRM Transmit Constellation", #name
            1, #number of inputs
            None # parent
        )
        self.constellation_preview.set_update_time(0.10)
        self.constellation_preview.set_y_axis((-2), 2)
        self.constellation_preview.set_x_axis((-2), 2)
        self.constellation_preview.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, "")
        self.constellation_preview.enable_autoscale(False)
        self.constellation_preview.enable_grid(True)
        self.constellation_preview.enable_axis_labels(True)


        labels = ["DRM carriers", '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        styles = [0, 0, 0, 0, 0,
            0, 0, 0, 0, 0]
        markers = [0, 0, 0, 0, 0,
            0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.constellation_preview.set_line_label(i, "Data {0}".format(i))
            else:
                self.constellation_preview.set_line_label(i, labels[i])
            self.constellation_preview.set_line_width(i, widths[i])
            self.constellation_preview.set_line_color(i, colors[i])
            self.constellation_preview.set_line_style(i, styles[i])
            self.constellation_preview.set_line_marker(i, markers[i])
            self.constellation_preview.set_line_alpha(i, alphas[i])

        self._constellation_preview_win = sip.wrapinstance(self.constellation_preview.qwidget(), Qt.QWidget)
        self.tx_rx_tabs_grid_layout_0.addWidget(self._constellation_preview_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.tx_rx_tabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.tx_rx_tabs_grid_layout_0.setColumnStretch(c, 1)
        self.cell_mapper = drm.cell_mapping_cc(tp, (tp.msc().N_MUX() * tp.ofdm().M_TF() * 8, tp.sdc().N() * 8, tp.fac().N() * tp.ofdm().M_TF() * 8))
        self.carrier_vector_to_stream = blocks.vector_to_stream(gr.sizeof_gr_complex*1, tp.ofdm().nfft())
        self.blocks_wavfile_source_0 = blocks.wavfile_source('/Users/henry/ecdc/27f_biobio_24k_mono.wav', True)
        self.audio_encoder = drm.opus_audio_encoder_fb(tp, opus_bitrate, False, "audio", "auto", "auto", 10, False)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.audio_encoder, 0), (self.msc_scrambler, 0))
        self.connect((self.blocks_wavfile_source_0, 0), (self.audio_encoder, 0))
        self.connect((self.carrier_vector_to_stream, 0), (self.constellation_preview, 0))
        self.connect((self.cell_mapper, 0), (self.carrier_vector_to_stream, 0))
        self.connect((self.cell_mapper, 0), (self.ifft, 0))
        self.connect((self.cyclic_prefix, 0), (self.rx_auto_receiver, 0))
        self.connect((self.cyclic_prefix, 0), (self.transmitter_spectrum, 0))
        self.connect((self.fac_generator, 0), (self.fac_scrambler, 0))
        self.connect((self.fac_mlc, 0), (self.cell_mapper, 2))
        self.connect((self.fac_scrambler, 0), (self.fac_mlc, 0))
        self.connect((self.ifft, 0), (self.cyclic_prefix, 0))
        self.connect((self.msc_interleaver, 0), (self.cell_mapper, 0))
        self.connect((self.msc_mlc, 0), (self.msc_interleaver, 0))
        self.connect((self.msc_scrambler, 0), (self.msc_mlc, 0))
        self.connect((self.rx_auto_receiver, 0), (self.decoded_audio_preview, 0))
        self.connect((self.rx_auto_receiver, 0), (self.soundcard_sink, 0))
        self.connect((self.sdc_generator, 0), (self.sdc_scrambler, 0))
        self.connect((self.sdc_mlc, 0), (self.cell_mapper, 1))
        self.connect((self.sdc_scrambler, 0), (self.sdc_mlc, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("gnuradio/flowgraphs", "drm_audio_loopback")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_tp(self):
        return self.tp

    def set_tp(self, tp):
        self.tp = tp

    def get_tx_available_bitrate(self):
        return self.tx_available_bitrate

    def set_tx_available_bitrate(self, tx_available_bitrate):
        self.tx_available_bitrate = tx_available_bitrate
        Qt.QMetaObject.invokeMethod(self._tx_available_bitrate_label, "setText", Qt.Q_ARG("QString", str(self._tx_available_bitrate_formatter(self.tx_available_bitrate))))
        self.set_tx_payload_utilization((100.0 * self.tx_used_bitrate / self.tx_available_bitrate))
        self.set_tx_used_bitrate(self.tx_available_bitrate if self.opus_bitrate == 0 else self.opus_bitrate)

    def get_opus_bitrate(self):
        return self.opus_bitrate

    def set_opus_bitrate(self, opus_bitrate):
        self.opus_bitrate = opus_bitrate
        self.set_tx_used_bitrate(self.tx_available_bitrate if self.opus_bitrate == 0 else self.opus_bitrate)

    def get_tx_used_bitrate(self):
        return self.tx_used_bitrate

    def set_tx_used_bitrate(self, tx_used_bitrate):
        self.tx_used_bitrate = tx_used_bitrate
        self.set_rx_bitrate_status(self.tx_used_bitrate)
        self.set_tx_payload_utilization((100.0 * self.tx_used_bitrate / self.tx_available_bitrate))
        Qt.QMetaObject.invokeMethod(self._tx_used_bitrate_label, "setText", Qt.Q_ARG("QString", str(self._tx_used_bitrate_formatter(self.tx_used_bitrate))))

    def get_tx_payload_utilization(self):
        return self.tx_payload_utilization

    def set_tx_payload_utilization(self, tx_payload_utilization):
        self.tx_payload_utilization = tx_payload_utilization
        Qt.QMetaObject.invokeMethod(self._tx_payload_utilization_label, "setText", Qt.Q_ARG("QString", str(self._tx_payload_utilization_formatter(self.tx_payload_utilization))))

    def get_tx_mode_status(self):
        return self.tx_mode_status

    def set_tx_mode_status(self, tx_mode_status):
        self.tx_mode_status = tx_mode_status
        Qt.QMetaObject.invokeMethod(self._tx_mode_status_label, "setText", Qt.Q_ARG("QString", str(self._tx_mode_status_formatter(self.tx_mode_status))))

    def get_rx_pcm_status(self):
        return self.rx_pcm_status

    def set_rx_pcm_status(self, rx_pcm_status):
        self.rx_pcm_status = rx_pcm_status
        Qt.QMetaObject.invokeMethod(self._rx_pcm_status_label, "setText", Qt.Q_ARG("QString", str(self._rx_pcm_status_formatter(self.rx_pcm_status))))

    def get_rx_detection_status(self):
        return self.rx_detection_status

    def set_rx_detection_status(self, rx_detection_status):
        self.rx_detection_status = rx_detection_status
        Qt.QMetaObject.invokeMethod(self._rx_detection_status_label, "setText", Qt.Q_ARG("QString", str(self._rx_detection_status_formatter(self.rx_detection_status))))

    def get_rx_codec_status(self):
        return self.rx_codec_status

    def set_rx_codec_status(self, rx_codec_status):
        self.rx_codec_status = rx_codec_status
        Qt.QMetaObject.invokeMethod(self._rx_codec_status_label, "setText", Qt.Q_ARG("QString", str(self._rx_codec_status_formatter(self.rx_codec_status))))

    def get_rx_bitrate_status(self):
        return self.rx_bitrate_status

    def set_rx_bitrate_status(self, rx_bitrate_status):
        self.rx_bitrate_status = rx_bitrate_status
        Qt.QMetaObject.invokeMethod(self._rx_bitrate_status_label, "setText", Qt.Q_ARG("QString", str(self._rx_bitrate_status_formatter(self.rx_bitrate_status))))




def main(top_block_cls=drm_audio_loopback, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()
    tb.flowgraph_started.set()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
