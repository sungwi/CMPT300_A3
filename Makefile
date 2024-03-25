# Compiler settings 
CC=gcc
CFLAGS=-Iinclude -Wall
LDFLAGS=

# Source and object files
SRC_DIR=src
LIB_DIR=lib
OBJ=$(SRC_DIR)/simulation.o $(SRC_DIR)/pcb.o $(LIB_DIR)/list.o

# Executable name
EXEC=simulation
LINK_NAME=simulation
BIN_DIR=$(HOME)/bin

all: $(EXEC) link

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

link:
	@mkdir -p $(BIN_DIR)
	@ln -sf $(PWD)/$(EXEC) $(BIN_DIR)/$(LINK_NAME)

.PHONY: clean

clean:
	rm -f $(SRC_DIR)/*.o $(EXEC)
	rm -f $(BIN_DIR)/$(LINK_NAME)1