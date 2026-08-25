cmake_minimum_required(VERSION 3.20)

project(HandmadeHero C)

add_executable(HandmadeHero WIN32
    main.c
)

target_link_libraries(HandmadeHero PRIVATE
    user32
    gdi32
)