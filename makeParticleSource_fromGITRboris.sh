#!/bin/bash

src=$1

[[ $# -lt 1 ]] && echo "input file missing" && exit

out=particleSourcesFromGITRboris
#[[ -f $out ]] && echo "out file exists !" && exit 1
nP=$( grep -c xyz $src)
echo " nP = $nP ;" > $out

x=( $(awk '{print $4}' $src))
y=( $(awk '{print $5}' $src))
z=( $(awk '{print $6}' $src))
vx=( $(awk '{print $8}' $src))
vy=( $(awk '{print $9}' $src))
vz=( $(awk '{print $10}' $src))
let nPminus=nP-1

function fill() {
  arr=("$@")
  for i in $(seq 0 $nPminus ); do \
    let "rem = $i % 5"
    [[ $rem -eq 0 && $i -gt 0 ]] && echo >> $out
    echo -n  "${arr[i]}" >> $out
    [[ $i -lt $nPminus ]] &&  echo -n ", " >> $out
    let i=i+1
  done;
  echo ";" >> $out
}

names=( "x" "y" "z" "vx" "vy" "vz" )
echo -n "${names[0]} = " >> $out
fill "${x[@]}"
echo -n "${names[1]} = " >> $out
fill "${y[@]}"
echo -n "${names[2]} = " >> $out
fill "${z[@]}"
echo -n "${names[3]} = " >> $out
fill "${vx[@]}"
echo -n "${names[4]} = " >> $out
fill "${vy[@]}"
echo -n "${names[5]} = " >> $out
fill "${vz[@]}"

