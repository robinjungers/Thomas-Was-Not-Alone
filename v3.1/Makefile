CC = gcc
CFLAGS = -Wall 
LDFLAGS = -lSDL -lSDLmain -lSDL_mixer -lSDL_image -lm

APP_BIN = Game

SRC_PATH = src
OBJ_PATH = obj
INC_PATH = -I include
BIN_PATH = bin
LIB_PATH = lib

SRC_FILES = $(shell find $(SRC_PATH) -type f -name '*.c')
OBJ_FILES = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o, $(SRC_FILES))

all: $(APP_BIN)

$(APP_BIN): $(OBJ_FILES)
	@mkdir -p $(BIN_PATH)
	$(CC) -o $(BIN_PATH)/$(APP_BIN) $(OBJ_FILES) $(LDFLAGS) -L/usr/local/lib -framework Cocoa -framework OpenGL
	@echo "*****************************"
	@echo "******** ./bin/Game *********"
	@echo "*****************************"

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@mkdir -p "$(@D)"
	$(CC) -c $< -o $@ $(CFLAGS) $(INC_PATH) -I/usr/local/include

clean:
	rm $(OBJ_FILES) $(BIN_PATH)/$(APP_BIN)
	@echo "*****************************"
	@echo "******** Cleaned ! **********"
	@echo "*****************************"
