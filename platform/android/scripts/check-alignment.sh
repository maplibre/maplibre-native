#!/bin/bash

# usage: alignment.sh path to search for *.so files

dir="$1"

matches="$(find $dir -name "*.so" -type f)"
IFS=$'\n'

err=0

for match in $matches; do
  res="$(objdump -p ${match} | grep LOAD | awk '{ print $NF }' | head -1)"
  if [[ $res =~ "2**14" ]] || [[ $res =~ "2**16" ]]; then
    echo -e "${match}: ALIGNED ($res)"
  else
    if [[ "$match" == *"x86_64"* || "$match" == *"arm64-v8a"* ]]; then
      echo "ERROR:"
      err=1
    fi
    echo -e "${match}: UNALIGNED ($res)"
  fi
done

exit $err
