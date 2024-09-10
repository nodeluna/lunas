cmake_minimum_required(VERSION 3.5)

project(lunas)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES src/*.cpp)
add_compile_options(-Wall -Wextra -pedantic -O1)
set(CMAKE_CXX_STANDARD 23)
add_executable(${PROJECT_NAME} ${SOURCES}) 

if(DISABLE_REMOTE)
	message("Compiling without remote support")
	target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_REMOTE)
else()
	target_link_libraries(${PROJECT_NAME} ssh)
endif()

add_custom_target(local
	COMMAND ${CMAKE_COMMAND} -D DISABLE_REMOTE=1 .
	COMMAND ${CMAKE_COMMAND} --build .
	COMMENT "Compiling without remote support"
)

install(
	TARGETS lunas DESTINATION bin
)

install(
	FILES man/${PROJECT_NAME}.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)

add_custom_target("uninstall"
	COMMENT ":: Uninstalling" ${PROJECT_NAME}
)

add_custom_command(TARGET "uninstall"
	COMMAND xargs rm -v < install_manifest.txt
	POST_BUILD COMMENT ":: Uninstalling"
)
