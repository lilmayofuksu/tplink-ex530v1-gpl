add_executable(hello hello.c)
target_link_libraries(hello mbedtls)

add_executable(sha256 sha256.c)
target_link_libraries(sha256 mbedtls)

add_executable(generic_sum generic_sum.c)
target_link_libraries(generic_sum mbedtls)

install(TARGETS hello generic_sum sha256
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
