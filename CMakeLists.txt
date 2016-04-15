cmake_minimum_required(VERSION 3.5)
project(MineSweeper)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_REENTRANT")
if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
endif()
set(LIBRARIES "SDL2" "SDL2_image" "SDL2_ttf" "SDL2_mixer")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ddebug")
endif ()

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

    list(INSERT LIBRARIES 0 "mingw32" "SDL2main")
endif ()

if (MSVC)
    # Assume library files are downloaded in ./lib/ folder
    set(SDL2_ARCH "x86")
    set(SDL2_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2-2.0.4")
    set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_image-2.0.1")
    set(SDL2_TTF_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_ttf-2.0.14")
    set(SDL2_MIXER_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_mixer-2.0.1")

    include_directories(${SDL2_DIR}/include)
    include_directories(${SDL2_IMAGE_DIR}/include)
    include_directories(${SDL2_TTF_DIR}/include)
    include_directories(${SDL2_MIXER_DIR}/include)

    link_directories("${SDL2_DIR}/lib/${SDL2_ARCH}")
    link_directories("${SDL2_IMAGE_DIR}/lib/${SDL2_ARCH}")
    link_directories("${SDL2_TTF_DIR}/lib/${SDL2_ARCH}")
    link_directories("${SDL2_MIXER_DIR}/lib/${SDL2_ARCH}")

    list(INSERT LIBRARIES 0 "SDL2main")
endif ()

set(SOURCE_FILES src/main.c src/model.c)
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

file(GLOB RES_FILES "${CMAKE_SOURCE_DIR}/res/*.*")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/res/)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RES_FILES} $<TARGET_FILE_DIR:${PROJECT_NAME}>/res/)

if (MINGW)
    file(GLOB DLL_Files
            "${SDL2_DIR}/bin/*.dll"
            "${SDL2_IMAGE_DIR}/bin/*.dll"
            "${SDL2_TTF_DIR}/bin/*.dll"
            "${SDL2_MIXER_DIR}/bin/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_Files} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif ()

if (MSVC)
    file(GLOB DLL_Files
            "${SDL2_DIR}/lib/${SDL2_ARCH}/*.dll"
            "${SDL2_IMAGE_DIR}/lib/${SDL2_ARCH}/*.dll"
            "${SDL2_TTF_DIR}/lib/${SDL2_ARCH}/*.dll"
            "${SDL2_MIXER_DIR}/lib/${SDL2_ARCH}/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_Files} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif ()