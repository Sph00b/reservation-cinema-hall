#
# Makefile sistema di prenutazione posti per sala cinematografica
# Sistema operativo Linux (RHEL 8.1) con compilatore gcc 8.3.1
# Utilizzo di macro
#
TARGET ?= cinema_server
CC ?= gcc
LD ?= gcc

BIN_DIR ?= ./bin
SRC_DIR ?= ./src
INC_DIR ?= ./include
OBJ_DIR ?= ./build

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)

CFLAGS ?= -Wall $(addprefix -I,$(INC_DIR))
LFLAGS ?= -pthread

$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(OBJ_DIR)

MKDIR_P ?= mkdir -p
