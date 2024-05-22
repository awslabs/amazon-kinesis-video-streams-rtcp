APP_NAME = "rtcp_app.bin"

# Source Files
SRCS += "rtcp_app.c"
SRCS += "../source/rtcp_api.c"
SRCS += "../source/rtcp_endianness.c"

# Include Directories
INCLUDE_DIRS += "-I../source/include/"

OBJS=$(SRCS:.c=.o)

CFLAGS+=-ggdb

.DEFAULT_GOAL:=all

all: build run

build:
	$(CC) -o $(APP_NAME) $(SRCS) $(INCLUDE_DIRS) $(CFLAGS)

run:
	./$(APP_NAME)

clean:
	rm -rf $(APP_NAME)

.PHONY: all build run clean