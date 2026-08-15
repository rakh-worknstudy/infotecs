ROOT=./../../
include $(ROOT)/make.cfg

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
LIBS = liblogger.so

DIR_DEBUG := $(ROOT)/$(BUILD)/$(DEBUG)/
DIR_RELEASE := $(ROOT)/$(BUILD)/$(RELEASE)/

all: build clean
build: $(LIBS) build-debug

$(OBJS): %.o: %.cpp
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(LIBS): lib%.so: %.o
	$(CC) $(CFLAGS) -shared $< -o $@

clean:
	rm $(OBJS)

build-debug: $(LIBS) mkdirs
	cp $(LIBS) $(DIR_DEBUG)/lib/
build-release: $(LIBS) mkdirs
	cp $(LIBS) $(DIR_RELEASE)/lib/

mkdirs:
	$(MAKE) -C $(ROOT) mkdirs

