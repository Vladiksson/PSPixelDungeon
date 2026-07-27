cmake_minimum_required(VERSION 3.11)
project(pspixeldungeon C CXX)

add_executable(${PROJECT_NAME}
    src/main.c
    src/level.c
    src/level_utils.c
    src/level_paint.c
    src/level_decorate.c
    src/level_boss.c
    src/level_last.c
    src/tiles.c
    src/render.c
    src/input.c
    src/camera.c
    src/player.c
    src/fov.c
    src/dungeon.c
    src/item.c
    src/item_db.c
    src/item_spawn.c
    src/item_identify.c
    src/inventory_screen.c
    src/font.c
    src/chrome.c
    src/bag.c
    src/icons.c
)

target_compile_options(${PROJECT_NAME} PRIVATE -O2 -G0 -Wall)
target_include_directories(${PROJECT_NAME} PRIVATE include)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    SDL2_image
    png
    jpeg
    z
    pspgu pspgum pspdisplay pspge
    pspaudio pspctrl pspnet psputility
    stdc++ c m
)

# ── Copy assets/ next to the EBOOT so paths resolve at runtime ──
# PPSSPP and real PSP both resolve "assets/tiles0.png" relative to
# the directory containing EBOOT.PBP — not the build/ subdirectory.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets/ next to the EBOOT"
)

# ── PSP EBOOT packaging ──────────────────────────────────────────
if(PSP)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PSP)
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "EBOOT")
    create_pbp_file(
        TARGET          ${PROJECT_NAME}
        TITLE           "PSPixel Dungeon"
        VERSION         01.00
        ICON_PATH       ${CMAKE_CURRENT_SOURCE_DIR}/icon/ICON0.PNG
        BACKGROUND_PATH NULL
        PREVIEW_PATH    NULL
    )
endif()