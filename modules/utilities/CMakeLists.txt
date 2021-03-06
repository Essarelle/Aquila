IF(UNIX)
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
  set(CUDA_NVCC_FLAGS "-std=c++11;--expt-relaxed-constexpr;${CUDA_NVCC_FLAGS}")
ENDIF()
aquila_declare_module(NAME utilities DEPENDS opencv_core aquila_core opencv_imgproc CXX_STANDARD 11)
set_target_properties(aquila_utilities PROPERTIES CXX_STANDARD_REQUIRED ON)
set(aquila_utilities_dep)
set(aquila_utilities_dep ${aquila_utilities_dep} opencv_core)
set(aquila_utilities_dep ${aquila_utilities_dep} aquila_core)
set(aquila_utilities_dep ${aquila_utilities_dep} opencv_imgproc)

if (GCCLIBATOMIC_FOUND AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
  set(aquila_utilities_dep ${aquila_utilities_dep} ${GCCLIBATOMIC_LIBRARY})
endif ()

target_link_libraries(aquila_utilities ${aquila_utilities_dep})
