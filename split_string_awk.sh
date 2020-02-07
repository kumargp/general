

awk 'BEGIN{FS=",";sum=0;} /surface/{for(i=0;i<NF;i++){if($i==1){sum+=1}}; print sum;}' <file>
