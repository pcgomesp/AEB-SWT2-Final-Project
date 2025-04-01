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

run:
	./bin/main_bin

TESTFILES := $(wildcard $(TESTFOLDER)test_*.c)

# Find all test source files and create a list of test executables
TESTS := $(patsubst $(TESTFOLDER)%.c, $(TESTFOLDER)%, $(TESTFILES))

.PHONY: test test_all
test:
	@if [ -z "$(file)" ]; then \
		$(MAKE) --no-print-directory test_all; \
	else \
		test_exe="$(TESTFOLDER)$(basename $(file))"; \
		echo "Compiling and running test $$test_exe"; \
		$(MAKE) --no-print-directory $$test_exe; \
		./$$test_exe; \
	fi

test_all: $(TESTS)
	@for test in $^; do ./$$test; done

test/test_mq_utils: test/test_mq_utils.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils.c src/mq_utils.c test/unity.c -o test/test_mq_utils -I$(TESTFOLDER)

test/test_mq_utils_read: test/test_mq_utils_read.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) test/test_mq_utils_read.c src/mq_utils.c test/unity.c -o test/test_mq_utils_read -I$(TESTFOLDER)

test/test_ttc: test/test_ttc.c src/ttc_control.c test/unity.c
	$(CC) $(CFLAGS) test/test_ttc.c src/ttc_control.c test/unity.c -o test/test_ttc -I$(TESTFOLDER) -lm

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

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
	rm -f $(TESTS)