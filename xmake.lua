add_rules("mode.release", "mode.debug")
set_languages("c++23")


-- set_toolchains("clang")
add_cxxflags("-Wall -Wextra -pedantic -O1")

add_cxxflags("-Wno-c++26-extensions -Wno-error=c++26-extensions -Wno-changes-meaning -Walloca -Wcast-align -Wchar-subscripts -Wctor-dtor-privacy -Wdouble-promotion -Wenum-conversion -Wextra-semi -Wformat-signedness -Wformat=2 -Wmismatched-tags -Wmissing-braces -Wmultichar -Wno-unused-command-line-argument -Wno-reserved-module-identifier -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wrange-loop-construct -Wuninitialized -Wvla -Wwrite-strings -Werror=unused-result  -Werror=unused-but-set-variable -Werror")


target("lunas")
	set_kind("binary")
	set_runtimes("c++_static")
	add_files("src/main.cpp")
	add_files("mod/**.cpp")
	add_includedirs("mod", {public = true, recursive = true})
	add_includedirs("lib/luco/include/", {public = false, recursive = true})
	add_links("ssh")
	add_defines("REMOTE_ENABLED")
	add_installfiles("man/lunas.1", { prefixdir = "share/man/man1"})
