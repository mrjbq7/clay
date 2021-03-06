set(FIX_SOURCES
    fix.clay
    fix/v0_0.clay
)

set(BINDGEN_SOURCES
    bindgen.clay
)

if (WIN32)
    set(EXE .exe)
    if(MSVC)
        set(LIBCLANG_LDFLAGS -Wl,${LLVM_DIR}/lib/libclang.lib)
        set(LIBFIX_LDFLAGS -Wl,/stack:4194304)
    else()
        set(LIBCLANG_LDFLAGS -L${LLVM_DIR}/lib -Wl,-rpath,${LLVM_DIR}/lib -llibclang.dll)
        set(LIBFIX_LDFLAGS )
    endif()
else (WIN32)
    set(EXE )
    set(LIBCLANG_LDFLAGS -L${LLVM_LIBDIR} -Wl,-rpath,${LLVM_LIBDIR} -lclang)
    set(LIBFIX_LDFLAGS )
endif (WIN32)

if(BUILD_FIX)
    add_custom_command(OUTPUT ${clay_BINARY_DIR}/tools/clay-fix${EXE}
        DEPENDS
            clay
            ${FIX_SOURCES}
        COMMAND clay
            -I${clay_SOURCE_DIR}/lib-clay
            -I${clay_SOURCE_DIR}/tools
            -O1
            -o ${clay_BINARY_DIR}/tools/clay-fix${EXE}
            ${LIBFIX_LDFLAGS}
            ${clay_SOURCE_DIR}/tools/fix.clay)

    add_custom_target(clay-fix ALL DEPENDS ${clay_BINARY_DIR}/tools/clay-fix${EXE})
    install(PROGRAMS ${clay_BINARY_DIR}/tools/clay-fix${EXE} DESTINATION bin)
endif(BUILD_FIX)

if(BUILD_BINDGEN)
    add_custom_command(OUTPUT ${clay_BINARY_DIR}/tools/clay-bindgen${EXE}
        DEPENDS
            clay
            ${BINDGEN_SOURCES}
        COMMAND clay
            -I${clay_SOURCE_DIR}/lib-clay
            -I${clay_SOURCE_DIR}/tools
            -o ${clay_BINARY_DIR}/tools/clay-bindgen${EXE}
            ${LIBCLANG_LDFLAGS}
            ${clay_SOURCE_DIR}/tools/bindgen.clay)

    add_custom_target(clay-bindgen ALL DEPENDS ${clay_BINARY_DIR}/tools/clay-bindgen${EXE})
    install(PROGRAMS ${clay_BINARY_DIR}/tools/clay-bindgen${EXE} DESTINATION bin)
endif(BUILD_BINDGEN)
