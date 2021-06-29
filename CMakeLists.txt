cmake_minimum_required(VERSION 3.1..3.20)

set(CMAKE_CXX_STANDARD 20)

project(
	HyperTiler
	VERSION 1.0
	LANGUAGES CXX)

file(GLOB HT_SRC src/*.cpp)

add_compile_definitions(INSTALL_DIR="/home/quinn/hypertiler/")

add_executable(ht ${HT_SRC})

target_link_libraries(ht PRIVATE pthread curl png)
