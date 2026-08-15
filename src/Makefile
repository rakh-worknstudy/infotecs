ROOT=./../
include $(ROOT)/make.cfg

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

DIR_DEBUG := $(ROOT)/$(BUILD)/$(DEBUG)/
DIR_RELEASE := $(ROOT)/$(BUILD)/$(RELEASE)/

all: build clean

build: $(OBJS) build-lib build-exec build-debug
$(OBJS): %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build-lib:
	$(MAKE) -C ./lib build

build-exec: $(OBJS)
	$(CC) $(CFLAGS) -Wl,-rpath=\$$ORIGIN/lib $(OBJS) -o $(EXEC) -Llib -llogger

build-debug: $(OBJS) mkdirs
	cp $(EXEC) $(DIR_DEBUG)
build-release: $(OBJS) mkdirs
	cp $(EXEC) $(DIR_RELEASE)

clean-all:
	$(MAKE) -C ./lib clean
clean:
	rm $(OBJS) $(EXEC)

mkdirs:
	$(MAKE) -C $(ROOT) mkdirs

