#!/bin/bash
#
# Lex all source files in lexer/. Compare the lexer output with the corresponding .lex file. If
# they differ, write the diff to a .diff file.

# Directories.

BASEDIR=`dirname $0`
ROOTDIR=${BASEDIR}/..

source ${BASEDIR}/helpers.sh

# Accumulate filenames of failing tests for a final report.

PASSCOUNT=0
FAILFILES=
FAILCOUNT=0
ERRORFILES=
ERRORCOUNT=0
MISSINGFILES=
MISSINGCOUNT=0

# Give each file to the lexer.

for SRCFILE in ${BASEDIR}/lexer/*.seg; do
  EXPECTED_LEX=${SRCFILE}.lex
  ACTUAL_LEX=${SRCFILE}.lex.actual
  LEX_DIFF=${SRCFILE}.diff

  echo -ne "lex: ${HIGHLIGHT}${SRCFILE}${RESET} .."

  if [ ! -e ${EXPECTED_LEX} ]; then
    echo -e " ${MISSING}missing${RESET}"
    MISSINGFILES="${MISSINGFILES} ${SRCFILE}"
    let MISSINGCOUNT=MISSINGCOUNT+1
    continue
  fi

  exec 3> /dev/stderr 2> /dev/null
  ${ROOTDIR}/bin/segment --phase lexer --debug lexer ${SRCFILE} > ${ACTUAL_LEX} 2> /dev/null
  LEX_CODE=$?
  exec 2>&3

  if [[ ${LEX_CODE} -ne 0 ]]; then
    # Parse error
    echo -e " ${ERROR}error${RESET} (${LEX_CODE})"
    ERRORFILES="${ERRORFILES} ${SRCFILE}"
    let ERRORCOUNT=ERRORCOUNT+1
    continue
  fi

  diff ${EXPECTED_LEX} ${ACTUAL_LEX} > ${LEX_DIFF}
  DIFF_CODE=$?

  if [[ ${DIFF_CODE} -eq 0 ]]; then
    echo -e " ${SUCCESS}pass${RESET}"
    [ -z ${LEX_KEEP} ] && rm -f ${ACTUAL_LEX} ${LEX_DIFF}
    let PASSCOUNT=PASSCOUNT+1
  else
    echo -e " ${FAILURE}fail${RESET}"
    FAILFILES="${FAILFILES} ${SRCFILE}"
    let FAILCOUNT=FAILCOUNT+1
  fi
done

# Print a summary of this run.
print_summary

print_missing () {
  cmd="${ROOTDIR}/bin/segment --lexer-debug ${MISSINGFILE} > ${MISSINGFILE}.lex"
  echo -e " ${MISSING}${cmd}${RESET} # ${HIGHLIGHT}${1}${RESET}"
}

print_details
derive_exit
exit ${EXITCODE}
