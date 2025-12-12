/* File: src/tests/main_test.c */
#include "test_framework.h"

/* Forward declare tests */
int test_logging_init(void);

int main(void) {
    RUN_TEST(test_logging_init);
    
    TEST_REPORT();
    
    return (tests_run != tests_passed);
}

