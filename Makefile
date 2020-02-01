#
# Makefile sistema di prenotazione posti per sala cinematografica
# Sistema operativo Linux (RHEL 8.1) con compilatore gcc 8.3.1
#

TARGET ?= cinemad
MGCLNT ?= cinemactl
CC ?= gcc
LD ?= gcc

DMN_DIR ?= $(shell printenv HOME)/.cinema
BIN_DIR ?= ./bin
SRC_DIR ?= ./src
INC_DIR ?= ./include
OBJ_DIR ?= ./build
LIB_DIR ?= ../lib

SRCS := $(shell find $(SRC_DIR) $(LIB_DIR)/$(SRC_DIR) ! -name cinemad.c -name *.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/$(MGCLNT)/%.o)

DMN_SRCS := $(shell find $(SRC_DIR) $(LIB_DIR)/$(SRC_DIR) ! -name cinemactl.c -name *.c)
DMN_OBJS := $(DMN_SRCS:%=$(OBJ_DIR)/$(TARGET)/%.o)

CFLAGS ?= -Wall $(addprefix -I,$(INC_DIR)) $(addprefix -I,$(LIB_DIR)/$(INC_DIR))
LFLAGS ?= -pthread

$(BIN_DIR)/$(MGCLNT): $(OBJS) $(DMN_DIR)/$(BIN_DIR)/$(TARGET)
	$(MKDIR_P) $(dir $@)
	$(CC) $(LFLAGS) $(OBJS) -o $@
$(OBJ_DIR)/$(MGCLNT)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(DMN_DIR)/$(BIN_DIR)/$(TARGET): $(DMN_OBJS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(LFLAGS) $(DMN_OBJS) -o $@
$(OBJ_DIR)/$(TARGET)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean rebase clean-all

clean:
	$(RM) -r $(OBJ_DIR)

rebase:
	$(RM) -r $(DMN_DIR)
	make

clean-all:
	$(RM) -r $(OBJ_DIR)
	$(RM) -r $(DMN_DIR)

MKDIR_P ?= mkdir -p
