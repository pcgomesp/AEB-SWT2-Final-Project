BINFOLDER := bin/
INCFOLDER := inc/
SRCFOLDER := src/
OBJFOLDER := obj/
TESTFOLDER := test/

CC := gcc
CFLAGS := -Wall -lpthread

SRCFILES := $(wildcard $(SRCFOLDER)*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/controller.o obj/mq_utils.o obj/leitura.o -o bin/controller_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/sensors.o obj/mq_utils.o obj/leitura.o -o bin/sensors_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/pedals.o obj/mq_utils.o obj/leitura.o -o bin/pedals_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/actuators.o obj/mq_utils.o obj/leitura.o -o bin/actuators_bin -I$(INCFOLDER)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCFOLDER)

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
	rm -rf test/test_mq_utils

run:
	bin/controller_bin

test: test/test_mq_utils
	./test/test_mq_utils

test/test_mq_utils: test/test_mq_utils.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils.c src/mq_utils.c test/unity.c -o test/test_mq_utils -I$(TESTFOLDER) -I$(INCFOLDER)
