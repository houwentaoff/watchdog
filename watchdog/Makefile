default:all

all:tips watchdog

tips:
	-@echo  "\033[32m Building watchdog ... \033[0m"

ifndef TOP_DIR
TOP_DIR = $(shell pwd)/../
endif

include ../env.mk

INC+= -I$(TOP_DIR)/include

LD:=

CFLAGS+=$(INC)
LDFLAGS+=$(LD)

watchdog:watchdog.o auto_conn.o
	$(CC) $^ -o $@ $(LD)

%.o:%.c
	$(CC) -g -c $(CFLAGS) $< -o $@

clean:
	-@$(RM) *.o *.a watchdog
.PHONY:clean
