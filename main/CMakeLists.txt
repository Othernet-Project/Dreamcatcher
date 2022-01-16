idf_build_get_property(idf_target IDF_TARGET)
set(target_name "${idf_target}")

#use either sx1280 or lr1110 to compile!
set(SRCS 
    main.cpp 
    src/sdcard.cpp
    src/wifi.c
    src/web_server.c
    #src/sx1280.cpp
    src/lr1110.cpp
    lr1110_driver/lr1110_system.c
    lr1110_driver/lr1110_radio.c
    lr1110_driver/lr1110_regmem.c
    lr1110_driver/lr1110_hal.cpp
    src/lnb.cpp
    src/settings.cpp
    src/HopeDuino_CMT211xA.cpp
    src/HopeDuino_TWI.cpp
    src/en50607protocol.cpp
)
idf_component_register(SRCS ${SRCS} 
                        INCLUDE_DIRS "includes" "ondd" 
                        EMBED_FILES "embedded/index.html" "embedded/app.js" "embedded/app.css.gz" "embedded/tone.js.gz")

add_prebuilt_library(carousel "ondd/${target_name}/libcarousel.a" PRIV_REQUIRES arduino spiffs fatfs spi_flash sdmmc)
target_link_libraries(${COMPONENT_LIB} PRIVATE carousel)