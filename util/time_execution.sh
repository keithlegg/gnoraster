#!/bin/bash


# commandstr="./scanfids scan 0001.png 30 40"

commandstr="./scanfids anchor 0001.png 30 40"
echo "############"
mytime="$(time $commandstr 2>&1 1>/dev/null )"
echo "$mytime"




