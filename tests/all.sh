#!/bin/bash
#
# Execute all tests, in sequence.

set -e

BASEDIR=`dirname $0`
ROOTDIR=${BASEDIR}/..

${BASEDIR}/ast.sh
