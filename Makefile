#
# Makefile sistema di prenotazione posti per sala cinematografica
# Sistema operativo Linux (RHEL 8.1) con compilatore gcc 8.3.1
#

SERVER := cinemad
MGCLNT := cinemactl
CC ?= gcc


SRV_DIR ?= $(shell printenv HOME)/.cinema
BIN_DIR ?= ./bin
OBJ_DIR ?= ./obj
LIB_DIR ?= ./libraries

MGC_SRCS := $(shell find ./$(MGCLNT) $(LIB_DIR) -name *.c)
MGC_OBJS := $(MGC_SRCS:%=./$(MGCLNT)/$(OBJ_DIR)/%.o)

SRV_SRCS := $(shell find ./$(SERVER) $(LIB_DIR) -name *.c)
SRV_OBJS := $(SRV_SRCS:%=./$(SERVER)/$(OBJ_DIR)/%.o)

CFLAGS ?= -w $(addprefix -I,$(LIB_DIR))
LFLAGS ?= -pthread

$(MGCLNT)/$(BIN_DIR)/$(MGCLNT): $(MGC_OBJS) $(SRV_DIR)/$(BIN_DIR)/$(SERVER)
	$(MKDIR_P) $(dir $@)
	$(CC) $(LFLAGS) $(MGC_OBJS) -o $@
$(MGCLNT)/$(OBJ_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(SRV_DIR)/$(BIN_DIR)/$(SERVER): $(SRV_OBJS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(LFLAGS) $(SRV_OBJS) -o $@
$(SERVER)/$(OBJ_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	
.PHONY: clean remove debug

clean:
	$(RM) -r $(SERVER)/$(OBJ_DIR)
	$(RM) -r $(MGCLNT)/$(OBJ_DIR)

remove:
	$(RM) -r $(SERVER)/$(OBJ_DIR)
	$(RM) -r $(MGCLNT)/$(OBJ_DIR)
	$(RM) -r $(SRV_DIR)

MKDIR_P ?= mkdir -p
