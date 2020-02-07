#!/bin/bash

# $1 filename
# $2 sequential number=index starting 0
# $3 number to get upper bound index of. Valid only if $2 is - 

[[ $# -eq 0 ]] && echo "argv[0] <file> <pos-index> <match-number>" && exit 1
file=$1
pos=$2
match=$3
[[ $pos -ge 0 ]]  && match= 


# NF-1 since last comma in each line
awk -F '[,;]' -v pos=$pos -v match=$match \
  'BEGIN { 
    pre=0;num=0;ind=pos; 
  } {
    pre=num;
    num+=NF-1;
  } { 
    if( pos<=num-1) {
      if(ind>=pre) 
        ind=pos-pre ;
      print $(ind+1) " :" pre "-" num-1 ":" $0;
      exit; 
    }
  }' $file

