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
DKRED="${prefix}0;31m"
RESET="${prefix}0m"

for SRCFILE in ${BASEDIR}/ast/*.seg; do
  EXPECTED_AST=${SRCFILE}.ast
  ACTUAL_AST=${SRCFILE}.ast.actual
  AST_DIFF=${SRCFILE%.*}.diff

  echo -ne "ast: ${WHITE}${SRCFILE}${RESET} .."

  ${ROOTDIR}/bin/segment --ast-debug ${SRCFILE} > ${ACTUAL_AST}
  PARSE_CODE=$?

  if [[ ${PARSE_CODE} -eq 0 ]]; then
    diff ${EXPECTED_AST} ${ACTUAL_AST} > ${AST_DIFF}
    DIFF_CODE=$?

    if [[ ${DIFF_CODE} -eq 0 ]]; then
      echo -e " ${LTGREEN}pass${RESET}"
      [ -z ${AST_KEEP} ] && rm -f ${ACTUAL_AST} ${AST_DIFF}
    else
      # Failed
      echo -e " ${DKRED}fail${RESET}"
    fi
  else
    # Parse error
    echo -e " ${DKRED}error${RESET} (${PARSE_CODE})"
  fi
done
