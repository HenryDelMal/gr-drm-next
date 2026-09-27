install(FILES
    api.h
    ${PROJECT_SOURCE_DIR}/lib/drm_global_constants.h
    ${PROJECT_SOURCE_DIR}/lib/drm_tables.h
    ${PROJECT_SOURCE_DIR}/lib/drm_config.h
    ${PROJECT_SOURCE_DIR}/lib/drm_params.h
    ${PROJECT_SOURCE_DIR}/lib/drm_transm_params.h
    ${PROJECT_SOURCE_DIR}/lib/drm_util.h
    ${PROJECT_SOURCE_DIR}/lib/wavfile_drm.h
    scrambler_bb.h audio_encoder_sb.h audio_decoder_sb.h ofdm_demodulator_cc.h cell_demapping_cc.h
    generate_fac_b.h generate_sdc_b.h add_tailbits_bb.h punct_bb.h
    interleaver_bb.h deinterleaver_bb.h interleaver_cc.h deinterleaver_cc.h partitioning_bb.h cell_mapping_cc.h
    qam_map_bc.h qam_demapper_cb.h m3ufile_source_f.h
    DESTINATION include/drm)
