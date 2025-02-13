cmake_minimum_required(VERSION 3.28)

project(lunas)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wextra -pedantic -O1)

set(output_dir "${PROJECT_SOURCE_DIR}/bin")
set(mods_dir "${PROJECT_SOURCE_DIR}/mod")
file(GLOB_RECURSE MODS ${mods_dir}/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${output_dir}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-D__cpp_concepts=202002L -Wno-builtin-macro-redefined -Wno-macro-redefined -fcolor-diagnostics)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color)
endif()

add_executable(${PROJECT_NAME} src/main.cpp) 

if(DISABLE_REMOTE)
	message("Compiling without remote support")
	target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_REMOTE)
else()
	find_package(libssh 0.11.0 REQUIRED)
	target_link_libraries(${PROJECT_NAME} ssh)
	target_compile_definitions(${PROJECT_NAME} PRIVATE REMOTE_ENABLED)
endif()

target_link_libraries(${PROJECT_NAME} pthread)

target_sources(${PROJECT_NAME}
	PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${mods_dir}
	FILES
	${MODS}
)

add_custom_target(local
	COMMAND ${CMAKE_COMMAND} -D DISABLE_REMOTE=1 .
	COMMAND ${CMAKE_COMMAND} --build .
	COMMENT "Compiling without remote support"
)

install(
	TARGETS lunas DESTINATION bin
)

install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/${PROJECT_NAME}.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)

add_custom_target("uninstall"
	COMMENT ":: Uninstalling" ${PROJECT_NAME}
)

add_custom_command(TARGET "uninstall"
	COMMAND xargs rm -v < install_manifest.txt
	POST_BUILD COMMENT ":: Uninstalling"
)
