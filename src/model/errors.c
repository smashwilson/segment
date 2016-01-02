#include "model/errors.h"

struct __seg_err __seg_err_nomem = {
  .code = SEG_CODE_NOMEM,
  .message = "Out of memory"
};

seg_err __seg_create_err(seg_err_code code, const char *msg) {
  seg_err err = malloc(sizeof(seg_err));
  if (err == NULL) {
    return &__seg_err_nomem;
  }
  err->code = code;
  err->message = msg;
  return err;
}
