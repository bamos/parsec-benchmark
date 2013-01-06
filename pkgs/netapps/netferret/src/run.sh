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
	PROGARGS_SERVER="corel lsh 5 5 ${NUMPROCS}"
	PROGARGS_CLIENT="queries ${CLIENTTHREADS} output.txt";;
"simdev"	) 
	PROGARGS_SERVER="corel lsh 5 5 ${NUMPROCS}"
	PROGARGS_CLIENT="queries ${CLIENTTHREADS} output.txt";;
"simsmall"	) 
	PROGARGS_SERVER="corel lsh 10 20 ${NUMPROCS}"
	PROGARGS_CLIENT="queries ${CLIENTTHREADS} output.txt";;
"simmedium"	) 
	PROGARGS_SERVER="corel lsh 10 20 ${NUMPROCS}"
	PROGARGS_CLIENT="queries ${CLIENTTHREADS} output.txt";;
"simlarge"	) 
	PROGARGS_SERVER="corel lsh 10 20 ${NUMPROCS}"
	PROGARGS_CLIENT="queries ${CLIENTTHREADS} output.txt";;
"native"	) 
	PROGARGS_SERVER="corel lsh 50 20 ${NUMPROCS}"
	PROGARGS_CLIENT="queries ${CLIENTTHREADS} output.txt";;
*)  
	echo "Input size error"
	exit 1;;
esac


PROG_PATH="${PARSECDIR}/pkgs/netapps/netferret/inst/${PARSECPLAT}/bin"

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






