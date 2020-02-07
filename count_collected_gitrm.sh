#!/bin/bash


[[ $# -lt 1 ]] && echo "$BASH_SOURCE  <file> [limit]" && exit

file=$1
limit=100000
#[[ $# -eq 1 ]] && echo "limit is set to 100K"
[[ $# -gt 1 ]] && limit=$2

for i in $(seq 0 13) ; do  \
  awk -v num=$i -vlimit=$limit \
  'BEGIN {sum=0} 
   ($6==num && $5 ~ detId && $NF < limit) {sum+=1} 
   END {print num " " sum}' $file ; 
done
