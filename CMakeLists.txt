cmake_minimum_required(VERSION 3.8)
project(CParty)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")

SET(CPARTY_CORE_SOURCES
  src/W_final.cc
  src/pseudo_loop.cc
  src/part_func.cc
  src/Result.cc
  src/s_energy_matrix.cc
  src/Hotspot.cc
  src/sparse_tree.cc
  src/dot_plot.cc
  src/mea.cc
  src/centroid.cc
  src/CPartyAPI.cc
  src/CPartyAPI_globals.cc
)

set(constraints_SOURCE
    src/ViennaRNA/constraints/constraints.c
    src/ViennaRNA/constraints/hard.c
    src/ViennaRNA/constraints/SHAPE.c
    src/ViennaRNA/constraints/soft.c
) 
set(datastructures_SOURCE
    src/ViennaRNA/datastructures/basic_datastructures.c
    src/ViennaRNA/datastructures/char_stream.c
    src/ViennaRNA/datastructures/lists.c
)
set(io_SOURCE
    src/ViennaRNA/io/file_formats.c
    src/ViennaRNA/io/io_utils.c
)
set(landscape_SOURCE
    src/ViennaRNA/landscape/move.c
)
set(loops_SOURCE
    src/ViennaRNA/loops/external_pf.c
    src/ViennaRNA/loops/external.c
    src/ViennaRNA/loops/hairpin.c
    src/ViennaRNA/loops/internal.c
    src/ViennaRNA/loops/multibranch.c
)
set(params_SOURCE
    src/ViennaRNA/params/default.c
    src/ViennaRNA/params/params.c
    src/ViennaRNA/params/io.c
)
set(utils_SOURCE
    src/ViennaRNA/utils/cpu.c
    src/ViennaRNA/utils/higher_order_functions.c
    src/ViennaRNA/utils/string_utils.c
    src/ViennaRNA/utils/structure_utils.c
    src/ViennaRNA/utils/utils.c
)
set(vienna_SOURCE
    ${constraints_SOURCE} ${datastructures_SOURCE}
    ${io_SOURCE} ${landscape_SOURCE}
    ${loops_SOURCE} ${params_SOURCE}
    ${utils_SOURCE}
    src/ViennaRNA/alphabet.c
    src/ViennaRNA/boltzmann_sampling.c
    src/ViennaRNA/centroid.c
    src/ViennaRNA/cofold.c
    src/ViennaRNA/commands.c
    src/ViennaRNA/dp_matrices.c
    src/ViennaRNA/equilibrium_probs.c
    src/ViennaRNA/eval.c
    src/ViennaRNA/fold_compound.c
    src/ViennaRNA/fold.c
    src/ViennaRNA/gquad.c
    src/ViennaRNA/grammar.c
    src/ViennaRNA/MEA.c
    src/ViennaRNA/mfe.c
    src/ViennaRNA/mm.c
    src/ViennaRNA/model.c
    src/ViennaRNA/part_func.c
    src/ViennaRNA/ribo.c
    src/ViennaRNA/sequence.c
    src/ViennaRNA/subopt.c
    src/ViennaRNA/unstructured_domains.c  
)

add_library(RNA STATIC ${vienna_SOURCE})
# configure_file(RNA.pc.in RNA.pc @ONLY)

target_include_directories(RNA PRIVATE .)

add_library(CPartyCore STATIC ${CPARTY_CORE_SOURCES})
target_link_libraries(CPartyCore PUBLIC RNA)
target_include_directories(CPartyCore PUBLIC src)
target_compile_definitions(CPartyCore PUBLIC CPARTY_API_BUILD)

add_executable(CParty src/CParty.cc src/cmdline.cc)

# Install CParty into the bin directory
install(TARGETS CParty RUNTIME DESTINATION bin)

target_link_libraries(CParty PRIVATE CPartyCore)

include_directories(src)

include(CTest)
if(BUILD_TESTING)
  add_test(
    NAME regression_matrix
    COMMAND ${CMAKE_SOURCE_DIR}/tests/regression_matrix.sh
            $<TARGET_FILE:CParty>
            ${CMAKE_SOURCE_DIR}/tests/baselines/regression
  )
  set_tests_properties(regression_matrix PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  add_test(
    NAME pseudo_loop_regression
    COMMAND ${CMAKE_SOURCE_DIR}/tests/regression_matrix.sh
            $<TARGET_FILE:CParty>
            ${CMAKE_SOURCE_DIR}/tests/baselines/regression
  )
  set_tests_properties(pseudo_loop_regression PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  add_test(
    NAME fixtures_schema
    COMMAND ${CMAKE_SOURCE_DIR}/tests/fixed_energy_fixtures.sh
            ${CMAKE_SOURCE_DIR}/tests/cparty_fixed_structure_energy/fixtures.tsv
  )
  set_tests_properties(fixtures_schema PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
