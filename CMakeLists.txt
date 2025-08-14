cmake_minimum_required(VERSION 3.28)

execute_process(
	COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libc++.modules.json
	OUTPUT_VARIABLE IMPORT_STD_PATH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (EXISTS "${IMPORT_STD_PATH}")
	if(CMAKE_VERSION VERSION_LESS "4.0.2")
		set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
	elseif(CMAKE_VERSION VERSION_LESS "4.0.3")
		set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
	else()
		set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
	endif()
	set(CMAKE_CXX_STANDARD_REQUIRED OFF)
	set(CMAKE_CXX_MODULE_STD ON)

	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()

	add_definitions(-DIMPORT_STD_IS_SUPPORTED=1)
	message("== import std; is supported. compiling goes vroom vroom")
else()
	message("[X] import std; is NOT supported. it should compile but slower")
endif()

project(lunas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
add_compile_options(-Wall -Wextra -pedantic -O1)

add_compile_options(
	-Walloca -Wcast-align -Wcast-qual -Wchar-subscripts -Wctor-dtor-privacy
	-Wdouble-promotion -Wenum-conversion -Wextra-semi -Wformat-signedness
	-Wformat=2 -Wmismatched-tags -Wmissing-braces -Wmultichar -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith
	-Wrange-loop-construct -Wuninitialized -Wvla -Wwrite-strings -Werror=unused-result  -Werror=unused-but-set-variable -Werror
	#-Wsign-conversion -Wdeprecated-copy-dtor -Wfloat-equal
)

set(output_dir "${PROJECT_SOURCE_DIR}/bin")
set(mods_dir "${PROJECT_SOURCE_DIR}/mod")
file(GLOB_RECURSE MODS ${mods_dir}/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${output_dir}")

include_directories(lib/luco/include/)
set(luco_mod_dir ${PROJECT_SOURCE_DIR}/lib/luco/mod)
set(MODS "${MODS};${luco_mod_dir}/luco.cpp")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-D__cpp_concepts=202002L -Wno-builtin-macro-redefined -Wno-macro-redefined -fcolor-diagnostics)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color -Wno-changes-meaning)
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
	BASE_DIRS ${mods_dir} ${luco_mod_dir}
	FILES ${MODS}
)

# add_custom_target(local
# 	COMMAND ${CMAKE_COMMAND} -D DISABLE_REMOTE=1 .
# 	COMMAND ${CMAKE_COMMAND} --build .
# 	COMMENT "Compiling without remote support"
# )

install(
	TARGETS ${PROJECT_NAME}
)

install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/${PROJECT_NAME}.1
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)

add_custom_target("uninstall"
	COMMENT ":: Uninstalling" ${PROJECT_NAME}
)

add_custom_command(TARGET "uninstall"
	COMMAND xargs rm -v < install_manifest.txt
	POST_BUILD COMMENT ":: Uninstalling"
)
