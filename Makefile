SRCS := $(wildcard src/*.cpp)
MODS := $(shell find mod -name "*.cpp")
HEADERS := $(shell find mod -name "*.hpp")
DIRS = build bin

all:
	@git submodule update --init --remote --recursive
	@xmake f -m release 
	@xmake -P . 

debug:
	@xmake f -m debug
	@xmake -P .

install:
	@xmake install --root --installdir=/usr

uninstall:
	@xmake uninstall --root --installdir=/usr


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
	clang-format -i $(MODS) $(SRCS) $(HEADERS)
