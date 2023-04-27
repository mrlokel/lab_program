CC = gcc
APP_NAME = coder

SRC_DIR = src
OBJ_DIR = obj

CFLAGS = -Wall -Werror -Wextra
DEPSFLAGS = -MMD

CODER_SRC = $(wildcard $(SRC_DIR)/zadanie2/*.c)
CODER_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CODER_SRC)) 

DEPS = $(CODER_OBJ:.o=.d)

.PHONY: all dir 

all: dir z1 test coder 

-include $(DEPS)

dir: 
	mkdir -p obj/src/zadanie2
	mkdir -p obj/src/zadanie1

z1: obj/src/zadanie1/z1.o 
	gcc -Wall -o $@ $<

test: obj/src/zadanie1/test.o 
	gcc -Wall -o $@ $<

$(APP_NAME): $(CODER_OBJ) 
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEPSFLAGS) -c -o $@ $<

clean:
	rm -rf test z1 coder obj