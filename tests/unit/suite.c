#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

/* Forward declarations for unit test suites */

CU_pSuite initialize_stringtable_suite(void);
CU_pSuite initialize_ptrtable_suite(void);
CU_pSuite initialize_symboltable_suite(void);

#define ADD_SUITE(name) \
  if (name() == NULL) { \
    CU_cleanup_registry(); \
    return CU_get_error(); \
  }

int main(void)
{
  if (CU_initialize_registry() != CUE_SUCCESS) {
    return CU_get_error();
  }

  ADD_SUITE(initialize_stringtable_suite);
  ADD_SUITE(initialize_symboltable_suite);
  ADD_SUITE(initialize_ptrtable_suite);

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  unsigned int failures = CU_get_number_of_tests_failed();

  CU_cleanup_registry();

  return failures > 0;
}
