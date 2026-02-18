cmake_minimum_required(VERSION 3.8)
project(CParty)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")

SET(SOURCE
  src/W_final.cc 
  src/pseudo_loop.cc 
  src/pseudo_loop_utils.cc
  src/pseudo_loop_getters.cc
  src/pseudo_loop_backtrack/vpl.cc
  src/pseudo_loop_backtrack/vpr.cc
  src/pseudo_loop_backtrack/wi.cc
  src/pseudo_loop_backtrack/wmbw.cc
  src/pseudo_loop_backtrack/wmb.cc
  src/pseudo_loop_backtrack/wmbp.cc
  src/pseudo_loop_backtrack/wip.cc
  src/pseudo_loop_backtrack/be.cc
  src/pseudo_loop_backtrack/vp_preamble.cc
  src/pseudo_loop_backtrack/vp_case1.cc
  src/pseudo_loop_backtrack/vp_case2.cc
  src/pseudo_loop_backtrack/vp_case3.cc
  src/pseudo_loop_backtrack/vp_case4.cc
  src/pseudo_loop_backtrack/vp_case5.cc
  src/part_func_bpp.cc
  src/part_func_sample.cc
  src/part_func_energy.cc
  src/part_func_pk.cc
  src/part_func_core.cc
  src/part_func_rescale.cc
  src/part_func_driver.cc
  src/part_func_init.cc
  src/scfg/core.cc
  src/scfg/constraint_oracle.cc
  src/scfg/legacy_adapter.cc
  src/scfg/part_func_adapter.cc
  src/scfg/rules_api.cc
  src/scfg/rules_core/base.cc
  src/scfg/rules_core/basic_wv.cc
  src/scfg/rules_core/basic_vm.cc
  src/scfg/rules_core/basic_wm.cc
  src/scfg/rules_core/band_vp.cc
  src/scfg/rules_core/band_wmb.cc
  src/scfg/rules_core/band_be.cc
  src/scfg/rules_config.cc
  src/scfg/rules_debug.cc
  src/scfg/inside_fill/basic.cc
  src/scfg/inside_fill/band.cc
  src/scfg/inside_fill/core.cc
  src/scfg/rules_pseudo_loop.cc
  src/scfg/legacy_rules_part_func.cc
  src/scfg/rules_part_helpers.cc
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
  set(FIXED_ENERGY_TEST_SOURCES
    src/fixed_energy_api.cc
    src/sparse_tree.cc
    src/scfg/core.cc
    src/scfg/constraint_oracle.cc
    src/scfg/legacy_adapter.cc
    src/scfg/rules_api.cc
    src/scfg/rules_core/base.cc
    src/scfg/rules_core/basic_wv.cc
    src/scfg/rules_core/basic_vm.cc
    src/scfg/rules_core/basic_wm.cc
    src/scfg/rules_core/band_vp.cc
    src/scfg/rules_core/band_wmb.cc
    src/scfg/rules_core/band_be.cc
    src/scfg/legacy_rules_part_func.cc
  src/scfg/rules_part_helpers.cc
  )
  add_executable(multi_secstruct_schema_test test/multi_secstruct_schema_test.cc)
  add_executable(api_cli_density2_energy_alignment_test test/api_cli_density2_energy_alignment_test.cc)
  add_executable(
    api_structure_energy_validation_test
    test/api_structure_energy_validation_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_internal_test
    test/fixed_energy_internal_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_rollout_plan_test
    test/fixed_energy_rollout_plan_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_slice_a_test
    test/fixed_energy_slice_a_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_slice_b_test
    test/fixed_energy_slice_b_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_slice_c_test
    test/fixed_energy_slice_c_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_slice_d_test
    test/fixed_energy_slice_d_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_breakdown_test
    test/fixed_energy_breakdown_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_union_test
    test/fixed_energy_union_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_union_energy_tool
    test/fixed_energy_union_energy_tool.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  add_executable(
    fixed_energy_cli_mfe_energy_alignment_test
    test/fixed_energy_cli_mfe_energy_alignment_test.cc
    ${FIXED_ENERGY_TEST_SOURCES}
  )
  set(FIXED_ENERGY_TEST_TARGETS
    api_structure_energy_validation_test
    fixed_energy_internal_test
    fixed_energy_rollout_plan_test
    fixed_energy_slice_a_test
    fixed_energy_slice_b_test
    fixed_energy_slice_c_test
    fixed_energy_slice_d_test
    fixed_energy_breakdown_test
    fixed_energy_union_test
    fixed_energy_union_energy_tool
    fixed_energy_cli_mfe_energy_alignment_test
  )
  foreach(target_name ${FIXED_ENERGY_TEST_TARGETS})
    target_link_libraries(${target_name} PRIVATE RNA)
  endforeach()
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
    NAME fixed_energy_rollout_registry
    COMMAND fixed_energy_rollout_plan_test
  )

  add_test(
    NAME fixed_energy_slice_a
    COMMAND fixed_energy_slice_a_test
  )

  add_test(
    NAME fixed_energy_slice_b
    COMMAND fixed_energy_slice_b_test
  )

  add_test(
    NAME fixed_energy_slice_c
    COMMAND fixed_energy_slice_c_test
  )

  add_test(
    NAME fixed_energy_slice_d
    COMMAND fixed_energy_slice_d_test
  )

  add_test(
    NAME fixed_energy_breakdown
    COMMAND fixed_energy_breakdown_test
  )

  add_test(
    NAME fixed_energy_union
    COMMAND fixed_energy_union_test
  )

  add_test(
    NAME fixed_energy_union_legacy_alignment
    COMMAND bash -lc
            "LEGACY_BIN=\"${CMAKE_SOURCE_DIR}/worktree_legacy/build/CParty\"; \
             if [[ -n \"$CPARTY_LEGACY_BIN\" ]]; then LEGACY_BIN=\"$CPARTY_LEGACY_BIN\"; fi; \
             \"${CMAKE_SOURCE_DIR}/test/tools/fixed_energy_union_legacy_alignment.sh\" \
             \"$LEGACY_BIN\" \
             \"$<TARGET_FILE:fixed_energy_union_energy_tool>\" \
             \"${CMAKE_SOURCE_DIR}/params/rna_DirksPierce09.par\" \
             20260217 100 30,50,80"
  )

  add_test(
    NAME fixed_energy_cli_mfe_energy_alignment
    COMMAND fixed_energy_cli_mfe_energy_alignment_test
            "${CMAKE_SOURCE_DIR}/test/baselines/fixed_energy_cli/density2_mfe_large.tsv"
  )

  add_test(
    NAME api_cli_density2_energy_alignment
    COMMAND api_cli_density2_energy_alignment_test
            "${CMAKE_SOURCE_DIR}/test/baselines/fixed_energy_cli/density2_mfe.tsv"
            "$<TARGET_FILE:CParty>"
            "${CMAKE_SOURCE_DIR}/params/rna_DirksPierce09.par"
  )

  add_test(
    NAME api_cli_stdout_legacy_alignment
    COMMAND bash -lc
            "LEGACY_BIN=\"${CMAKE_SOURCE_DIR}/worktree_legacy/build/CParty\"; \
             if [[ -n \"$CPARTY_LEGACY_BIN\" ]]; then LEGACY_BIN=\"$CPARTY_LEGACY_BIN\"; fi; \
             \"${CMAKE_SOURCE_DIR}/test/tools/compare_cli_stdout.sh\" \
             \"$LEGACY_BIN\" \
             \"$<TARGET_FILE:CParty>\" \
             20260213 200 30,50,80"
  )
endif()
