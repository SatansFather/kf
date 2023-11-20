cmake_minimum_required (VERSION 3.21)

project (KarnageFreak)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")

include_directories (
"libs/enet/include"
"libs/zlib"
"libs/soloud/include"
"libs/obj"
"src"
"libs/stb"
"libs/glfw/include"
"libs/glm"
"libs/md5"
"libs/http/include"
)

add_definitions(-DUNICODE -D_UNICODE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
file(GLOB_RECURSE kf_src 
"src/engine/*.h"
"src/engine/*.cpp"
"src/game/*.h" 
"src/game/*.cpp" 
"src/compiler/brush/brush.h"
"src/compiler/brush/brush.cpp"
"src/compiler/brush/brush_face.h"
"src/compiler/brush/brush_face.cpp"
"src/compiler/entity.h"
"src/compiler/entity.cpp"
"src/compiler/map_file.h"
"src/compiler/map_file.cpp"
"libs/stb/*.h" 
"libs/glfw/include/*.h" 
"libs/glm/*.h"
"libs/glm/*.c"
"libs/glm/*.cpp"
"libs/glm/*.hpp"
"libs/md5/*.h"
"libs/md5/*.c"
"libs/zlib/*.h" 
"libs/zlib/*.cpp" 
"libs/zlib/*.c"
"libs/enet/*.h" 
"libs/enet/*.c"
"libs/soloud/include/*.h" 
"libs/obj/tiny_obj_loader.h" 
"libs/obj/tiny_obj_loader.cc"
"libs/http/include/*.h"
)

file(GLOB_RECURSE kf_src_compiler
"src/engine/collision/*.h"
"src/engine/collision/*.cpp" 
"src/engine/global/*.h"
"src/engine/global/*.cpp" 
"src/engine/math/*.h"
"src/engine/math/*.cpp" 
"src/engine/os/*.h"
"src/engine/os/*.cpp" 
"src/engine/render/*.h"
"src/engine/render/*.cpp" 
"src/engine/run/*.h"
"src/engine/run/*.cpp" 
"src/engine/system/*.h"
"src/engine/system/*.cpp" 
"src/engine/utility/*.h"
"src/engine/utility/*.cpp" 
"src/compiler/*.h"
"src/compiler/*.cpp" 
"libs/stb/*.h" 
"libs/glfw/include/*.h" 
"libs/glad/include/*.h"
"libs/glad/src/*.c"
"libs/glm/*.h"
"libs/glm/*.c"
"libs/glm/*.cpp"
"libs/glm/*.hpp"
"libs/lua542/*.h"
"libs/sol/*.hpp"
"libs/md5/*.h"
"libs/md5/*.c"
"libs/zlib/*.h" 
"libs/zlib/*.cpp" 
"libs/zlib/*.c"
"libs/enet/*.h" 
"libs/enet/*.c"
"libs/obj/tiny_obj_loader.h" 
"libs/obj/tiny_obj_loader.cc" 
"libs/freetype/include/*.h"
"libs/flecs/flecs.h"
"libs/flecs/flecs.c"
)

file(GLOB_RECURSE kf_src_server
"src/engine/*.h"
"src/engine/*.cpp" 
"src/game/*.h"
"src/game/*.cpp" 
"src/compiler/brush/*.h"
"src/compiler/brush/*.cpp"
"src/compiler/entity.h"
"src/compiler/entity.cpp"
"src/compiler/map_file.h"
"src/compiler/map_file.cpp"
"libs/stb/*.h" 
"libs/glm/*.h"
"libs/glm/*.c"
"libs/glm/*.cpp"
"libs/glm/*.hpp"
"libs/lua542/*.h"
"libs/sol/*.hpp"
"libs/md5/*.h"
"libs/md5/*.c"
"libs/zlib/*.h" 
"libs/zlib/*.cpp" 
"libs/zlib/*.c"
"libs/enet/*.h" 
"libs/enet/*.c"
"libs/http/include/*.h"
"libs/flecs/flecs.h"
"libs/flecs/flecs.c"
)

add_executable(karnagefreak WIN32 ${kf_src} ${APP_ICON_RESOURCE_WINDOWS})
target_link_libraries(karnagefreak debug ${CMAKE_SOURCE_DIR}/libs/soloud/Debug/soloud_static.lib optimized ${CMAKE_SOURCE_DIR}/libs/soloud/Release/soloud_static.lib)
target_link_libraries(karnagefreak ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2019/64/glfw3.lib)

add_executable(kfdev WIN32 ${kf_src})
target_compile_definitions(kfdev PRIVATE $<$<CONFIG:Release>:_DEV>)
target_compile_definitions(kfdev PRIVATE $<$<CONFIG:Debug>:_DEV>)
target_compile_definitions(kfdev PRIVATE $<$<CONFIG:RelWithDebInfo>:_DEV>)

target_link_libraries(kfdev debug ${CMAKE_SOURCE_DIR}/libs/soloud/Debug/soloud_static.lib optimized ${CMAKE_SOURCE_DIR}/libs/soloud/Release/soloud_static.lib)
target_link_libraries(kfdev ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2019/64/glfw3.lib)

add_executable(kfpack WIN32 ${kf_src})
target_compile_definitions(kfpack PRIVATE $<$<CONFIG:Release>:_DEV>)
target_compile_definitions(kfpack PRIVATE $<$<CONFIG:Debug>:_DEV>)
target_compile_definitions(kfpack PRIVATE $<$<CONFIG:RelWithDebInfo>:_DEV>)
target_compile_definitions(kfpack PRIVATE $<$<CONFIG:Release>:_PACK>)
target_compile_definitions(kfpack PRIVATE $<$<CONFIG:Debug>:_PACK>)
target_compile_definitions(kfpack PRIVATE $<$<CONFIG:RelWithDebInfo>:_PACK>)
target_link_libraries(kfpack debug ${CMAKE_SOURCE_DIR}/libs/soloud/Debug/soloud_static.lib optimized ${CMAKE_SOURCE_DIR}/libs/soloud/Release/soloud_static.lib)
target_link_libraries(kfpack ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2019/64/glfw3.lib)

if (MSVC) 
	target_link_options(kfdev PRIVATE $<$<CONFIG:RelWithDebInfo>:/PROFILE>)
endif()

add_executable(kfserver WIN32 ${kf_src_server})
target_compile_definitions(kfserver PRIVATE $<$<CONFIG:Release>:_SERVER>)
target_compile_definitions(kfserver PRIVATE $<$<CONFIG:RelWithDebInfo>:_SERVER>)
target_compile_definitions(kfserver PRIVATE $<$<CONFIG:Debug>:_SERVER>)
target_compile_definitions(kfserver PRIVATE $<$<CONFIG:Release>:_NOSOUND>)
target_compile_definitions(kfserver PRIVATE $<$<CONFIG:RelWithDebInfo>:_NOSOUND>)
target_compile_definitions(kfserver PRIVATE $<$<CONFIG:Debug>:_NOSOUND>)

add_executable(kfserverdev WIN32 ${kf_src_server})
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:Release>:_SERVER>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:RelWithDebInfo>:_SERVER>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:Debug>:_SERVER>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:Release>:_NOSOUND>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:RelWithDebInfo>:_NOSOUND>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:Debug>:_NOSOUND>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:Release>:_DEV>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:RelWithDebInfo>:_DEV>)
target_compile_definitions(kfserverdev PRIVATE $<$<CONFIG:Debug>:_DEV>)

