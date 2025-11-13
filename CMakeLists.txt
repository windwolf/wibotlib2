cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(cmake/utilities.cmake)



# 接口项目
add_library(wibotlib INTERFACE)

# 实现项目
add_library(wibotlib_impl )


set(WIBOTLIB_DEF 
    ${PROJECT_DEFS}
)
target_compile_definitions(wibotlib INTERFACE ${WIBOTLIB_DEF})

set(WIBOTLIB_LNK
    stm32cubemx
    ${TOOLCHAIN_LINK_LIBRARIES}
)
target_link_libraries(wibotlib INTERFACE ${WIBOTLIB_LNK})


target_link_libraries(wibotlib_impl PUBLIC wibotlib)


# wibotlib INCLUDES
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/base)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/base)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/log/macro-log)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/log/rtt)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/log/macro-log)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/log/rtt)


target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/port/arch)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/port/os)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/os)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/port/os)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/os)



target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/port/chip)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/hal)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/hal/stm32)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/hal)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/hal/stm32)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model/source)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model/mapper)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model/filter)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model/adapter)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model/rw)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/model/controller)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model/filter)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model/source)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model/mapper)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model/adapter)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model/rw)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/model/controller)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/camera)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/dshot)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/gnss)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/crc)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/camera)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/dshot)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/gnss)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/crc)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/display)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/io)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/misc)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rf)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rls)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rtc)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/storage)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/device/motor)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/display)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/io)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/misc)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rf)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rls)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rtc)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/storage)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/motor)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/graph)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/graph)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/fonts)


target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/app)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/app)


