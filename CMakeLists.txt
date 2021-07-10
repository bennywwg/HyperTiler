cmake_minimum_required(VERSION 3.1..3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")
set(CMAKE_BUILD_TYPE Debug)

project(
	HyperTiler
	VERSION 1.0
	LANGUAGES CXX)

include(ExternalProject)
ExternalProject_Add(
	ht_front
	SOURCE_DIR "${PROJECT_SOURCE_DIR}/front"
	BUILD_IN_SOURCE 1
	BUILD_ALWAYS 1
	CONFIGURE_COMMAND npm install
	BUILD_COMMAND npm run build
	INSTALL_COMMAND cp -r "${PROJECT_SOURCE_DIR}/front/public" "${CMAKE_BINARY_DIR}/ht_front"
	BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/ht_front/"
)

file(GLOB HT_SRC src/*.cpp)

add_compile_definitions(FRONT_DIR="./ht_front")

add_executable(ht ${HT_SRC})

target_link_libraries(ht PRIVATE pthread curl png)
