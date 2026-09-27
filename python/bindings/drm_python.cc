#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <drm/add_tailbits_bb.h>
#include <drm/audio_encoder_sb.h>
#include <drm/audio_decoder_sb.h>
#include <drm/ofdm_demodulator_cc.h>
#include <drm/cell_demapping_cc.h>
#include <drm/cell_mapping_cc.h>
#include <drm/generate_fac_b.h>
#include <drm/generate_sdc_b.h>
#include <drm/interleaver_bb.h>
#include <drm/interleaver_cc.h>
#include <drm/deinterleaver_bb.h>
#include <drm/deinterleaver_cc.h>
#include <drm/m3ufile_source_f.h>
#include <drm/partitioning_bb.h>
#include <drm/punct_bb.h>
#include <drm/qam_map_bc.h>
#include <drm/qam_demapper_cb.h>
#include <drm/mlc_decoder_bb.h>
#include <drm/pilot_equalizer_vcc.h>
#include <drm/scrambler_bb.h>
#include "drm_global_constants.h"
#include "drm_transm_params.h"

namespace py = pybind11;

template <size_t N>
std::vector<std::vector<float>> qam_table(const float (&table)[N][2])
{
    std::vector<std::vector<float>> result;
    for (const auto& row : table)
        result.push_back({ row[0], row[1] });
    return result;
}

