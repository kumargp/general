#! /bin/bash

repo=/lore/gopan/pumi-pic
wdir=/lore/gopan/tempdir
stat=/lore/gopan/stat
touch $stat

cd $repo && git pull &> /dev/null
mkdir -p $wdir
cd $wdir

if [ `pwd` = "$wdir" ] ; then
  rm -rf ./* 
  source $repo/envRhel7Openmp.sh &> /dev/null
  $repo/doConfig.sh $repo &> /dev/null

  /usr/bin/make  &> /dev/null
  flag=$?
  if [ -f "$stat" ] && [ "$flag" = 1 ]; then
    echo "Failed" > "$stat"
    /usr/bin/mail -s "pumi-pic compiling failed"  perumg@rpi.edu 
  elif [ -f "$stat" ]; then
    echo "Good" > "$stat"
  fi

fi
