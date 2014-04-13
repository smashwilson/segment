#!/bin/bash
#
# Parse all source files in ast/. Compare the AST generated from each with the corresponding .ast
# file. If they differ, write the diff to a .diff file.

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

for SRCFILE in ${BASEDIR}/ast/*.seg; do
  EXPECTED_AST=${SRCFILE}.ast
  ACTUAL_AST=${SRCFILE}.ast.actual
  AST_DIFF=${SRCFILE}.diff

  echo -ne "ast: ${HIGHLIGHT}${SRCFILE}${RESET} .."

  if [ ! -e ${EXPECTED_AST} ]; then
    echo -e " ${MISSING}missing${RESET}"
    MISSINGFILES="${MISSINGFILES} ${SRCFILE}"
    let MISSINGCOUNT=MISSINGCOUNT+1
    continue
  fi

  exec 3> /dev/stderr 2> /dev/null
  ${ROOTDIR}/bin/segment --debug ast ${SRCFILE} > ${ACTUAL_AST} 2> /dev/null
  PARSE_CODE=$?
  exec 2>&3

  if [[ ${PARSE_CODE} -ne 0 ]]; then
    # Parse error
    echo -e " ${ERROR}error${RESET} (${PARSE_CODE})"
    ERRORFILES="${ERRORFILES} ${SRCFILE}"
    let ERRORCOUNT=ERRORCOUNT+1
    continue
  fi

  diff ${EXPECTED_AST} ${ACTUAL_AST} > ${AST_DIFF}
  DIFF_CODE=$?

  if [[ ${DIFF_CODE} -eq 0 ]]; then
    echo -e " ${SUCCESS}pass${RESET}"
    [ -z ${AST_KEEP} ] && rm -f ${ACTUAL_AST} ${AST_DIFF}
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
  cmd="${ROOTDIR}/bin/segment --ast-debug ${MISSINGFILE} > ${MISSINGFILE}.ast"
  echo -e " ${MISSING}${cmd}${RESET} # ${HIGHLIGHT}${1}${RESET}"
}

print_details
derive_exit
exit ${EXITCODE}
