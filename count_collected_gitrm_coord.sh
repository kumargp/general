#!/bin/bash


[[ $# -lt 1 ]] && echo "$BASH_SOURCE  <file>" && exit

file=$1

for n in $( seq 0 13); do
awk -v n=$n '
BEGIN {
 h1=0;
 h2=0.01275;
 h=0.01;
 c=0.0446;
 r=0.005;
 sum=0;

 if(n==1) { h1=h2; h2+=h;}
 else if(n>1) {h1=h2+(n-1)*h; h2+=n*h;}
}
{
  if(sqrt(($8-c)*($8-c)+($9*$9)) <= r && $10>h1 && $10<=h2) {
    sum+=1;
    #if(n==2) print substr($NF, 0, length($2-2)) "bead2 " $8, $9, $10;
    #print $8, $9, $10;
  }
}
 END {
  print n " " sum " " h1 " " h2 ;
 }' $file

 done
