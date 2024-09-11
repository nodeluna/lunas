CC = g++
CFLAGS = -O1 -Iinclude -Wall -Wextra -pedantic -std=c++23 -MMD -MP -g
ASANFLAGS = $(CFLAGS) -g -fsanitize=address -fno-omit-frame-pointer
TARGET = lunas
LIB = $(shell pkg-config --cflags --libs libssh)
INSTALL_DIR = /usr/bin
BUILD_DIR = build
HEADER_DIR = include
MANPAGE_DIR = /usr/share/man/man1
SRC_DIR = src
SRCS := $(wildcard src/*.cpp)
OBJS := $(addprefix build/, $(notdir $(SRCS:.cpp=.o)))
DEP := $(OBJS:.o=.d)
pwd = $(shell pwd)

ifneq ($(findstring clang++, $(CC)),)
	CFLAGS += $(CLAGS) -D__cpp_concepts=202002L -Wno-builtin-macro-redefined -Wno-macro-redefined
endif

building: create_directory $(OBJS) $(TARGET)

-include $(DEP)

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIB) $^ -o $@

create_directory:
ifeq ($(shell test -e $(BUILD_DIR) && echo -n true),true)
else
	mkdir $(BUILD_DIR)
endif

install: 
ifeq ($(shell test -e $(INSTALL_DIR) && echo -n true),true)
else
	$(eval INSTALL_DIR = $(shell echo $$PATH | cut -d ' ' -f1))
endif

ifeq ($(shell test -e $(MANPAGE_DIR) && echo -n true),true)
else
	$(eval MANPAGE_DIR = $(shell whereis man | awk '{for(i=1; i<=NF; i++) print $$i}' | awk '/man.1.gz/' | awk '{gsub("/man.1.gz", ""); print}'))
endif
	$(info :: Installing $(TARGET))
	install -Dm755 $(TARGET) $(INSTALL_DIR)/$(TARGET)
	install -Dm644 man/$(TARGET).1 $(MANPAGE_DIR)/
clean:
	$(info :: Cleaning build)
	rm $(TARGET)
	rm -r $(BUILD_DIR)
uninstall:
ifeq ($(shell test -e $(INSTALL_DIR) && echo -n true),true)
else
	$(eval INSTALL_DIR = $(shell echo $$PATH | cut -d ' ' -f1))
endif

ifeq ($(shell test -e $(MANPAGE_DIR) && echo -n true),true)
else
	$(eval MANPAGE_DIR = $(shell whereis man | awk '{for(i=1; i<=NF; i++) print $$i}' | awk '/man.1.gz/' | awk '{gsub("/man.1.gz", ""); print}'))
endif
	$(info :: Uninstalling $(TARGET))
	rm $(INSTALL_DIR)/$(TARGET)
	rm $(MANPAGE_DIR)/$(TARGET).1

local_options:
	$(info :: compiling without remote support)
	$(eval LIB=)
	$(eval CFLAGS=$(CFLAGS) -D DISABLE_REMOTE)
local: local_options building

cppcheck:
	$(info :: running static code analysis)
	$(info  )
	cppcheck --cppcheck-build-dir=build --std=c++23 --check-level=exhaustive --suppress=unreadVariable --suppress=missingIncludeSystem --enable=all -I $(HEADER_DIR) $(SRC_DIR)

asan: create_directory
	$(info :: compiling with asan)
	$(info )
	cmake -B build -D CMAKE_CXX_COMPILER="$(CC)" -D CMAKE_CXX_FLAGS="$(ASANFLAGS)"
	make -C build 

asan-clean:
	$(info :: cleaning asan build)
	$(info )
	rm -rf build

run-test:
	$(info :: running unit tests)
	$(info )
	cd test && \
	go run simple_syncing.go
clean-test:
	$(info :: cleaning unit test files)
	$(info )
	cd test && \
	rm -r random*
