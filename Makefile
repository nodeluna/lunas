SRCS := $(wildcard src/*.cpp)
MODS := $(shell find mod -name "*.cpp")
DIRS = build bin
CMAKE_CMD=cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_CXX_COMPILER=clang++ -Wno-dev

all:
	@$(CMAKE_CMD) -DCMAKE_BUILD_TYPE=Release
	@ninja -C build

debug:
	@$(CMAKE_CMD) -DCMAKE_BUILD_TYPE=Debug
	@ninja -C build

install: 
	@ninja -C build install

uninstall:
	@ninja -C build uninstall


clean:
	 $(info :: cleaning build)
	 @for i in $(DIRS); do\
		 if [ -f "$$i" ] || [ -d "$$i" ]; then\
		 	rm -r "$$i";\
		 fi;\
		 done

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
	clang-format -i $(MODS) $(SRCS)
