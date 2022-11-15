#!/bin/bash

set -o errexit -o nounset -o pipefail

# Directory of this script
DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Recompile pintool if necessary
(cd $DIR; make -q >/dev/null 2>/dev/null || make)

# Get root of Pin installation (determined from Makefile)
PIN_ROOT=$(cat $DIR/.pin_root)

# program to trace and its arguments
PROGARGS=$(echo ${@} | sed s,.*--\ ,,)
PROG=$(echo $PROGARGS | { read first rest; echo $(basename $first) | sed s,\\s.*,, ; } )

# Run pin
echo "### running pin: $PROGARGS $DIR"

set +o errexit
time -p $PIN_ROOT/pin -t $DIR/obj-*/memory_access_rtn_csv.so ${@}
retcode=$?
set -o errexit

exit $retcode
