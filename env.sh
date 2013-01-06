#!/bin/bash
# Copyright (C) 2008 Princeton University
# All rights reserved.
# Author: Christian Bienia

#
# Source this script to setup environment for more convenient work with PARSEC.
# This step is purely optional.
#
# You can use the script as follows:
#
#     source env.sh
#
# This script only works if you are using the bash shell.
#


xxPWDxx=pwd
xxDIRNAMExx=dirname

# Function that tries to autodetect path to PARSEC distribution
# Side effect: `xxPARSECDIRxx' environment variable is set
function detect_path {
  # Try to find path
  local xxuniquefilexx=".parsec_uniquefile"
  local xxparsecdirxx=""
  if [ ! -z "${PARSECDIR}" ]; then
    # User defined PARSECDIR, check it
    xxparsecdirxx="${PARSECDIR}"
    if [ ! -f "${xxparsecdirxx}/${xxuniquefilexx}" ]; then
      echo "Error: Variable PARSECDIR points to '${PARSECDIR}', but this does not seem to be the PARSEC directory. Either unset PARSECDIR to make me try to autodetect the path or set it to the correct value."
      exit 1
    fi
  else
    # Try to autodetect path by looking at path used to invoke this script

    # Try to extract absoute or relative path
    if [ "${1:0:1}" == "/" ]; then
      # Absolute path given
      eval xxparsecdirxx=$(${xxDIRNAMExx} $(${xxDIRNAMExx} $1))
      # Check
      if [ -f "${xxparsecdirxx}/${xxuniquefilexx}" ]; then
        xxPARSECDIRxx=${xxparsecdirxx}
      fi
    else
      # No absolute path, maybe relative path?
      eval xxparsecdirxx=$(${xxPWDxx})/$(${xxDIRNAMExx} $(${xxDIRNAMExx} $1))
      # Check
      if [ -f "${xxparsecdirxx}/${xxuniquefilexx}" ]; then
        xxPARSECDIRxx=${xxparsecdirxx}
      fi
    fi

    # If xxPARSECDIRxx is still undefined, we try to guess the path
    if [ -z "${xxPARSECDIRxx}" ]; then
      # Check current directory
      if [ -f "./${xxuniquefilexx}" ]; then
        xxparsecdirxx="$(${xxPWDxx})"
        xxPARSECDIRxx=${xxparsecdirxx}
      fi
    fi
    if [ -z "${xxPARSECDIRxx}" ]; then
      # Check next-higher directory
      if [ -f "../${xxuniquefilexx}" ]; then
        xxparsecdirxx="$(${xxPWDxx})/.."
        xxPARSECDIRxx=${xxparsecdirxx}
      fi
    fi
  fi

  # Make sure xxPARSECDIRxx is defined and exported
  if [ -z "${xxPARSECDIRxx}" ]; then
    echo "$Error: Unable to autodetect path to the PARSEC benchmark suite. Please define environment variable PARSECDIR."
    exit 1
  fi
  export xxPARSECDIRxx

  # Eliminate trailing `/.' from xxPARSECDIRxx
  xxPARSECDIRxx=${xxPARSECDIRxx/%\/./}
}

detect_path "."

# Append `bin/' directory to PATH
if [ -z "${PATH}" ]; then
  export PATH=${xxPARSECDIRxx}/bin
else
  export PATH=${PATH}:${xxPARSECDIRxx}/bin
fi

# Append `man/' directory to MANPATH
if [ -z "${MANPATH}" ]; then
  export MANPATH=${xxPARSECDIRxx}/man
else
  export MANPATH=${MANPATH}:${xxPARSECDIRxx}/man
fi

# Add directory with hooks library to library search path
if [ -z "${LD_LIBRARY_PATH}" ]; then
  export LD_LIBRARY_PATH="${PARSECDIR}/pkgs/libs/hooks/inst/${PARSECPLAT}/lib"
else
  export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${PARSECDIR}/pkgs/libs/hooks/inst/${PARSECPLAT}/lib"
fi

