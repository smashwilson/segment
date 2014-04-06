#!/bin/bash
#
# Common helpers and functions for output massaging and such.

EXITCODE=0

# Shell Colors.

prefix="\033["
WHITE="${prefix}1;37m"
YELLOW="${prefix}1;33m"
LTGREEN="${prefix}1;32m"
LTRED="${prefix}1;31m"
DKRED="${prefix}0;31m"
RESET="${prefix}0m"

# Associate semantic meanings with specific colors in a consistent way.

export HIGHLIGHT=${WHITE}
export SUCCESS=${LTGREEN}
export FAILURE=${LTRED}
export ERROR=${DKRED}
export MISSING=${YELLOW}

# Text massaging functions.

echo_pluralize () {
  echo -n "$1 $2"
  [[ $1 -ne 1 ]] && echo -n "s"
}

# Output summarization.

print_summary () {
  let TOTALCOUNT=PASSCOUNT+FAILCOUNT+ERRORCOUNT

  echo
  echo -ne "Summary: of ${HIGHLIGHT}"
  echo_pluralize ${TOTALCOUNT} test
  echo -ne ${RESET}
  [[ ${PASSCOUNT} -ne 0 ]] && echo -ne ", ${SUCCESS}${PASSCOUNT} passed${RESET}"
  [[ ${FAILCOUNT} -ne 0 ]] && echo -ne ", ${FAILURE}${FAILCOUNT} failed${RESET}"
  [[ ${ERRORCOUNT} -ne 0 ]] && echo -ne ", ${ERROR}${ERRORCOUNT} caused errors${RESET}"
  [[ ${MISSINGCOUNT} -ne 0 ]] && echo -ne ", ${MISSING}${MISSINGCOUNT} pending${RESET}"
  echo "."
}

print_failure () {
  echo -e " ${FAILURE}cat ${1}.diff${RESET} # ${HIGHLIGHT}${1}${RESET}"
}

print_error () {
  echo -e " ${ERROR}script/debug ${1}${RESET} # ${HIGHLIGHT}${1}${RESET}"
}

print_details () {
  if [[ -n ${FAILFILES} ]]; then
    echo
    echo_pluralize ${FAILCOUNT} "failing test"
    echo ":"

    for FAILFILE in ${FAILFILES}; do print_failure ${FAILFILE}; done
  fi

  if [[ -n ${ERRORFILES} ]]; then
    echo
    echo_pluralize ${ERRORCOUNT} error
    echo ":"

    for ERRORFILE in ${ERRORFILES}; do print_error ${ERRORFILE}; done
  fi

  if [[ -n ${MISSINGFILES} ]]; then
    echo
    echo_pluralize ${MISSINGCOUNT} "pending test"
    echo ":"

    for MISSINGFILE in ${MISSINGFILES}; do print_missing ${MISSINGFILE}; done
  fi
}

derive_exit () {
  # Exit cleanly if everything was great. Exit uncleanly if everything was not great.
  if [[ ${FAILCOUNT} -eq 0 ]] && [[ ${ERRORCOUNT} -eq 0 ]]; then
    EXITCODE=0
  else
    EXITCODE=1
  fi
}
