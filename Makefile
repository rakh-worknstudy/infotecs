include make.cfg

.PHONY: build

all: build clean

build: build-debug
build-debug:
	$(MAKE) -C ./$(SRC) build-debug
build-release:
	$(MAKE) -C ./$(SRC) build-release

clean:
	$(MAKE) -C ./$(SRC) clean-all

mkdirs:
	mkdir -p ./$(BUILD)/$(RELEASE)/lib
	mkdir -p ./$(BUILD)/$(DEBUG)/lib

run: run-debug
run-debug:
	(cd ./$(BUILD)/$(DEBUG); ./$(TEST_EXEC))
run-release:
ifndef journal
	$(error journal is undefined)
endif
ifndef level
	$(error level is undefined)
endif
	(cd ./$(BUILD)/$(RELEASE); ./$(EXEC) $(journal) $(level))

