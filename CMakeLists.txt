cmake_minimum_required(VERSION 3.21)
project(lua-nuspell CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Binding
add_library(nuspell MODULE src/nuspell.cxx)

# find_package(Lua REQUIRED)
# Lua, from LuaRocks config
set(LUA_INCLUDE_DIR ${LUA_INCDIR})
set(LUA_LIBRARY "${LUA_LIBDIR}${LUA_LIBDIR_FILE}")

# Nuspell  
find_package (Nuspell CONFIG REQUIRED)

set_target_properties(nuspell PROPERTIES PREFIX "")
set_target_properties(nuspell PROPERTIES COMPILE_FLAGS "${CFLAGS}")

include_directories(nuspell ${LUA_INCLUDE_DIR})
include_directories(nuspell ${NUSPELL_INCLUDE_DIR})

target_link_libraries(nuspell ${LUA_LIBRARY})
target_link_libraries(nuspell Nuspell::nuspell)

install(TARGETS nuspell DESTINATION "${INSTALL_CMOD}")
