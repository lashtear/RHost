#!/bin/sh
LOCATE=""
if [ -f /bin/bash ]
then
   LOCATE="/bin/bash"
elif [ -f /usr/bin/bash ]
then
   LOCATE="/usr/bin/bash"
elif [ -f /usr/local/bin/bash ]
then
   LOCATE="/usr/local/bin/bash"
elif [ -f /bin/ksh ]
then
   LOCATE="/bin/ksh"
elif [ -f /usr/bin/ksh ]
then
   LOCATE="/usr/bin/ksh"
elif [ -f /usr/local/bin/ksh ]
then
   LOCATE="/usr/local/bin/ksh"
fi
if [ ! -z "${LOCATE}" ]
then
   echo "#!${LOCATE}" > ./bin/asksource.sh
   cat ./bin/asksource.blank >> ./bin/asksource.sh
   chmod 755 ./bin/asksource.sh
   echo "#!${LOCATE}" > ./bin/bugreport.sh
   cat ./bin/bugreport.blank >> ./bin/bugreport.sh
   chmod 755 ./bin/bugreport.sh
else
   echo "Warning!  Can not find an effective shell.  Needs ksh or bash compatable!"
fi