PYBIND11_MODULE(drm_python, m)
{
    py::module_::import("gnuradio.gr");

    py::class_<tables>(m, "tables")
        .def_property_readonly("d_QAM64SM", [](const tables&) { return qam_table(tables::d_QAM64SM); })
        .def_property_readonly("d_QAM64HMsym", [](const tables&) { return qam_table(tables::d_QAM64HMsym); })
        .def_property_readonly("d_QAM64HMmix", [](const tables&) { return qam_table(tables::d_QAM64HMmix); })
        .def_property_readonly("d_QAM16", [](const tables&) { return qam_table(tables::d_QAM16); })
        .def_property_readonly("d_QAM4", [](const tables&) { return qam_table(tables::d_QAM4); });

    py::class_<config>(m, "config")
        .def("RM", &config::RM).def("SO", &config::SO).def("UEP", &config::UEP)
        .def("n_bytes_A", &config::n_bytes_A).def("text", &config::text)
        .def("msc_mapping", &config::msc_mapping)
        .def("msc_prot_level_1", &config::msc_prot_level_1)
        .def("msc_prot_level_2", &config::msc_prot_level_2)
        .def("sdc_mapping", &config::sdc_mapping).def("sdc_prot_level", &config::sdc_prot_level)
        .def("long_interl", &config::long_interl).def("audio_samp_rate", &config::audio_samp_rate)
        .def("station_label", &config::station_label).def("text_message", &config::text_message)
        .def("ptables", &config::ptables, py::return_value_policy::reference);

    py::class_<ofdm_params>(m, "ofdm_params")
        .def("nfft", &ofdm_params::nfft).def("n_cp", &ofdm_params::n_cp)
        .def("cp_ratio_enum", &ofdm_params::cp_ratio_enum)
        .def("cp_ratio_denom", &ofdm_params::cp_ratio_denom)
        .def("K_min", &ofdm_params::K_min).def("K_max", &ofdm_params::K_max)
        .def("n_unused", &ofdm_params::n_unused).def("N_S", &ofdm_params::N_S)
        .def("M_TF", &ofdm_params::M_TF).def("fs_soundcard", &ofdm_params::fs_soundcard);

    py::class_<channel_params>(m, "channel_params")
        .def("r_p", &channel_params::r_p).def("mod_order", &channel_params::mod_order)
        .def("bit_interl_seq_0_1", &channel_params::bit_interl_seq_0_1)
        .def("bit_interl_seq_0_2", &channel_params::bit_interl_seq_0_2);
    py::class_<control_chan_params, channel_params>(m, "control_chan_params")
        .def("L", &control_chan_params::L).def("N", &control_chan_params::N)
        .def("R_0", &control_chan_params::R_0).def("R_0_enum", &control_chan_params::R_0_enum)
        .def("R_0_denom", &control_chan_params::R_0_denom).def("M_total", &control_chan_params::M_total)
        .def("punct_pat_0", &control_chan_params::punct_pat_0)
        .def("punct_pat_tail_0", &control_chan_params::punct_pat_tail_0);
    py::class_<sdc_params, control_chan_params>(m, "sdc_params")
        .def("R_1", &sdc_params::R_1).def("R_1_enum", &sdc_params::R_1_enum)
        .def("R_1_denom", &sdc_params::R_1_denom).def("n_bytes_datafield", &sdc_params::n_bytes_datafield)
        .def("M_index", &sdc_params::M_index).def("n_levels_mlc", &sdc_params::n_levels_mlc)
        .def("punct_pat_1", &sdc_params::punct_pat_1)
        .def("punct_pat_tail_1", &sdc_params::punct_pat_tail_1)
        .def("bit_interl_seq_1_1", &sdc_params::bit_interl_seq_1_1)
        .def("bit_interl_seq_1_2", &sdc_params::bit_interl_seq_1_2);
    py::class_<fac_params, control_chan_params>(m, "fac_params");
    py::class_<msc_params, channel_params>(m, "msc_params")
        .def("L_MUX", &msc_params::L_MUX).def("L_1", &msc_params::L_1)
        .def("L_2", &msc_params::L_2).def("L_VSPP", &msc_params::L_VSPP)
        .def("N_MUX", &msc_params::N_MUX).def("N_1", &msc_params::N_1).def("N_2", &msc_params::N_2)
        .def("M_index", &msc_params::M_index).def("M_total", &msc_params::M_total)
        .def("n_levels_mlc", &msc_params::n_levels_mlc)
        .def("punct_pat_0_2", &msc_params::punct_pat_0_2)
        .def("punct_pat_tail_0_2", &msc_params::punct_pat_tail_0_2)
        .def("punct_pat_1_2", &msc_params::punct_pat_1_2)
        .def("punct_pat_tail_1_2", &msc_params::punct_pat_tail_1_2)
        .def("punct_pat_2_2", &msc_params::punct_pat_2_2)
        .def("punct_pat_tail_2_2", &msc_params::punct_pat_tail_2_2)
        .def("bit_interl_seq_1_1", &msc_params::bit_interl_seq_1_1)
        .def("bit_interl_seq_1_2", &msc_params::bit_interl_seq_1_2)
        .def("bit_interl_seq_2_1", &msc_params::bit_interl_seq_2_1)
        .def("bit_interl_seq_2_2", &msc_params::bit_interl_seq_2_2)
        .def("cell_interl_seq", &msc_params::cell_interl_seq);

    py::class_<transm_params>(m, "transm_params")
        .def(py::init<unsigned short, unsigned short, bool, unsigned int, unsigned short,
                      unsigned short, unsigned short, unsigned short, unsigned short, bool,
                      unsigned int, std::string, std::string>(),
             py::arg("RM") = 1, py::arg("SO") = 3, py::arg("UEP") = false,
             py::arg("n_bytes_A") = 0, py::arg("msc_mapping") = 2,
             py::arg("msc_prot_level_1") = 0, py::arg("msc_prot_level_2") = 0,
             py::arg("sdc_mapping") = 1, py::arg("sdc_prot_level") = 0,
             py::arg("long_interl") = true, py::arg("audio_samp_rate") = 12000,
             py::arg("station_label") = "gr-drm",
             py::arg("text_message") = "This is GNU Radio on DRM")
        .def("cfg", &transm_params::cfg).def("ofdm", &transm_params::ofdm)
        .def("msc", &transm_params::msc).def("sdc", &transm_params::sdc).def("fac", &transm_params::fac);

#define BIND_BLOCK(NAME, BASE, ...) \
    py::class_<gr::drm::NAME, BASE, \
               std::shared_ptr<gr::drm::NAME>>(m, #NAME).def(py::init(&gr::drm::NAME::make), __VA_ARGS__)
    BIND_BLOCK(scrambler_bb, gr::sync_block, py::arg("block_len"));
    BIND_BLOCK(add_tailbits_bb, gr::block, py::arg("vlen_in"), py::arg("n_tailbits"));
    BIND_BLOCK(punct_bb, gr::block, py::arg("punct_pat_1"), py::arg("punct_pat_2"),
               py::arg("vlen_in"), py::arg("vlen_out"), py::arg("num_tailbits"));
    BIND_BLOCK(interleaver_bb, gr::sync_block, py::arg("interl_seq"));
    BIND_BLOCK(interleaver_cc, gr::block, py::arg("interl_seq"), py::arg("long_interl"), py::arg("depth"));
    BIND_BLOCK(deinterleaver_bb, gr::sync_block, py::arg("sequence"));
    BIND_BLOCK(deinterleaver_cc, gr::block, py::arg("sequence"), py::arg("long_interleaving"), py::arg("depth"));
    BIND_BLOCK(partitioning_bb, gr::block, py::arg("vlen_in"), py::arg("vlen_out"));
    BIND_BLOCK(generate_fac_b, gr::sync_block, py::arg("tp"), py::keep_alive<1, 2>());
    BIND_BLOCK(generate_sdc_b, gr::sync_block, py::arg("tp"), py::keep_alive<1, 2>());
    BIND_BLOCK(audio_encoder_sb, gr::block, py::arg("tp"), py::keep_alive<1, 2>());
    BIND_BLOCK(audio_decoder_sb, gr::block, py::arg("tp"), py::keep_alive<1, 2>());
    BIND_BLOCK(ofdm_demodulator_cc, gr::block, py::arg("nfft"), py::arg("ncp"),
               py::arg("fft_shift") = true);
    BIND_BLOCK(cell_demapping_cc, gr::block, py::arg("tp"), py::keep_alive<1, 2>());
    BIND_BLOCK(mlc_decoder_bb, gr::block, py::arg("tp"), py::arg("channel_type"), py::keep_alive<1, 2>());
    BIND_BLOCK(pilot_equalizer_vcc, gr::sync_block, py::arg("tp"), py::keep_alive<1, 2>());
    BIND_BLOCK(cell_mapping_cc, gr::block, py::arg("tp"), py::arg("input_sizes"), py::keep_alive<1, 2>());
    BIND_BLOCK(m3ufile_source_f, gr::sync_block, py::arg("filename"), py::arg("tp"), py::keep_alive<1, 3>());
#undef BIND_BLOCK

    py::class_<gr::drm::qam_map_bc, gr::sync_decimator,
               std::shared_ptr<gr::drm::qam_map_bc>>(m, "qam_map_bc")
        .def(py::init([](const std::vector<std::vector<float>>& table, int bits, int vlen, int inputs) {
            if (table.size() > 8) throw py::value_error("QAM table has more than 8 rows");
            float native[8][2] = {};
            for (size_t i = 0; i < table.size(); ++i) {
                if (table[i].size() != 2) throw py::value_error("each QAM row must contain I and Q");
                native[i][0] = table[i][0]; native[i][1] = table[i][1];
            }
            return gr::drm::qam_map_bc::make(native, bits, vlen, inputs);
        }), py::arg("map_table"), py::arg("bits_per_symbol"), py::arg("vlen_out"), py::arg("n_inputs"));

    py::class_<gr::drm::qam_demapper_cb, gr::block,
               std::shared_ptr<gr::drm::qam_demapper_cb>>(m, "qam_demapper_cb")
        .def(py::init([](const std::vector<std::vector<float>>& table, int bits, int outputs) {
            if (table.size() > 8) throw py::value_error("QAM table has more than 8 rows");
            float native[8][2] = {};
            for (size_t i = 0; i < table.size(); ++i) {
                if (table[i].size() != 2) throw py::value_error("each QAM row must contain I and Q");
                native[i][0] = table[i][0]; native[i][1] = table[i][1];
            }
            return gr::drm::qam_demapper_cb::make(native, bits, outputs);
        }), py::arg("map_table"), py::arg("bits_per_symbol"), py::arg("n_outputs"));

    m.attr("INTL_DEPTH_DRM") = INTL_DEPTH_DRM;
    m.attr("INTL_DEPTH_DRMPLUS") = INTL_DEPTH_DRMPLUS;
    m.attr("DENOM_MOTHER_CODE") = DENOM_MOTHER_CODE;
    m.attr("N_TAILBITS") = N_TAILBITS;
}
