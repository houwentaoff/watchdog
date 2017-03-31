default:all

all:tips watchdog watch_gprs

tips:
	-@echo  "\033[32m Building watchdog ... \033[0m"

ifndef TOP_DIR
TOP_DIR = $(shell pwd)/../
endif

include ../env.mk

INC+= -I$(TOP_DIR)/watchdog/include -I$(TOP_DIR)/libgpio/include

LD:= -lpthread -L$(TOP_DIR)/gpio/lib

CFLAGS+=$(INC)
LDFLAGS+=$(LD)

watchdog:watchdog.o auto_conn.o
	$(CC) $^ -o $@ $(LD)

watch_gprs:watch_gprs.o 
	$(CC) $^ -o $@ $(LD) $(TOP_DIR)/libgpio/lib/libgpio.a

%.o:%.c
	$(CC) -g -c $(CFLAGS) $< -o $@

clean:
	-@$(RM) *.o *.a watchdog
.PHONY:clean
