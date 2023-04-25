project (einit)

include (platform/image)

project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

set (ENTITIES DynClient DynServer ${ns_ENTITY})

# KasperskyOS image for the target hardware platform.
build_kos_hw_image (kos-image
        EINIT_ENTITY EinitHw
        CONNECTIONS_CFG "src/init.yaml.in"
        SECURITY_PSL "src/security.psl"
        ${ENTITIES})

# KasperskyOS image for QEMU with simulation targets (sim, gdbsim, gdb).
build_kos_qemu_image (kos-qemu-image
        EINIT_ENTITY EinitQemu
        CONNECTIONS_CFG "src/init.yaml.in"
        SECURITY_PSL "src/security.psl"
        ${ENTITIES})

set_target_properties (EinitHw PROPERTIES LINK_FLAGS ${VULN_LDFLAGS})
set_target_properties (EinitQemu PROPERTIES LINK_FLAGS ${VULN_LDFLAGS})
