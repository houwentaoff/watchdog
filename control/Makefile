default:all

all:tips

tips:
	-@echo  "\033[32m Building control ... \033[0m"

ifndef TOP_DIR
TOP_DIR = $(shell pwd)/../
endif

include ../env.mk


LD:=

CFLAGS+=$(INC)
LDFLAGS+=$(LD)

%.o:%.c
	$(CC) -g -c $(CFLAGS) $< -o $@

clean:
	-@$(RM) *.o *.a
.PHONY:clean
