CC=gcc
CFLAGS=-Iinclude -std=c11 -Wall -Wextra -O2
TARGET=build/soj
SRCS=$(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRCS) include/oaj.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f workspace/executables/* workspace/outputs/* workspace/errors/*

reset-data:
	rm -f data/users.dat data/problems.dat data/submissions.dat data/promotions.dat
	rm -rf workspace/sources/* workspace/executables/* workspace/outputs/* workspace/errors/*

.PHONY: all run clean reset-data
