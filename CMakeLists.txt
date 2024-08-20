cmake_minimum_required(VERSION 3.5)

project(lunas)

link_libraries(ssh)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES src/*.cpp)
add_compile_options(-Wall -Wextra -pedantic -O1)
set(CMAKE_CXX_STANDARD 20)
add_executable(${PROJECT_NAME} ${SOURCES}) 

install(TARGETS lunas DESTINATION bin)
install(FILES man/${PROJECT_NAME}.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)

add_custom_target("uninstall" COMMENT ":: Uninstalling" ${PROJECT_NAME})
add_custom_command(TARGET "uninstall" POST_BUILD COMMENT ":: Uninstalling" COMMAND xargs rm -v < install_manifest.txt)
