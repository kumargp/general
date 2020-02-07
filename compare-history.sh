#! /bin/bash

# compare gitrm and gitr history files
[[ $# -lt 2 ]] && echo "argv[0] <histroy-file1> <history-file2> " && exit 1

hfile1=$1
hfile2=$2
np=0
[[ $# -gt 2 ]] && np=$3

result=compare_out.txt
mv -f $result ${result}-old

compare=comparePtclHistoryLists

#1st file if for GITRm;  "iter 0 pos" ; "ptclHistory_accum 1"
f1patt1="iter"
f1patt2="pos"
f1patt3="ptclHistory_accum"

#file2 for GITR , "gitr 1 tstep"
f2patt1="gitr"
f2patt2="tstep"

npfile=$( grep --color=never -w "$f1patt1 0 $f1patt2" "$hfile1" | wc -l )
if (( np <= 0 )) || (( npfile < np )); then
  echo "setting np from $np to $npfile from file"
  np=$npfile
fi

[[ $np -eq 0 ]] && echo "#particles = 0" && exit 1

echo "Number of Particles: $np"

for ((p=1; p <= np; p++)); do
  echo "processing ptcl $p .."
  pfile1=${hfile1}_ptcl${p}
  pfile2=${hfile2}_ptcl${p}

  grep --color=never  -w "$f1patt3 $p" $hfile1 > $pfile1 #"ptclHistory_accum 1"
  grep --color=never  -w "$f2patt1 $p $f2patt2" $hfile2 > $pfile2 #"gitr 1 tstep"

  stopped1=$( tail -1 $pfile1 | awk '{print $NF}' )
  stopped2=$( tail -1 $pfile2 | awk '{print $NF}' )

  ~/GITRmdev/${compare} $pfile1 $pfile2 1 >> $result
  echo "stopped1: $stopped1 stopped2: $stopped2" >> $result

  rm -f $pfile1
  rm -f $pfile2
done