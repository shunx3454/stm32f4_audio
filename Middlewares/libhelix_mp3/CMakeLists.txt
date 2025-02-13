cmake_minimum_required(VERSION 3.22)

project(helixmp3)

add_library(${PROJECT_NAME} INTERFACE)

if (MSVC)
        add_compile_options(/W4)
else()
        add_compile_options(-Wall -Werror)
endif()

file(GLOB HelixMP3_real_src
        real/*.c
        ./*.c
)

target_include_directories(${PROJECT_NAME} INTERFACE
        pub
)

target_sources(${PROJECT_NAME} INTERFACE
        ${HelixMP3_real_src}
)

target_link_directories(${PROJECT_NAME} INTERFACE
)

target_link_libraries(${PROJECT_NAME} INTERFACE
)