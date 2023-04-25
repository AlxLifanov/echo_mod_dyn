project (dynclient)

include (platform/nk)

project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

include_directories(${CMAKE_SOURCE_DIR}/resources/include)
add_executable          (DynClient "src/dynclient.c")
add_dependencies        (DynClient echo_mod_dyn_idl_files echo_mod_dyn_client_edl_files )
set_target_properties   (DynClient PROPERTIES LINK_FLAGS ${VULN_LDFLAGS})
