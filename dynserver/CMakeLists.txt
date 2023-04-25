project (dynserver)

include (platform/nk)

project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

include_directories(${CMAKE_SOURCE_DIR}/resources/include)
add_executable          (DynServer "src/dynserver.c")
add_dependencies        (DynServer echo_mod_dyn_server_edl_files)
set_target_properties   (DynServer PROPERTIES LINK_FLAGS ${VULN_LDFLAGS})
