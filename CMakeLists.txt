cmake_minimum_required(VERSION 3.5)
project(MineSweeper)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -D_REENTRANT")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ddebug")
endif ()
set(LIBRARIES "-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer")

if (MINGW)
    # Assume library files are downloaded in ./lib/ folder
    set(SDL2_ARCH "i686-w64-mingw32")
    set(SDL2_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2-2.0.4/${SDL2_ARCH}")
    set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_image-2.0.1/${SDL2_ARCH}")
    set(SDL2_TTF_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_ttf-2.0.14/${SDL2_ARCH}")
    set(SDL2_MIXER_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_mixer-2.0.1/${SDL2_ARCH}")

    include_directories(${SDL2_DIR}/include/SDL2)
    include_directories(${SDL2_IMAGE_DIR}/include/SDL2)
    include_directories(${SDL2_TTF_DIR}/include/SDL2)
    include_directories(${SDL2_MIXER_DIR}/include/SDL2)

    link_directories("${SDL2_DIR}/lib")
    link_directories("${SDL2_IMAGE_DIR}/lib")
    link_directories("${SDL2_TTF_DIR}/lib")
    link_directories("${SDL2_MIXER_DIR}/lib")

    set(LIBRARIES "-lmingw32 -lSDL2main ${LIBRARIES}")
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LIBRARIES "${LIBRARIES} -mwindows")
    endif ()
endif ()

set(SOURCE_FILES src/main.c src/model.c)
add_executable(MineSweeper ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res/"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res/)

if (MINGW)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SDL2_DIR}/bin/"
            "${SDL2_IMAGE_DIR}/bin/"
            "${SDL2_TTF_DIR}/bin/"
            "${SDL2_MIXER_DIR}/bin/"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif ()