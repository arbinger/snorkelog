#!/bin/bash

# Change this line to wherever you have snorkelog installed on your
# system. I use the home folder:
cd ~/snorkelog
ls -altr .

# make sure there is only one snorkelog running at a time
RUNID=`ps -ef | grep "./[s]norkelog$" | awk '{print $2}'`
echo "RUNID=[$RUNID]"
if [[ -n "$RUNID" ]]; then kill $RUNID; fi;

# kick it off
./snorkelog
