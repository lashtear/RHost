GRP=`ps -l|grep 17581|grep -c sleep`
if [ "$GRP" -ne 0 ]
then
   kill -9 17581 2>/dev/null 1>&2
fi
