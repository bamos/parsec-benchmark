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
if [ -n "$3" ]
then
  NETMODE="$3"
fi


#Determine program name, file names & arguments
case "${INPUTSIZE}" in 
"test"	) 
	PROGARGS_CLIENT="-i test.dat";;
"simdev"	) 
	PROGARGS_CLIENT="-i test.dat";;
"simsmall"	) 
	PROGARGS_CLIENT="-i test.dat";;
"simmedium"	) 
	PROGARGS_CLIENT="-i test.dat";;
"simlarge"	) 
	PROGARGS_CLIENT="-i media.dat";;
"native"	) 
	PROGARGS_CLIENT="-i FC-6-x86_64-disc1.iso";;
*)  
	echo "Input size error"
	exit 1;;
esac
 
PROGARGS_SERVER="-c -v -t ${NUMPROCS} -o output.dat.ddp"


PROG_PATH="${PARSECDIR}/pkgs/netapps/netdedup/inst/${PARSECPLAT}/bin"

#Execution
if [ -z "$NETMODE" ]; then
  RUN_SERVER="$PROG_PATH/netdedup $PROGARGS_SERVER"
  RUN_CLIENT="$PROG_PATH/netdedup_client $PROGARGS_CLIENT &"

  echo "Running"
  echo $RUN_CLIENT
  echo $RUN_SERVER

  eval $RUN_CLIENT
  eval $RUN_SERVER

elif [ "$NETMODE" == "server" ]; then
  RUN_SERVER="$PROG_PATH/netdedup_sim $PROGARGS_SERVER"

  echo "Running"
  echo $RUN_SERVER

  eval $RUN_SERVER

elif [ "$NETMODE" == "client" ]; then
  RUN_CLIENT="$PROG_PATH/netdedup_client_sim $PROGARGS_CLIENT"

  echo "Running"
  echo $RUN_CLIENT

  eval $RUN_CLIENT

fi






