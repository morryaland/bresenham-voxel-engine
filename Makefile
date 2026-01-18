TARGET=unk
CC=gcc
XXD=xxd
MKDIR=mkdir -p

SRC_PATH = ./src
OBJ_PATH = ./obj

SRC = $(shell find $(SRC_PATH) -type f -name '*.c') $(SRC_PATH)/ray.c
OBJ = $(patsubst $(SRC_PATH)/%.c, $(OBJ_PATH)/%.o,$(SRC))

CFLAGS+= -g -O2 -DNDEBUG
LDFLAGS+= -L/usr/local/lib -lm -lSDL3 -lOpenCL

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(MKDIR) $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SRC_PATH)/ray.c: $(SRC_PATH)/ray.cl
	$(XXD) -n ray_cl -a -i $< > $@

clean:
	rm $(OBJ_PATH)/*
	rm $(TARGET)
