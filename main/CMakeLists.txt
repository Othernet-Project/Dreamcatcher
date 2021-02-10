idf_build_get_property(idf_target IDF_TARGET)
set(target_name "${idf_target}")

set(SRCS 
    main.cpp 
    src/sdcard.cpp
    src/wifi.c
    src/web_server.c
    src/sx1280.cpp
    src/lnb.cpp
    src/settings.cpp
)
idf_component_register(SRCS ${SRCS} 
                        INCLUDE_DIRS "includes" "ondd" 
                        EMBED_FILES "embedded/files_tree.html" "embedded/index.html" "embedded/app.js" "embedded/app.css" "embedded/spinner.gif")

add_prebuilt_library(carousel "ondd/${target_name}/libcarousel.a" PRIV_REQUIRES arduino spiffs fatfs spi_flash sdmmc)
target_link_libraries(${COMPONENT_LIB} PRIVATE carousel)
