get_filename_component(LIBSSH_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

if (EXISTS "${LIBSSH_CMAKE_DIR}/CMakeCache.txt")
    # In build tree
    include(${LIBSSH_CMAKE_DIR}/libssh-build-tree-settings.cmake)
else()
    set(LIBSSH_INCLUDE_DIR /proj/mtk69551/tony/ws_tony.du_AP31/BBN_Linux/DEV/main/tclinux_phoenix/apps/public/libssh-0.7.5/build_install/include)
endif()

set(LIBSSH_LIBRARY /proj/mtk69551/tony/ws_tony.du_AP31/BBN_Linux/DEV/main/tclinux_phoenix/apps/public/libssh-0.7.5/build_install/lib/libssh.so)
set(LIBSSH_LIBRARIES /proj/mtk69551/tony/ws_tony.du_AP31/BBN_Linux/DEV/main/tclinux_phoenix/apps/public/libssh-0.7.5/build_install/lib/libssh.so)

set(LIBSSH_THREADS_LIBRARY /proj/mtk69551/tony/ws_tony.du_AP31/BBN_Linux/DEV/main/tclinux_phoenix/apps/public/libssh-0.7.5/build_install/lib/libssh.so)
