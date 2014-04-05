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

for SRCFILE in ${BASEDIR}/ast/*.seg; do
  EXPECTED_AST=${SRCFILE}.ast
  ACTUAL_AST=${SRCFILE}.ast.actual
  AST_DIFF=${SRCFILE%.*}.diff

  echo -ne "ast: ${HIGHLIGHT}${SRCFILE}${RESET} .."

  exec 3> /dev/stderr 2> /dev/null
  ${ROOTDIR}/bin/segment --ast-debug ${SRCFILE} > ${ACTUAL_AST} 2> /dev/null
  PARSE_CODE=$?
  exec 2>&3

  if [[ ${PARSE_CODE} -eq 0 ]]; then
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
  else
    # Parse error
    echo -e " ${ERROR}error${RESET} (${PARSE_CODE})"
    ERRORFILES="${ERRORFILES} ${SRCFILE}"
    let ERRORCOUNT=ERRORCOUNT+1
  fi
done

# Print a summary of this run.
let TOTALCOUNT=PASSCOUNT+FAILCOUNT+ERRORCOUNT

echo
echo -ne "Summary: of ${HIGHLIGHT}"
echo_pluralize ${TOTALCOUNT} test
echo -ne ${RESET}
[[ ${PASSCOUNT} -ne 0 ]] && echo -ne ", ${SUCCESS}${PASSCOUNT} passed${RESET}"
[[ ${FAILCOUNT} -ne 0 ]] && echo -ne ", ${FAILURE}${FAILCOUNT} failed${RESET}"
[[ ${ERRORCOUNT} -ne 0 ]] && echo -ne ", ${ERROR}${ERRORCOUNT} caused errors${RESET}"
echo "."

if [ -n ${FAILFILES} ]; then
  echo
  echo_pluralize ${FAILCOUNT} "failing test"
  echo ":"

  for FAILFILE in ${FAILFILES}; do
    echo -e " ${FAILURE}cat ${FAILFILE%.*}.diff${RESET} # ${HIGHLIGHT}${FAILFILE}${RESET}"
  done
fi

if [[ -n ${ERRORFILES} ]]; then
  echo
  echo_pluralize ${ERRORCOUNT} error
  echo ":"

  for ERRORFILE in ${ERRORFILES}; do
    echo -e " ${ERROR}script/debug ${ERRORFILE}${RESET} # ${HIGHLIGHT}${ERRORFILE}${RESET}"
  done
fi

# Exit cleanly if everything was great. Exit uncleanly if everything was not great.
if [[ ${FAILCOUNT} -eq 0 ]] && [[ ${ERRORCOUNT} -eq 0 ]]; then
  exit 0
else
  exit 1
fi