add_executable(kfcompile WIN32 ${kf_src_compiler})
target_compile_definitions(kfcompile PRIVATE $<$<CONFIG:Release>:_COMPILER>)
target_compile_definitions(kfcompile PRIVATE $<$<CONFIG:RelWithDebInfo>:_COMPILER>)
target_compile_definitions(kfcompile PRIVATE $<$<CONFIG:Debug>:_COMPILER>)
target_compile_definitions(kfcompile PRIVATE $<$<CONFIG:Release>:_NOSOUND>)
target_compile_definitions(kfcompile PRIVATE $<$<CONFIG:RelWithDebInfo>:_NOSOUND>)
target_compile_definitions(kfcompile PRIVATE $<$<CONFIG:Debug>:_NOSOUND>)
target_link_libraries(kfcompile ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2019/64/glfw3.lib)

add_executable(kfcompiledev WIN32 ${kf_src_compiler})
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:Release>:_COMPILER>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:RelWithDebInfo>:_COMPILER>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:Debug>:_COMPILER>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:Release>:_NOSOUND>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:RelWithDebInfo>:_NOSOUND>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:Debug>:_NOSOUND>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:Release>:_DEV>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:RelWithDebInfo>:_DEV>)
target_compile_definitions(kfcompiledev PRIVATE $<$<CONFIG:Debug>:_DEV>)
target_link_libraries(kfcompiledev ${CMAKE_SOURCE_DIR}/libs/glfw/lib-vc2019/64/glfw3.lib)

find_package (Threads REQUIRED)

add_executable(kfserverdev-linux ${kf_src_server})
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:Release>:_SERVER>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:RelWithDebInfo>:_SERVER>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:Debug>:_SERVER>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:Release>:_NOSOUND>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:RelWithDebInfo>:_NOSOUND>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:Debug>:_NOSOUND>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:Release>:_DEV>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:RelWithDebInfo>:_DEV>)
target_compile_definitions(kfserverdev-linux PRIVATE $<$<CONFIG:Debug>:_DEV>)
target_link_libraries(kfserverdev-linux PUBLIC Threads::Threads)

add_executable(kfserver-linux ${kf_src_server})
target_compile_definitions(kfserver-linux PRIVATE $<$<CONFIG:Release>:_SERVER>)
target_compile_definitions(kfserver-linux PRIVATE $<$<CONFIG:RelWithDebInfo>:_SERVER>)
target_compile_definitions(kfserver-linux PRIVATE $<$<CONFIG:Debug>:_SERVER>)
target_compile_definitions(kfserver-linux PRIVATE $<$<CONFIG:Release>:_NOSOUND>)
target_compile_definitions(kfserver-linux PRIVATE $<$<CONFIG:RelWithDebInfo>:_NOSOUND>)
target_compile_definitions(kfserver-linux PRIVATE $<$<CONFIG:Debug>:_NOSOUND>)
target_link_libraries(kfserver-linux PUBLIC Threads::Threads)

#install (TARGETS karnagefreak DESTINATION bin)
#install (TARGETS kfserver DESTINATION bin)