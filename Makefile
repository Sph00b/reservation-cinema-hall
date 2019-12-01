#
# Makefile sistema di prenutazione posti per sala cinematografica
# Sistema operativo Linux (RHEL 8.1) con compilatore gcc 8.3.1
# Utilizzo di macro
#
CC = gcc
LD = gcc
OBJS = *.o
LOGF = make.log
main:
	@if [ -f $(LOGF) ]; then\
	       rm $(LOGF);\
	fi 
	@make compile >> $(LOGF)
	@make link >> $(LOGF)
	@make clean >> $(LOGF)
compile:
	$(CC) -c ./lib/database.c
	$(CC) -c ./lib/connection.c
	$(CC) -c server.c -DDEBUG
link:
	$(LD) -pthread -o cinema_server $(OBJS)
clean:
	rm $(OBJS)
