default:all

all:tips watch_3g watch_nic

tips:
	-@echo  "\033[32m Building watchdog ... \033[0m"

ifndef TOP_DIR
TOP_DIR = $(shell pwd)/../
endif

include ../env.mk

INC+= -I$(TOP_DIR)/watchdog/include -I$(TOP_DIR)/libgpio/include -I$(TOP_DIR)/include


LD:= -lpthread -L$(TOP_DIR)/gpio/lib

CFLAGS+=$(INC)
LDFLAGS+=$(LD)

watchdog:watchdog.o auto_conn.o
	$(CC) $^ -o $@ $(LD)

watch_3g:watch_3g.o 
	$(CC) $^ -o $@ $(LD) $(TOP_DIR)/libgpio/lib/libgpio.a

watch_nic:watch_nic.o 
	$(CC) $^ -o $@ 

%.o:%.c
	$(CC) -g -c $(CFLAGS) $< -o $@

clean:
	-@$(RM) *.o *.a watchdog watch_3g watch_nic
.PHONY:clean
