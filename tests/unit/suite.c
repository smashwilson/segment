#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static void null_test_success(void **state)
{
  (void) state;
}

int main(void)
{
  const UnitTest tests[] = {
    unit_test(null_test_success),
  };

  return run_tests(tests);
}
