set(SRC 3d_array.cc blas_intfc.cc blas_intfc23.cc cc_excited.cc cc_wfn.cc ci_wfn.cc david.cc dirprd_block.cc dot_block.cc dx_read.cc dx_write.cc fill_sym_matrix.cc filter.cc invert.cc lapack_intfc.cc mat_in.cc mat_print.cc newmm_rking.cc normalize.cc orient_fragment.cc pople.cc probabil.cc ras_set.cc reorder_qt.cc rootfind.cc rotate_vecs.cc schmidt.cc schmidt_add.cc slaterdset.cc solve_pep.cc sort.cc strncpy.cc timer.cc v_3.cc zmat_point.cc)
add_library(qt ${SRC})
add_dependencies(qt mints)
