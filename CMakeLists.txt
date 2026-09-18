cmake_minimum_required(VERSION 3.20)

project(HandmadeHero C)

option(HANDMADE_SLOW "Enable slow-build assertions" ON)
option(HANDMADE_INTERNAL "Enable internal development build" ON)

add_executable(HandmadeHero WIN32
    Win32_Handmade.c
    Handmade.c
)

if(HANDMADE_SLOW)
    target_compile_definitions(HandmadeHero PRIVATE HANDMADE_SLOW=1)
endif()

if(HANDMADE_INTERNAL)
    target_compile_definitions(HandmadeHero PRIVATE HANDMADE_INTERNAL=1)
endif()

target_link_libraries(HandmadeHero PRIVATE
    user32
    gdi32
)