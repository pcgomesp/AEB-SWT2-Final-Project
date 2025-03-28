BINFOLDER := bin/
INCFOLDER := inc/
SRCFOLDER := src/
OBJFOLDER := obj/
TESTFOLDER := test/

CC := gcc
CFLAGS := -Wall -lpthread -I$(INCFOLDER)

SRCFILES := $(wildcard $(SRCFOLDER)*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/sensors.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o -o bin/sensors_bin
	$(CC) $(CFLAGS) obj/actuators.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o -o bin/actuators_bin
	$(CC) $(CFLAGS) obj/aeb_controller.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o obj/ttc_control.o -o bin/aeb_controller_bin -lm -lrt
	$(CC) $(CFLAGS) obj/main.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o -o bin/main_bin


obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
	rm -f test/test_mq_utils
	rm -f test/test_mq_utils_read

run:
	./bin/main_bin

test: build_test_executables
	./test/test_mq_utils
	./test/test_mq_utils_read

.PHONY: build_test_executables
build_test_executables: test/test_mq_utils test/test_mq_utils_read

test/test_mq_utils: test/test_mq_utils.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils.c src/mq_utils.c test/unity.c -o test/test_mq_utils -I$(TESTFOLDER)

test/test_mq_utils_read: test/test_mq_utils_read.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils_read.c src/mq_utils.c test/unity.c -o test/test_mq_utils_read -I$(TESTFOLDER)

cppcheck:
	cppcheck --addon=misra -I ./inc --force --library=posix $(SRCFOLDER) $(INCFOLDER)

.PHONY: docs
docs:
	doxygen Doxyfile

.PHONY: clean-docs
clean-docs:
	rm -rf docs/html
	rm -rf docs/latex
	rm -rf docs/rtf