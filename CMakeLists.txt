cmake_minimum required(VERSION 3.10)

project(ctp)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory(. SRC_LIST)

include_directories (${PROJECT_SOURCE_DIR}/include) #指定头文件路径

message(" ${PROJECT_ SOURCE_ DIR}")

find_library (libvar libthosttraderapi_sm.so ${PR0JECT_SOURCE_DIR}/dll)
file(GLOB_RECURSE GT_LIB ${PROJECT_SOURCE_DIR}/d11/* .so)
if(${libvar} STREQUAL "libvar-NOTFOUND" )
	message(FATAL_ERROR "library not found!”)
else()
	message(STATUS "thosttraderapi_sm.so found in ${libvar}" )
endif()

add executable(main ${SRC_LIST})

target_link libraries(main ${GT_LIB}) #链接动态库
