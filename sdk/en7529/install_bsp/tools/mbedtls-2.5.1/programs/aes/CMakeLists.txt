add_executable(aescrypt2 aescrypt2.c)
target_link_libraries(aescrypt2 mbedtls)

add_executable(aescrypt_secure_boot aescrypt_secure_boot.c)
target_link_libraries(aescrypt_secure_boot mbedtls)

add_executable(crypt_and_hash crypt_and_hash.c)
target_link_libraries(crypt_and_hash mbedtls)

install(TARGETS aescrypt2 crypt_and_hash aescrypt_secure_boot
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
