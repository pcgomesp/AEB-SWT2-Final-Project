BINFOLDER := bin/
INCFOLDER := inc/
SRCFOLDER := src/
OBJFOLDER := obj/
TESTFOLDER := test/

CC := gcc
CFLAGS := -Wall -lpthread -lm -lrt -I$(INCFOLDER)
TESTFLAGS := -DUNITY_OUTPUT_COLOR -DTEST_MODE
COVFLAGS := -fprofile-arcs -ftest-coverage

SRCFILES := $(wildcard $(SRCFOLDER)*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/sensors.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o -o bin/sensors_bin
	$(CC) $(CFLAGS) obj/actuators.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o -o bin/actuators_bin
	$(CC) $(CFLAGS) obj/aeb_controller.o obj/mq_utils.o obj/file_reader.o obj/log_utils.o obj/dbc.o obj/ttc_control.o -o bin/aeb_controller_bin
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
	@for test in $^; do \
		echo "\n\033[1;33mTest $$test results:\033[0m"; \
		./$$test; \
	done

test/test_mq_utils: test/test_mq_utils.c src/mq_utils.c test/unity.c
	$(CC) $(CFLAGS) $(TESTFLAGS) test/test_mq_utils.c src/mq_utils.c test/unity.c -o test/test_mq_utils -I$(TESTFOLDER)

test/test_ttc: test/test_ttc.c src/ttc_control.c test/unity.c
	$(CC) $(CFLAGS) $(TESTFLAGS) test/test_ttc.c src/ttc_control.c test/unity.c -o test/test_ttc -I$(TESTFOLDER) -lm

test/test_file_reader: test/test_file_reader.c src/file_reader.c test/unity.c
	$(CC) $(CFLAGS) $(TESTFLAGS) test/test_file_reader.c src/file_reader.c test/unity.c -o test/test_file_reader -I$(TESTFOLDER)

test/test_log_utils: test/test_log_utils.c src/log_utils.c test/unity.c
	$(CC) $(CFLAGS) $(TESTFLAGS) -Wl,--wrap=fopen -Wl,--wrap=perror test/test_log_utils.c src/log_utils.c test/unity.c -o test/test_log_utils -I$(TESTFOLDER)

test/test_actuators: test/test_actuators.c src/actuators.c test/unity.c
	$(CC) $(CFLAGS) -DTEST_MODE test/test_actuators.c src/actuators.c test/unity.c -o test/test_actuators -Iinc -Itest -lpthread	

test/test_aeb_controller: test/test_aeb_controller.c src/aeb_controller.c src/ttc_control.c test/unity.c
	$(CC) $(CFLAGS) -DTEST_MODE test/test_aeb_controller.c src/aeb_controller.c src/ttc_control.c test/unity.c -o test/test_aeb_controller -I$(TESTFOLDER) -lm

.SILENT: cov
cov:
	if [ -z "$(src_file)" ] || [ -z "$(test_file)" ]; then \
		echo "Please provide both src_file and test_file as arguments, e.g., 'make cov src_file=example.c test_file=test_example.c'"; \
	else \
		echo "Running gcov for source file $(src_file) and test $(test_file)"; \
		echo ""; \
		if [ "$(test_file)" = "test_log_utils.c" ]; then \
			WRAP_FLAGS="-Wl,--wrap=fopen -Wl,--wrap=perror"; \
		else \
			WRAP_FLAGS=""; \
		fi; \
		$(CC) $(TESTFLAGS) $(COVFLAGS) $$WRAP_FLAGS $(SRCFOLDER)$(src_file) $(TESTFOLDER)$(test_file) $(TESTFOLDER)unity.c -I$(INCFOLDER) -o $(OBJFOLDER)$(test_file:.c=)_gcov_bin; \
		./$(OBJFOLDER)$(test_file:.c=)_gcov_bin > /dev/null 2>&1; \
		gcov -b $(SRCFOLDER)$(src_file) -o $(OBJFOLDER)$(test_file:.c=)_gcov_bin-$(src_file:.c=.gcda); \
	fi

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
	rm -f *.gcov
	rm -f *.gcda
	rm -f *.gcno