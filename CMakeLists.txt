cmake_minimum_required (VERSION 3.12)

project (echo_mod_dyn)

include (platform)
initialize_platform ()

include (platform/nk)

find_package (ns REQUIRED)

nk_build_idl_files (echo_mod_dyn_idl_files                                                  NK_MODULE "echo_mod_dyn" IDL "resources/IDynPing.idl")
nk_build_edl_files (echo_mod_dyn_server_edl_files   IDL_TARGET echo_mod_dyn_idl_files       NK_MODULE "echo_mod_dyn" EDL "resources/DynServer.edl")
nk_build_edl_files (echo_mod_dyn_client_edl_files                                           NK_MODULE "echo_mod_dyn" EDL "resources/DynClient.edl")

# Recommended compiler flags against vulnerabilities.
set(VULN_LDFLAGS "\
    -Wl,-z,noexecstack \
    -Wl,-z,separate-code \
    -Wl,-z,now")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -Wall -Wextra -Wconversion \
    -fPIE -pie -D_FORTIFY_SOURCE=2 -O2 \
    -fstack-protector-strong -fstack-clash-protection \
    -mbranch-protection=standard \
    -Wsign-conversion -Wformat=2 -Wformat-security -Werror=format-security \
    -fsanitize=undefined -fsanitize-undefined-trap-on-error")

add_subdirectory (dynserver)
add_subdirectory (dynclient)
add_subdirectory (einit)
