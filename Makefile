BINFOLDER := bin/
INCFOLDER := inc/
SRCFOLDER := src/
OBJFOLDER := obj/
TESTFOLDER := test/

CC := gcc
CFLAGS := -Wall -lpthread

SRCFILES := $(wildcard $(SRCFOLDER)*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/sensors.o obj/mq_utils.o obj/file_reader.o obj/dbc.o -o bin/sensors_bin -I$(INCFOLDER)
	$(CC) $(CFLAGS) obj/actuators.o obj/mq_utils.o obj/file_reader.o obj/dbc.o -o bin/actuators_bin -I$(INCFOLDER)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCFOLDER)

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
	rm -rf test/test_mq_utils
	rm -rf test/test_mq_utils_read

test: build_test_executables
	./test/test_mq_utils
	./test/test_mq_utils_read

.PHONY: build_test_executables
build_test_executables: test/test_mq_utils test/test_mq_utils_read

test/test_mq_utils: test/test_mq_utils.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils.c src/mq_utils.c test/unity.c -o test/test_mq_utils -I$(TESTFOLDER) -I$(INCFOLDER)

test/test_mq_utils_read: test/test_mq_utils_read.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils_read.c src/mq_utils.c test/unity.c -o test/test_mq_utils_read -I$(TESTFOLDER) -I$(INCFOLDER)
