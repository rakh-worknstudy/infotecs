include make.cfg

.PHONY: build

all: build clean

build:
	$(MAKE) -C ./$(SRC) build
build-test:
	$(MAKE) -C ./$(SRC) build-test

clean:
	$(MAKE) -C ./$(SRC) clean-all

mkdirs:
	mkdir -p ./$(BUILD)/$(RELEASE)/lib
	mkdir -p ./$(BUILD)/$(DEBUG)/lib

run: run-debug
run-debug:
	(cd ./$(BUILD)/$(DEBUG); ./$(EXEC))
run-release:
	(cd ./$(BUILD)/$(RELEASE); ./$(EXEC))

