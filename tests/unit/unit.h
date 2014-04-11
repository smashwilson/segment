#ifndef UNIT
#define UNIT

/**
 * Register the test function +name+ with CUnit.  Use within an
 * +initialize_xx_suite+ function to cleanly register a test case.
 */
#define ADD_TEST(name) \
    if (CU_add_test(pSuite, #name, name) == NULL) {\
        return NULL;\
    }

#endif
