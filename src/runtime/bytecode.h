#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>

/*
 * A single 32-bit instruction to be interpreted by the segment VM.
 */
typedef struct {

  /*
   * Specify the operation to perform. A seg_opcode constant.
   */
  unsigned opcode: 2;

  /*
   * Operand location. A seg_loc constant.
   */
  unsigned loc: 2;

  /*
   * Operand index.
   */
  unsigned index: 28;
} seg_instruction;

typedef enum {
  SEG_OPCODE_SEND = 0,
  SEG_OPCODE_LOAD,
  SEG_OPCODE_STORE,
  SEG_OPCODECOUNT
} seg_opcode;

typedef enum {
  SEG_LOC_LITERAL = 0,
  SEG_LOC_TEMP,
  SEG_LOCCOUNT
} seg_loc;

#endif
