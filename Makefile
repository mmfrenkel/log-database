# Makefile inspired by: Chnossos @ https://stackoverflow.com/questions/30573481/path-include-and-src-directory-makefile

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

EXE = $(BIN_DIR)/lsm-system
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CFLAGS =  -g -Wall -std=c11 -fmax-errors=10
LDFLAGS =  -g

.PHONY: all clean

all: clean $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rv $(BIN_DIR) $(OBJ_DIR)
