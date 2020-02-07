#! /bin/bash

[[ $# -lt 1 ]] && echo "file missing " && exit
file=$1

awk '/findDist2bdryTime/{sum+=$2;n+=1} END{if(n>0)print n " \
  avg_findDist2bdryTime " sum/n}' $file

awk '/calcETime/{sum+=$2;n+=1} END{ if(n>0)print n " avg_calcETime " sum/n}' $file
awk '/pushTime/{sum+=$2;n+=1} END{if(n>0)print n " avg_pushTime " sum/n}' $file
awk '/searchTime/{sum+=$2;n+=1} END{if(n>0)print n " avg_searchTime " sum/n}' $file

awk '/recTime/{sum+=$2;n+=1} END{if(n>0)print n " avg_ionRecTime " sum/n}' $file
