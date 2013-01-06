#!/bin/sh

# Description:
#
# Runs the benchmark 
#
#

#Arguments
if [ -n "$1" ]
then
  NUMPROCS="$1"
fi

if [ -n "$2" ]
then
  INPUTSIZE="$2"
fi

NETMODE=""
CLIENTTHREADS=""
if [ -n "$3" ]
then
  if [ "$3" == "server" ] || [ "$3" == "client" ]
  then
     NETMODE="$3"
  else
     CLIENTTHREADS="$3"
  fi
else
  CLIENTTHREADS="1"
fi

if [ -n "$4" ]
then
  CLIENTTHREADS="$4"
fi

#Determine program name, file names & arguments
case "${INPUTSIZE}" in 
"test"	) 
	PROGARGS_SERVER="2 5 1 10 10 5 none output.txt ${NUMPROCS}"
	PROGARGS_CLIENT="1 10 10 ${CLIENTTHREADS}";;
"simdev"	) 
	PROGARGS_SERVER="3 10 3 16 16 10 none output.txt ${NUMPROCS}"
	PROGARGS_CLIENT="3 16 16 ${CLIENTTHREADS}";;
"simsmall"	) 
	PROGARGS_SERVER="10 20 32 4096 4096 1000 none output.txt ${NUMPROCS}"
	PROGARGS_CLIENT="32 4096 4096 ${CLIENTTHREADS}";;
"simmedium"	) 
	PROGARGS_SERVER="10 20 64 8192 8192 1000  none output.txt ${NUMPROCS}"
	PROGARGS_CLIENT="64 8192 8192 ${CLIENTTHREADS}";;
"simlarge"	) 
	PROGARGS_SERVER="10 20 128 16384 16384 1000 none output.txt ${NUMPROCS}"
	PROGARGS_CLIENT="128 16384 16384 ${CLIENTTHREADS}";;
"native"	) 
	PROGARGS_SERVER="10 20 128 1000000 200000 5000 none output.txt ${NUMPROCS}"
	PROGARGS_CLIENT="128 1000000 200000 ${CLIENTTHREADS}";;
*)  
	echo "Input size error"
	exit 1;;
esac


PROG_PATH="${PARSECDIR}/pkgs/netapps/netstreamcluster/inst/${PARSECPLAT}/bin"

#Execution
if [ -z "$NETMODE" ]; then
  RUN_SERVER="$PROG_PATH/server $PROGARGS_SERVER"
  RUN_CLIENT="$PROG_PATH/client $PROGARGS_CLIENT &"

  echo "Running"
  echo $RUN_CLIENT
  echo $RUN_SERVER

  eval $RUN_CLIENT
  eval $RUN_SERVER

elif [ "$NETMODE" == "server" ]; then
  RUN_SERVER="$PROG_PATH/server_sim $PROGARGS_SERVER"

  echo "Running"
  echo $RUN_SERVER

  eval $RUN_SERVER

elif [ "$NETMODE" == "client" ]; then
  RUN_CLIENT="$PROG_PATH/client_sim $PROGARGS_CLIENT"

  echo "Running"
  echo $RUN_CLIENT

  eval $RUN_CLIENT

fi






