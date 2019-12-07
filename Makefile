#
# Makefile sistema di prenotazione posti per sala cinematografica
# Sistema operativo Linux (RHEL 8.1) con compilatore gcc 8.3.1
#

TARGET ?= cinema_server
DAEMON ?= cinemad
CC ?= gcc
LD ?= gcc

DMN_DIR ?= $(shell printenv HOME)/.cinema
BIN_DIR ?= ./bin
SRC_DIR ?= ./src
INC_DIR ?= ./include
OBJ_DIR ?= ./build

SRCS := $(shell find $(SRC_DIR) ! -name daemon.c -name *.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)

DMN_SRCS := $(shell find $(SRC_DIR) ! -name server.c -name *.c)
DMN_OBJS := $(DMN_SRCS:%=$(OBJ_DIR)/$(DAEMON)/%.o)

CFLAGS ?= -Wall $(addprefix -I,$(INC_DIR))
LFLAGS ?= -pthread

$(BIN_DIR)/$(TARGET): $(OBJS) $(DMN_DIR)/$(BIN_DIR)/$(DAEMON)
	$(CC) $(LFLAGS) $(OBJS) -o $@
$(OBJ_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(DMN_DIR)/$(BIN_DIR)/$(DAEMON): $(DMN_OBJS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(LFLAGS) $(DMN_OBJS) -o $@
$(OBJ_DIR)/$(DAEMON)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
.PHONY: remove

clean:
	$(RM) -r $(OBJ_DIR)

remove:
	$(RM) -r $(OBJ_DIR)
	$(RM) -r $(DMN_DIR)

MKDIR_P ?= mkdir -p
