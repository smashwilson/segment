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

/*
 * Assert that a seg_err is SEG_OK. If it isn't, print its error as a failure message.
 */
#define SEG_ASSERT_OK(err) \
  do { \
    if (err != SEG_OK) { \
      fprintf(stderr, "\nerror: %s\n", err->message); \
      CU_FAIL_FATAL(err->message); \
    } \
  } while(0)

#endif
