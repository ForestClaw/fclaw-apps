message(${PROJECT_SOURCE_DIR})
# set(rp ${PROJECT_SOURCE_DIR}/applications/clawpack/euler/3d/rp)
# set(all2d ${PROJECT_SOURCE_DIR}/applications/clawpack/advection/2d/all)

set(sgn ${PROJECT_SOURCE_DIR}/applications/tsunami/sgn)
set(rp ${PROJECT_SOURCE_DIR}/applications/tsunami/rp_tsunami)


add_library(tsunami_f OBJECT 
  setprob.f
  bathy.f90
  user_4.6/qinit.f90
  user_4.6/setaux.f90
  ${sgn}/fort_4.6/sgn_apply_bc.f90
  ${sgn}/fort_4.6/sgn_eval_bc.f90
  ${sgn}/fort_4.6/sgn_fort_rhs.f90
  ${sgn}/fort_4.6/sgn_update_q.f90
  ${rp}/swe_simple_riemann.f90
  ${rp}/rpn2_tsunami.f90
  ${rp}/rpt2_tsunami.f90
)

add_executable(tsunami 
  tsunami.cpp 
  tsunami_user.cpp 
  tsunami_options.c
  ${sgn}/sgn_operator.cpp
  ${sgn}/sgn_patch_operator.cpp
  ${sgn}/sgn_options.c
  ${sgn}/sgn_run.c
  ${sgn}/sgn.c
  $<TARGET_OBJECTS:tsunami_f>
)

target_include_directories(tsunami PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR} 
)

target_link_libraries(tsunami PRIVATE 
 FORESTCLAW::CLAWPACK4.6
 FORESTCLAW::FC2D_THUNDEREGG
)

# add_test(NAME clawpack_euler_3d_msh COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/regressions.sh WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
# set_tests_properties(clawpack_euler_3d_msh  PROPERTIES ENVIRONMENT "${FCLAW_TEST_ENVIRONMENT}")