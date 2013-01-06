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
TARGET=radiosity

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
	PROGARGS="-batch -room -p ${NUMPROCS}";;
"simdev"	) 
	PROGARGS="-batch -room -p ${NUMPROCS}";;
"simsmall"	) 
	PROGARGS="-batch -room -p ${NUMPROCS}";;
"simmedium"	) 
	PROGARGS="-batch -room -p ${NUMPROCS}";;
"simlarge"	) 
	PROGARGS="-batch -room -p ${NUMPROCS}";;
"native"	) 
	PROGARGS="-batch -largeroom -p ${NUMPROCS}";;
*)  
	echo "Input size error"
	exit 1;;
esac
 

PROG="${PARSECDIR}/ext/splash2/apps/${TARGET}/inst/${PARSECPLAT}/bin/${TARGET}"


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


