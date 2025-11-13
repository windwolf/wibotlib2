if (NOT DEFINED LOG_LEVEL)
    message(STATUS "LOG_LEVEL: not set, use default value : INFO")
    set(LOG_LEVEL "INFO")
else ()
    message(STATUS "LOG_LEVEL: ${LOG_LEVEL}")
endif ()


if (NOT DEFINED LOG_COLOR)
    message(STATUS "LOG_COLOR: not set, use default value : ON")
    set(LOG_COLOR ON)
else ()
    message(STATUS "LOG_COLOR: ${LOG_COLOR}")
endif ()

if (LOG_COLOR)
    target_compile_definitions(${PROJECT_NAME}
            PUBLIC -DLOG_COLOR)
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC -DLOG_LEVEL_${LOG_LEVEL})
process_src_dir(${CMAKE_CURRENT_LIST_DIR}/macro-log ${PROJECT_NAME})

# process_src_dir(${CMAKE_CURRENT_LIST_DIR}/object-log ${PROJECT_NAME})
