#!/bin/bash
#
# Common helpers and functions for output massaging and such.

# Shell Colors.

prefix="\033["
WHITE="${prefix}1;37m"
LTGREEN="${prefix}1;32m"
LTRED="${prefix}1;31m"
DKRED="${prefix}0;31m"
RESET="${prefix}0m"

# Associate semantic meanings with specific colors in a consistent way.

export HIGHLIGHT=${WHITE}
export SUCCESS=${LTGREEN}
export FAILURE=${LTRED}
export ERROR=${DKRED}

# Text massaging functions.

echo_pluralize () {
  echo -n "$1 $2"
  [[ $1 -ne 1 ]] && echo -n "s"
}
