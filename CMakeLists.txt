cmake_minimum_required (VERSION 3.25)
project ("easylog" CXX)
include(CTest)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
file(GLOB_RECURSE srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(lib_name "easylog")
add_library(${lib_name} SHARED ${srcs})
target_compile_features(${lib_name} PRIVATE cxx_std_20)
target_compile_definitions(${lib_name} PRIVATE EASYLOG)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${lib_name} PRIVATE -Wall -Wextra -fvisibility=hidden)
elseif(MSVC)
    target_compile_options(${lib_name} PRIVATE /W4)
endif()
target_include_directories(${lib_name} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
install(TARGETS ${lib_name}
  EXPORT "${lib_name}Targets"
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)