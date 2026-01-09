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


# Detect CMSIS-DSP (arm_math.h) in workspace
# find_path(CMSIS_DSP_INCLUDE
#         NAMES arm_math.h
#         PATHS
#             ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/ARM_CMSIS/Include
#             ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
#             ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/DSP/Include
#             ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/DSP/Include/dsp
# )

# find_library(CMSIS_DSP_INCLUDE
#     NAMES CMSISDSP
#     PATHS ${CMAKE_SOURCE_DIR}/build # 或你放库的路径
# )



# 接口项目
add_library(wibotlib INTERFACE)

# 实现项目
add_library(wibotlib_impl )

if (CMSIS_DSP_ENABLED)
    message(STATUS "CMSIS-DSP detected")
    target_link_libraries(wibotlib INTERFACE CMSISDSP)
    # target_include_directories(wibotlib INTERFACE ${CMSIS_DSP_INCLUDE})
    target_compile_definitions(wibotlib INTERFACE CMSIS_DSP_ENABLED)
    if (ARM_MATH)
        message(STATUS "ARM_MATH macro detected")
        target_compile_definitions(wibotlib INTERFACE ${ARM_MATH})
    endif()
endif()


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
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/base)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/log/macro-log)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/port/chip)
target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/port/os)



target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/base)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/log/macro-log)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/log/rtt)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/port/arch)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/port/chip)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/port/os)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/os)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/hal)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/hal/stm32)

target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/controller)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/filter)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/mapper)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/util)

target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/pp)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/pp/controller)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/pp/filter)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/pp/mapper)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/pp/source)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/pp/util)

target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/comm)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/comm/msg)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/comm/crc)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/comm/modbus)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/comm/protocol)

target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/display)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/io)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/misc)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rf)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rls)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/rtc)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/storage)
target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/device/motor)

target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/math)

# target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/graph)
# target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/graph/fonts)

target_sources_batch(wibotlib_impl ${CMAKE_CURRENT_SOURCE_DIR}/src/app)








target_include_directories(wibotlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/app)



# ------------------------------

target_link_libraries(${CMAKE_PROJECT_NAME}
    wibotlib_impl
    wibotlib
    # Add user defined libraries
)