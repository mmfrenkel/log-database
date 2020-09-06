# Makefile inspired by: Chnossos @ https://stackoverflow.com/questions/30573481/path-include-and-src-directory-makefile

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
LOG_DIR = logs

EXE = $(BIN_DIR)/lsm-system
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CFLAGS =  -g -Wall -std=c11 -fmax-errors=10
LDFLAGS =  -g

.PHONY: all clean delete

all: clean $(BIN_DIR) $(OBJ_DIR) $(LOG_DIR) $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(LOG_DIR):
	mkdir -p $@

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

delete:
	rm -rf $(LOG_DIR)
