set(PLUGIN_WEBKITBROWSER_COOKIE_JAR_CRYPTO_IMPLEMENTATION "CookieJarCryptoExample.h" CACHE STRING "Implementation of cookie jar encryption/decryption routines." )
set(PLUGIN_WEBKITBROWSER_COOKIE_JAR_CRYPTO_LIBS "" CACHE STRING "Additional linker options that are needed to build specified cookie jar crypto implementation.")

target_include_directories(${PLUGIN_WEBKITBROWSER_IMPLEMENTATION}
    PRIVATE
        CookieJarCrypto)

target_compile_definitions(${PLUGIN_WEBKITBROWSER_IMPLEMENTATION}
    PRIVATE
        COOKIE_JAR_CRYPTO_IMPLEMENTATION="${PLUGIN_WEBKITBROWSER_COOKIE_JAR_CRYPTO_IMPLEMENTATION}")

if(PLUGIN_WEBKITBROWSER_COOKIE_JAR_CRYPTO_LIBS)
    target_link_libraries(${PLUGIN_WEBKITBROWSER_IMPLEMENTATION}
        PRIVATE
            ${PLUGIN_WEBKITBROWSER_COOKIE_JAR_CRYPTO_LIBS})
endif()
