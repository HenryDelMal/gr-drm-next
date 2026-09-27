find_package(pybind11 REQUIRED)

pybind11_add_module(drm_python drm_python.cc)
target_include_directories(drm_python PRIVATE ${PROJECT_SOURCE_DIR}/lib
                                              ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(drm_python PRIVATE gnuradio-drm gnuradio::gnuradio-runtime)

add_custom_command(TARGET drm_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/test_modules/drm_next
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:drm_python>
            ${PROJECT_BINARY_DIR}/test_modules/drm_next/)

install(TARGETS drm_python DESTINATION ${GR_PYTHON_DIR}/drm_next COMPONENT pythonapi)
