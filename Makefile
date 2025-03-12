BINFOLDER := bin/
INCFOLDER := inc/
SRCFOLDER := src/
OBJFOLDER := obj/

CC := gcc
CFLAGS := -Wall -lpthread

SRCFILES := $(wildcard $(SRCFOLDER)*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/controller.o obj/mq_utils.o obj/leitura.o -o bin/controller_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/sensors.o obj/mq_utils.o obj/leitura.o -o bin/sensors_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/pedals.o obj/mq_utils.o obj/leitura.o -o bin/pedals_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/actuators.o obj/mq_utils.o obj/leitura.o -o bin/actuators_bin -I$(INCFOLDER)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./inc

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*

run:
	bin/controller_bin
