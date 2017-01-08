CROSS_COMPILE=arm-linux-
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
INC=-I$(TOP_DIR)/include
export CC AR 

