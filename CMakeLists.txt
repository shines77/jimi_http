CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

###############################################################

# project(picohttpparser)
project(jimi_http_parser)

# Single thread mode /ML and /MLd have be deprecated from VS 2003.
option(BoostOption_USE_MULTITHREADED "Enable Boost's library use multithread" ON)

if (WIN32)
    option(BoostOption_USE_STATIC_LIBS "Enable Boost use static library" ON)
    option(BoostOption_USE_STATIC_RUNTIME "Enable Boost use static runtime library" OFF)
else()
    option(BoostOption_USE_STATIC_LIBS "Enable Boost use static library" OFF)
    option(BoostOption_USE_STATIC_RUNTIME "Enable Boost use static runtime library" OFF)
endif()

set(Boost_USE_STATIC_LIBS        BoostOption_USE_STATIC_LIBS)      # only find static libs
set(Boost_USE_MULTITHREADED      BoostOption_USE_MULTITHREADED)
set(Boost_USE_STATIC_RUNTIME     BoostOption_USE_STATIC_RUNTIME)

find_package(Boost 1.54 COMPONENTS system thread atomic chrono program_options REQUIRED)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(EXTRA_LIBS ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("------------ Options -------------")
message("  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message("----------------------------------")

message("-------------- Env ---------------")
message("  CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("  CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("  PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("----------------------------------")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

if (NOT MSVC)
    ## -Wall
    ## Reserve: [Support Intel SMID SHA1/SHA256: -msha]
    set(CMAKE_C_FLAGS_COMMON "-mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4a -msse4.1 -msse4.2 -mpclmul -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-macros -Wno-unused-private-field -Wno-unreachable-code -Wno-gnu-anonymous-struct -fPIC -U__STRICT_ANSI__")

    set(CMAKE_C_FLAGS_DEFAULT "${CMAKE_C_FLAGS} -std=c99 ${CMAKE_C_FLAGS_COMMON}")
    ##
    ## G++ optimization beyond -O3/-Ofast
    ## See: https://stackoverflow.com/questions/14492436/g-optimization-beyond-o3-ofast
    ##
    ## Reserve: -O3 -Ofast  -flto
    ##
    ## BUG: GCC's "-fwhole-program" option may cause functions that are not used by the static/dynamic link library to be optimized,
    ##      resulting in the exported function or symbol table not being exported.
    ##
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEFAULT} -Ofast -march=native -mtune=native -funroll-loops -frename-registers")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEFAULT} -g -pg")

    ## --std=gnu++11 -Wall -Werror -Wextra -Wno-format -Wno-unused-function
    set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS} --std=gnu++11 ${CMAKE_C_FLAGS_COMMON}")
    ##
    ## G++ optimization beyond -O3/-Ofast
    ## See: https://stackoverflow.com/questions/14492436/g-optimization-beyond-o3-ofast
    ##
    ## Reserve: -O3 -Ofast -flto
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEFAULT} -Ofast -march=native -mtune=native -funroll-loops -frename-registers -fwhole-program")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEFAULT} -g -pg")

    add_compile_options(-D__SSE3__=1 -D__SSE4A__=1 -D__SSE4_1__=1 -D__SSE4_2__=1)
endif()

if (WIN32)
    add_compile_options(-D_WIN32_WINNT=0x0601 -D_CRT_SECURE_NO_WARNINGS -D__SSE4_2__=1)
    set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 mswsock)

    if (Boost_USE_STATIC_RUNTIME)
        #
        # Replace all "/MD" flags to "/MT"
        # See: http://stackoverflow.com/questions/14172856/cmake-compile-with-mt-instead-of-md
        #
        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
        foreach(CompilerFlag ${CompilerFlags})
            if (${CompilerFlag} MATCHES "/MD")
                string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            endif()
        endforeach()
    endif()
endif()

# if (UNIX)
#     set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
#     set(EXTRA_LIBS ${EXTRA_LIBS} atomic)
# else()
#     set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
# endif()
# 
# include_directories(deps)
# include_directories(deps/picohttpparser)
# 
# set(SOURCE_FILES
#     deps/picohttpparser/picohttpparser.h
#     deps/picohttpparser/picohttpparser.c
#     )
# 
# add_library(picohttpparser STATIC ${SOURCE_FILES})
# target_link_libraries(picohttpparser ${EXTRA_LIBS})

###############################################################

# project(jimi_http_parser)

# find_package(Threads)
# find_path(picohttpparser NAMES picohttpparser.h PATHS "${CMAKE_SOURCE_DIR}/deps/")

if (UNIX)
    set(LIBPICOHTTPPARSER "${CMAKE_SOURCE_DIR}/libpicohttpparser.a")
else()
    set(LIBPICOHTTPPARSER "")
endif()

include_directories(src)
include_directories(src/main)
include_directories(src/test)
include_directories(src/unittest)
include_directories(deps)

link_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_SOURCE_DIR}/deps/)

# add_subdirectory(deps)

set(LIB_SOURCE_FILES
    deps/picohttpparser/picohttpparser.c
    )

add_library(picohttpparser STATIC ${LIB_SOURCE_FILES})

set(SOURCE_FILES
    src/test/http_parser_test/main.cpp
    )

# add_custom_target(debug
#     COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
#     COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
#     COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
# )
# 
# add_custom_target(release
#     COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
#     COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
#     COMMENT "Switch CMAKE_BUILD_TYPE to Release"
# )

add_executable(jimi_http_parser ${SOURCE_FILES})
if (UNIX)
    add_dependencies(jimi_http_parser picohttpparser)
endif()
# target_link_libraries(jimi_http_parser ${EXTRA_LIBS})
target_link_libraries(jimi_http_parser ${EXTRA_LIBS} picohttpparser)

###############################################################

project(jimi_http_serv)

include_directories(src)
include_directories(src/main)
include_directories(src/test)
include_directories(src/unittest)
include_directories(deps)

set(SOURCE_FILES
    src/main/jimi_http_serv/main.cpp
    )

add_executable(jimi_http_serv ${SOURCE_FILES})
target_link_libraries(jimi_http_serv ${EXTRA_LIBS})

###############################################################
