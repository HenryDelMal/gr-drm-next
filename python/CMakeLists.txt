include(GrPython)
add_subdirectory(bindings)

gr_python_install(FILES
    __init__.py mlc_factory.py drm_mlc_4qam_bc.py drm_mlc_16qam_bc.py
    drm_mlc_64qam_sm_bc.py drm_receiver_ccf.py
    DESTINATION ${GR_PYTHON_DIR}/drm_next)

add_custom_target(copy_module_for_tests ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/test_modules/drm_next/)

include(GrTest)
set(GR_TEST_TARGET_DEPS gnuradio-drm drm_python copy_module_for_tests)
foreach(test
    qa_scrambler_bb qa_audio_encoder_sb qa_generate_fac_b qa_generate_sdc_b
    qa_add_tailbits_bb qa_punct_bb qa_interleaver_bb qa_interleaver_cc
    qa_partitioning_bb qa_cell_mapping_cc qa_qam_map_bc qa_mlc_decoder_bb
    qa_ofdm_synchronizer_cc qa_m3ufile_source_f)
    gr_add_test(${test} ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
endforeach()
