#!/bin/bash

# NOTE: after loading in paraview, select 'Points' instead of surface

[[ $# < 1 ]] && echo "argv[0] <text-file> [<nptcls=n/->  <xcol> <key>] " && exit
infile=$1
key=bead #PtclSourcePositions #ptclID
[[ ! -f $infile ]] && echo "input file invalid" && exit
nptcl=0
np=0
[[ $# -gt 1 ]] && np=$2
[[ $# -gt 2 ]] && xcol=$3
[[ $# -gt 3 ]] && key=$4
re='^[0-9]+$'
[[ ! $np =~ $re ]] && echo "No particle limit provided"
[[ $np =~ $re ]] && nptcl=$np && echo "ptcls limit = $np"

nptcl=$(grep -c -w --color=no "$key"  "$infile")
echo "nptcl = $nptcl"
[[ $nptcl < 1 ]] && echo "No particle matched $key" && exit

name=$(basename "$infile")
fname=${name%%.*}.vtk;
[[ -f $fname ]] && mv -f $fname ${fname}_old;
let points=nptcl

echo -e "# vtk DataFile Version 2.0\nparticle paths\nASCII\n \
  DATASET UNSTRUCTURED_GRID\nPOINTS $points float" > $fname; 

grep -w --color=no "$key"  "$infile" | grep -v nan | \
  awk -v xc=$xcol '{print $xc, $(xc+1), $(xc+2)}' >> $fname; 
  
let "connect = $points" ;
let "cellnums = $connect *3" ;
let "connectmin1 = $connect -1";

echo "CELLS $connect $cellnums" >> $fname; 
for n in $(seq 0 $connectmin1); do
  echo -e "2 $n $n" >> $fname; 
done;
echo "CELL_TYPES $connect" >> $fname;
for n in $(seq 0 $connectmin1); do
  echo 3 >> $fname; 
done; 
 
 

