#!/bin/bash

#find median
[[ $# -eq 0 ]] && echo "$0 <string> <colum_index> <file>" && exit 1
str=$1
col=$2
file=$3
awk -v str=$str -v col=$col '$0 ~ str{print $col}' $file \
  | sort -n | \
  awk '{a[NR] = $0} END{print (NR%2==1)?a[int(NR/2)+1]:(a[NR/2]+a[NR/2+1])/2}'
