set(drm_sources
    drm_util.cc drm_transm_params.cc drm_params.cc drm_config.cc
    drm_tables_mapping.cc drm_tables_params.cc scrambler_bb_impl.cc
    audio_encoder_sb_impl.cc audio_decoder_sb_impl.cc ofdm_demodulator_cc_impl.cc cell_demapping_cc_impl.cc generate_fac_b_impl.cc generate_sdc_b_impl.cc
    add_tailbits_bb_impl.cc punct_bb_impl.cc interleaver_bb_impl.cc
    interleaver_cc_impl.cc partitioning_bb_impl.cc cell_mapping_cc_impl.cc
    qam_map_bc_impl.cc qam_demapper_cb_impl.cc m3ufile_source_f_impl.cc wavfile_drm.cc)

add_library(gnuradio-drm SHARED ${drm_sources})
target_include_directories(gnuradio-drm
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gnuradio-drm BEFORE PRIVATE ${Faac_INCLUDE_DIR})
target_compile_options(gnuradio-drm PRIVATE "-I${Faac_INCLUDE_DIR}")
target_include_directories(gnuradio-drm BEFORE PRIVATE ${Faad_INCLUDE_DIR})
target_compile_options(gnuradio-drm PRIVATE "-I${Faad_INCLUDE_DIR}")
target_link_libraries(gnuradio-drm
    PUBLIC gnuradio::gnuradio-runtime gnuradio::gnuradio-fft
    PRIVATE ${Faac_LIBRARIES} ${Faad_LIBRARIES})
set_target_properties(gnuradio-drm PROPERTIES DEFINE_SYMBOL "gnuradio_drm_EXPORTS")
target_compile_definitions(gnuradio-drm PRIVATE HAVE_CONFIG_H)

install(TARGETS gnuradio-drm
    LIBRARY DESTINATION ${GR_LIBRARY_DIR}
    ARCHIVE DESTINATION ${GR_LIBRARY_DIR}
    RUNTIME DESTINATION ${GR_RUNTIME_DIR})
