#!/bin/sh

# Description:
#
# Runs the benchmark with a minimal problem size in a multiprocessor
# simulator.
#
# Usage:
#
# ./run.sh [NUMPROCS]
#

#Some default values
TARGET=fft

#Arguments
if [ -n "$1" ]
then
  NUMPROCS="$1"
fi

if [ -n "$2" ]
then
  INPUTSIZE="$2"
fi


if [ -n "$3" ]
then
  echo "Error: Too many arguments!"
  echo 
  head -n11 $0 | tail -n9 | sed 's/#//'
  exit 1
fi


#Determine program name, file names & arguments
case "${INPUTSIZE}" in 
"test"	) 
	PROGARGS="-m18 -p${NUMPROCS}";;
"simdev"	) 
	PROGARGS="-m18 -p${NUMPROCS}";;
"simsmall"	) 
	PROGARGS="-m20 -p${NUMPROCS}";;
"simmedium"	) 
	PROGARGS="-m22 -p${NUMPROCS}";;
"simlarge"	) 
	PROGARGS="-m24 -p${NUMPROCS}";;
"native"	) 
	PROGARGS="-m28 -p${NUMPROCS}";;
*)  
	echo "Input size error"
	exit 1;;
esac
 


PROG="${PARSECDIR}/ext/splash2x/kernels/${TARGET}/inst/${PARSECPLAT}/bin/${TARGET}"


#Some tests
if [ ! -x "$PROG" ]
then
  echo "Error: Binary ${PROG} does not exist!"
  exit 1
fi


#Execution
echo Generating input file ${INPUTFILE}...

RUN="$PROG $PROGARGS"

echo "Running $RUN:"
eval $RUN


