TARGET=unk
CC=gcc
MKDIR=mkdir -p

SRC_PATH = ./src
OBJ_PATH = ./obj

SRC = $(shell find $(SRC_PATH) -type f -name '*.c')
OBJ = $(patsubst $(SRC_PATH)%.c, $(OBJ_PATH)%.o,$(SRC))

CFLAGS+= -g -O3
LDFLAGS+= -L/usr/local/lib -lm -lSDL3

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(MKDIR) $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(OBJ_PATH)/*
	rm $(TARGET)
