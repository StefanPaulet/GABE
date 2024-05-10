#!/bin/bash

while :
do
  result=$(xwininfo -root -tree | grep --ignore-case "Counter-Strike 2")
  if [ -n "$result" ]; then
    IFS=' ' read -r -a array <<< "$result"
    echo $array
    break
  else
    sleep 1
  fi
done