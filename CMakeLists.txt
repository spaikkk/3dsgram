cmake_minimum_required(VERSION 3.13)

project(3DSTemplate LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
configure_file(${CMAKE_CURRENT_LIST_DIR}/.clangd.in
                    ${CMAKE_CURRENT_LIST_DIR}/.clangd @ONLY)

set(APP_NAME "Nocciogram" CACHE STRING "Nocciogram")
set(APP_DESCRIPTION "Telegram nocciolato" CACHE STRING "Telegram nocciolato")
set(APP_AUTHOR "Spaik@NoccioloProductions" CACHE STRING "Spaik@NoccioloProductions")

set(DEVKITPRO $ENV{DEVKITPRO})
list(APPEND CMAKE_MODULE_PATH ${DEVKITPRO}/cmake)
include(3DS)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JANSSON REQUIRED IMPORTED_TARGET jansson)

add_subdirectory(third_party/inih)

set(NOCCIOLIB_ROOT src/nocciolib)
set(SRC_LIST
    ${NOCCIOLIB_ROOT}/bot.c
    ${NOCCIOLIB_ROOT}/longpoll.c
    ${NOCCIOLIB_ROOT}/types.c 
    ${NOCCIOLIB_ROOT}/parser.c
    ${NOCCIOLIB_ROOT}/methods.c
    ${NOCCIOLIB_ROOT}/utility/logger.c
    ${NOCCIOLIB_ROOT}/net/httpClient.c)


add_library(tg-nocciolib STATIC ${SRC_LIST})
target_link_libraries(tg-nocciolib PUBLIC PkgConfig::JANSSON inih)
target_include_directories(tg-nocciolib 
                        PUBLIC include/nocciolib
                        PRIVATE src/nocciolib
                        )

add_executable(${PROJECT_NAME} src/example/example.c)
target_link_libraries(${PROJECT_NAME} PRIVATE tg-nocciolib)
target_include_directories(${PROJECT_NAME} PRIVATE include include/nocciolib)

set(SMDH_FILE ${PROJECT_NAME}.smdh)

ctr_generate_smdh(${SMDH_FILE}
    NAME "${APP_NAME}"
    DESCRIPTION "${APP_DESCRIPTION}"
    AUTHOR "${APP_AUTHOR}"
)

ctr_create_3dsx(${PROJECT_NAME}
    SMDH ${SMDH_FILE}
)