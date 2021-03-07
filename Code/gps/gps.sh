#!/bin/bash
exec 2>/dev/null
# get positions
gpstmp=./gps.data

rm $gpstmp $gpstmp"1"

gpspipe -w -n 40 >$gpstmp"1"&
ppid=$!
sleep 5
kill -9 $ppid
cat $gpstmp"1"|grep -om1 "[-]\?[[:digit:]]\{1,3\}\.[[:digit:]]\{9\}" >$gpstmp
size=$(stat -c%s $gpstmp)
if [ $size -gt 10 ]; then
   cat $gpstmp|sed -n -e 1p >./gps.lat
   cat $gpstmp|sed -n -e 2p >./gps.lon
fi
