CC=clang
CFLAGS = -std=c99
LDFLAGS = -Wall -g -lglfw -lvulkan -ldl -lX11 -lXxf86vm -lXrandr -lXi
LDLIBS = 

SRC=./src
OBJ=./obj
BIN=./bin
INCLUDE=./include
TETRIS = $(BIN)/tetris

.PHONY: all test clean

all: clean build test

build: $(TETRIS)

$(TETRIS): $(SRC)/main.c
	$(CC) $(CFLAGS) $(wildcard $(SRC)/*.c) -o $(TETRIS) $(LDFLAGS)

test: build
	./$(TETRIS)

clean:
	rm -rf $(BIN)/*