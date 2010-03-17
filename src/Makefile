# $Id $

# Makefile for Rom24. Works fine on my Debian system.
# You may need to use 'gmake' on BSD systems.

CC = gcc
RM = rm
EXE = rom
PROF = -O -ggdb

# Use these two lines to use crypt(), ie on Linux systems.
# C_FLAGS = $(PROF) -Wall
# L_FLAGS = $(PROF) -lcrypt

# Uncomment these two lines to use plaintext passwords.
# This is how you fix the 'crypt' linking errors!
C_FLAGS = -Wall $(PROF) -DNOCRYPT -DQMFIXES
L_FLAGS = $(PROF)

# Source Files
SRC_FILES := $(wildcard *.c)

# Object Files
OBJ_DIR = obj
OBJ_FILES := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

rom: $(OBJ_FILES)
	$(RM) -f $(EXE)
	$(CC) $(L_FLAGS) -o $(EXE) $(OBJ_FILES)

$(OBJ_DIR)/%.o: %.c
	$(CC) $(C_FLAGS) -c -o $@ $<

clean:
	$(RM) -f $(OBJ_FILES) $(EXE) *~ *.bak *.orig *.rej

distro:
	make clean
	#cd ../ && $(RM) -f log/*.log
	#cd ../ && $(RM) -f player/*
	#cd ../ && $(RM) -f gods/*
	#cd ../ && $(RM) -f notes/*
	cd ../../ && tar zvcf quickmud-`date -I`.tar.gz --exclude=QuickMUD/player/* --exclude=QuickMUD/log/* --exclude=QuickMUD/gods/* --exclude=QuickMUD/notes/* QuickMUD
	
