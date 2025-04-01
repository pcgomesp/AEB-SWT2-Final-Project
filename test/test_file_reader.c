#include "unity.h"
#include "file_reader.h"
#include <sys/stat.h>

const char *test_filename = "cts/cenario.txt";
FILE *test_file;

void setUp()
{
    test_file = open_file(test_filename);
}

void tearDown()
{
    if(test_file)
    {
        fclose(test_file);
        test_file = NULL;
    }
}

void test_open_file_not_null()
{
    // Checks if test_file pointer is not null
    TEST_ASSERT_NOT_NULL(test_file);
    
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_file_not_null);
    return UNITY_END();

}