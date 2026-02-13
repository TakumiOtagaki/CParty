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

SET(SOURCE
  src/W_final.cc 
  src/pseudo_loop.cc 
  src/part_func.cc
  src/scfg/core.cc
  src/scfg/constraint_oracle.cc
  src/scfg/legacy_adapter.cc
  src/scfg/rules_pseudo_loop.cc
  src/scfg/rules_part_func.cc
  src/fixed_energy_api.cc
  src/CParty.cc
  src/cmdline.cc
  src/Result.cc	
  src/s_energy_matrix.cc	
  src/Hotspot.cc
  src/sparse_tree.cc
  src/dot_plot.cc
  src/mea.cc
  src/centroid.cc
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

add_executable(CParty ${SOURCE})

# Install CParty into the bin directory
install(TARGETS CParty RUNTIME DESTINATION bin)

target_link_libraries(CParty PRIVATE RNA)

include_directories(src)

include(CTest)
if(BUILD_TESTING)
  add_executable(multi_secstruct_schema_test test/multi_secstruct_schema_test.cc)
  add_executable(api_cli_density2_energy_alignment_test test/api_cli_density2_energy_alignment_test.cc)
  add_executable(
    api_structure_energy_validation_test
    test/api_structure_energy_validation_test.cc
    src/fixed_energy_api.cc
  )
  add_executable(
    fixed_energy_internal_test
    test/fixed_energy_internal_test.cc
    src/fixed_energy_api.cc
  )
  add_executable(
    can_pair_policy_test
    test/can_pair_policy_test.cc
    src/sparse_tree.cc
    src/scfg/constraint_oracle.cc
  )

  # Source-controlled smoke checks so ctest registration is reproducible from a fresh build.
  add_test(
    NAME source_tree_test_dir_exists
    COMMAND bash -lc "test -d '${CMAKE_SOURCE_DIR}/test'"
  )

  add_test(
    NAME source_tree_seed_dataset_exists
    COMMAND bash -lc "test -s '${CMAKE_SOURCE_DIR}/test/multi.secstruct'"
  )

  add_test(
    NAME multi_secstruct_schema_test
    COMMAND multi_secstruct_schema_test
            "${CMAKE_SOURCE_DIR}/test/multi.secstruct"
            "${CMAKE_SOURCE_DIR}/test/fixtures/seed_from_multi_secstruct.tsv"
  )

  add_test(
    NAME can_pair_policy
    COMMAND can_pair_policy_test
  )

  add_test(
    NAME api_structure_energy_validation
    COMMAND api_structure_energy_validation_test
  )

  add_test(
    NAME fixed_energy_internal
    COMMAND fixed_energy_internal_test
  )

  add_test(
    NAME api_cli_density2_energy_alignment
    COMMAND api_cli_density2_energy_alignment_test
            "${CMAKE_SOURCE_DIR}/test/baselines/fixed_energy_cli/density2_mfe.tsv"
            "$<TARGET_FILE:CParty>"
            "${CMAKE_SOURCE_DIR}/params/rna_DirksPierce09.par"
  )
endif()
