#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "ds/hashtable_tests.h"

int main(void)
{
  if (CU_initialize_registry() != CUE_SUCCESS) {
    return CU_get_error();
  }

  if (
    (initialize_hashtable_suite() == NULL)
  ) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  unsigned int failures = CU_get_number_of_tests_failed();

  CU_cleanup_registry();

  return failures > 0;
}
