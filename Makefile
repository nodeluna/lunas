CC = g++
CFLAGS = -O1 -Iinclude -Wall -Wextra -pedantic -std=c++23
TARGET = lunas
PREFIX = /usr
INSTALL_DIR = $(PREFIX)/bin
LIB = $(shell pkg-config --cflags --libs libssh) -pthread
BUILD_DIR = build
HEADER_DIR = include
MANPAGE_DIR = $(PREFIX)/share/man/man1
SRC_DIR = src
SRCS := $(wildcard src/*.cpp)
MODS := $(shell find mod -name "*.cpp")
HEADERS := $(wildcard include/*.h)
OBJS := $(addprefix build/, $(notdir $(SRCS:.cpp=.o)))
DEP := $(OBJS:.o=.d)


ifneq ($(findstring clang++, $(CC)),)
	CFLAGS += -D__cpp_concepts=202002L -Wno-builtin-macro-redefined -Wno-macro-redefined
endif

all: mkdir_build pkg_version $(OBJS) $(TARGET)

debug: append_debug_option all

asan: append_asan_option all

tsan: append_tsan_option all

local: local_options all

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIB) $^ -o $@

install: 
	$(info :: Installing $(TARGET))
	install -Dm755 $(TARGET) $(INSTALL_DIR)/$(TARGET)
	mkdir -p $(INSTALL_DIR) $(MANPAGE_DIR)
	install -Dm644 man/$(TARGET).1 $(MANPAGE_DIR)/

uninstall:
	$(info :: Uninstalling $(TARGET))
	rm $(INSTALL_DIR)/$(TARGET)
	rm $(MANPAGE_DIR)/$(TARGET).1

mkdir_build:
	@for i in $(BUILD_DIR); do\
		if [ ! -f "$$i" ]; then\
			mkdir -p $$i;\
		fi;\
	done

clean:
	$(info :: cleaning build)
	@for i in $(BUILD_DIR) $(TARGET); do\
		if [ -f "$$i" ] || [ -d "$$i" ]; then\
			rm -r "$$i";\
		fi;\
	done

define pkg_version
	@pkg-config --atleast-version=$(1) $(2); \
	STATUS=$$?; \
	if [ $$STATUS -ne 0 ]; then \
		echo -e "\x1b[1;31m-[X] $(2) >= $(1) is required to compile $(TARGET) \x1b[1;0m"; \
	fi
endef

pkg_version:
	$(call pkg_version,0.11.0,libssh)

append_debug_option:
	$(eval CFLAGS=$(CFLAGS) -g)

append_asan_option:
	$(info :: compiling with asan)
	$(eval CFLAGS=$(CFLAGS) -g -fsanitize=address -fsanitize=undefined -fsanitize=float-cast-overflow\
		-fsanitize=float-divide-by-zero -fno-sanitize-recover=all -fsanitize=leak\
		-fsanitize=alignment -fno-omit-frame-pointer)

append_tsan_option:
	$(info :: compiling with tsan)
	$(eval CFLAGS=$(CFLAGS) -g -fsanitize=undefined -fsanitize=float-cast-overflow\
		-fsanitize=float-divide-by-zero -fno-sanitize-recover=all -fsanitize=thread\
		-fsanitize=alignment -fno-omit-frame-pointer)
	$(eval CC=clang++ -D__cpp_concepts=202002L -Wno-builtin-macro-redefined -Wno-macro-redefined)

local_options:
	$(info :: compiling without remote support)
	$(eval LIB= -pthread)
	$(eval CFLAGS=$(CFLAGS) -D DISABLE_REMOTE)

cppcheck: mkdir_build
	$(info :: running static code analysis)
	$(info  )
	cppcheck --cppcheck-build-dir=$(BUILD_DIR) --std=c++23 --check-level=exhaustive --suppress=unreadVariable\
		--suppress=missingIncludeSystem --enable=all -I $(HEADER_DIR) $(SRC_DIR)

run-test:
	$(info :: running unit tests)
	$(info )
	cd test && \
	make
clean-test:
	$(info :: cleaning unit test files)
	$(info )
	cd test && \
	go run clean.go
format:
	clang-format -i $(MODS)


-include $(DEP)
