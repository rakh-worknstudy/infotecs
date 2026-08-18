ROOT=./../
include $(ROOT)/make.cfg

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(SRCS:%.cpp=TEST_%.o)

DIR_DEBUG := $(ROOT)/$(BUILD)/$(DEBUG)/
DIR_RELEASE := $(ROOT)/$(BUILD)/$(RELEASE)/

all: build clean

build: $(OBJS) build-lib-debug build-exec build-debug

$(OBJS): %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
$(TEST_OBJS): TEST_%.o: %.cpp
	$(CC) $(CFLAGS) -D _BUILD_TEST -c $< -o $@

build-lib: build-lib-debug
build-lib-debug:
	$(MAKE) -C ./lib build-debug
build-lib-release:
	$(MAKE) -C ./lib build-release

build-exec: build-exec-debug
build-exec-debug: $(TEST_OBJS) build-lib-debug
	$(CC) $(CFLAGS) -Wl,-rpath=\$$ORIGIN/lib $(TEST_OBJS) -o $(TEST_EXEC) -Llib -llogger
build-exec-release: $(OBJS) build-lib-release
	$(CC) $(CFLAGS) -Wl,-rpath=\$$ORIGIN/lib $(OBJS) -o $(EXEC) -Llib -llogger

build-debug: $(TEST_OBJS) build-lib-debug build-exec-debug mkdirs
	cp $(TEST_EXEC) $(DIR_DEBUG)
build-release: $(OBJS) build-lib-release build-exec-release mkdirs
	cp $(EXEC) $(DIR_RELEASE)

clean-all: clean
	$(MAKE) -C ./lib clean
clean:
	-rm $(OBJS) $(EXEC)
	-rm $(TEST_OBJS) $(TEST_EXEC)

mkdirs:
	$(MAKE) -C $(ROOT) mkdirs

