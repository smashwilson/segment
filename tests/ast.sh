#!/bin/bash
#
# Parse all source files in ast/. Compare the AST generated from each with the corresponding .ast
# file. If they differ, write the diff to a .diff file.

BASEDIR=`dirname $0`
ROOTDIR=${BASEDIR}/..

# Shell colors.

prefix="\033["
WHITE="${prefix}1;37m"
LTGREEN="${prefix}1;32m"
LTRED="${prefix}1;31m"
DKRED="${prefix}0;31m"
RESET="${prefix}0m"

# Output functions.

pluralize () {
  echo -n "$1 $2"
  [[ $1 -ne 1 ]] && echo -n "s"
}

# Accumulate filenames of failing tests for a final report.

PASSCOUNT=0
FAILFILES=
FAILCOUNT=0
ERRORFILES=
ERRORCOUNT=0

for SRCFILE in ${BASEDIR}/ast/*.seg; do
  EXPECTED_AST=${SRCFILE}.ast
  ACTUAL_AST=${SRCFILE}.ast.actual
  AST_DIFF=${SRCFILE%.*}.diff

  echo -ne "ast: ${WHITE}${SRCFILE}${RESET} .."

  exec 3> /dev/stderr 2> /dev/null
  ${ROOTDIR}/bin/segment --ast-debug ${SRCFILE} > ${ACTUAL_AST} 2> /dev/null
  PARSE_CODE=$?
  exec 2>&3

  if [[ ${PARSE_CODE} -eq 0 ]]; then
    diff ${EXPECTED_AST} ${ACTUAL_AST} > ${AST_DIFF}
    DIFF_CODE=$?

    if [[ ${DIFF_CODE} -eq 0 ]]; then
      echo -e " ${LTGREEN}pass${RESET}"
      [ -z ${AST_KEEP} ] && rm -f ${ACTUAL_AST} ${AST_DIFF}
      let PASSCOUNT=PASSCOUNT+1
    else
      echo -e " ${LTRED}fail${RESET}"
      FAILFILES="${FAILFILES} ${SRCFILE}"
      let FAILCOUNT=FAILCOUNT+1
    fi
  else
    # Parse error
    echo -e " ${DKRED}error${RESET} (${PARSE_CODE})"
    ERRORFILES="${ERRORFILES} ${SRCFILE}"
    let ERRORCOUNT=ERRORCOUNT+1
  fi
done

# Print a summary of this run.
let TOTALCOUNT=PASSCOUNT+FAILCOUNT+ERRORCOUNT

echo
echo -ne "Summary: of ${WHITE}"
pluralize ${TOTALCOUNT} test
echo -ne ${RESET}
[[ ${PASSCOUNT} -ne 0 ]] && echo -ne ", ${LTGREEN}${PASSCOUNT} passed${RESET}"
[[ ${FAILCOUNT} -ne 0 ]] && echo -ne ", ${LTRED}${FAILCOUNT} failed${RESET}"
[[ ${ERRORCOUNT} -ne 0 ]] && echo -ne ", ${DKRED}${ERRORCOUNT} caused errors${RESET}"
echo "."
